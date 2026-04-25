/**
 * @file GoogleCalendarService.cpp
 *
 * OAuth 2.0 — **client de type Application de bureau** (Google Cloud) :
 * loopback redirect, pas de secret côté serveur tiers ; voir RFC 8252 (loopback redirect URIs).
 *
 * 1) loadCredentials() lit `client_secret.json` en priorité (ou `google_credentials.json`) :
 *    clé racine `installed` (recommandée pour Desktop) ou `web` avec client_id, client_secret,
 *    auth_uri, token_uri.
 *
 * 2) connectToGoogle() démarre QTcpServer sur 127.0.0.1 avec **port 0** (port éphémère choisi par l’OS),
 *    construit redirect_uri = http://127.0.0.1:<port>/ (même valeur pour auth et pour l’échange token),
 *    ouvre le navigateur avec QDesktopServices::openUrl().
 *
 * 3) Google redirige vers http://127.0.0.1:<port>/?code=...&state=... ; parsing HTTP minimal, puis
 *    QNetworkAccessManager POST vers token_uri → access_token + refresh_token.
 *
 * 4) saveTokensFromJson() enregistre access_token (et refresh_token) dans tokens.json
 *    (répertoire de configuration utilisateur, voir tokenStorageWritePath()).
 *
 * 5) syncCalendar() assure un access_token valide (refresh si besoin), puis GET
 *    https://www.googleapis.com/calendar/v3/calendars/dorrajridi8%40gmail.com/events
 *
 * 6) createEvent() POST du corps JSON sur le même endpoint (insertion d’un événement).
 */

#include "GoogleCalendarService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace {

/// Lecture + écriture d’événements (nécessaire pour createEvent ; readonly ne suffit pas).
constexpr char kGoogleCalendarScope[] = "https://www.googleapis.com/auth/calendar.events";
constexpr char kCalendarEventsEndpoint[] =
    "https://www.googleapis.com/calendar/v3/calendars/dorrajridi8%40gmail.com/events";
QString randomStateString()
{
    QString s;
    for (int i = 0; i < 32; ++i)
        s += QString::number(QRandomGenerator::global()->bounded(16), 16);
    return s;
}

qint64 epochSecondsPlus(int secondsFromNow)
{
    return QDateTime::currentSecsSinceEpoch() + secondsFromNow;
}

int httpStatusCode(QNetworkReply *reply)
{
    if (!reply)
        return 0;
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

} // namespace

GoogleCalendarService::GoogleCalendarService(QObject *parent)
    : QObject(parent)
{
    loadStoredTokens();
}

QString GoogleCalendarService::resolveCredentialsJsonPath() const
{
    /// client_secret.json en premier (fichier standard téléchargé depuis Google Cloud).
    const QStringList names = {
        QStringLiteral("client_secret.json"),
        QStringLiteral("google_credentials.json"),
    };
    for (const QString &fileName : names) {
        const QStringList candidates = {
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(fileName),
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../") + fileName),
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../../") + fileName),
            QDir::current().absoluteFilePath(fileName),
            QDir::current().absoluteFilePath(QStringLiteral("leather/") + fileName),
        };
        for (const QString &p : candidates) {
            if (QFile::exists(p))
                return p;
        }
    }
    return {};
}

bool GoogleCalendarService::loadCredentials(QString *errorMessage)
{
    const QString path = resolveCredentialsJsonPath();
    if (path.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral(
                "Fichier introuvable : client_secret.json (ou google_credentials.json).\n"
                "Placez-le dans le dossier du projet (à côté du .pro) ou à côté de l’exécutable.");
        return false;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible d’ouvrir : %1").arg(path);
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("JSON invalide (objet racine attendu).");
        return false;
    }

    const QJsonObject root = doc.object();
    QJsonObject cfg = root.value(QStringLiteral("installed")).toObject();
    if (cfg.isEmpty())
        cfg = root.value(QStringLiteral("web")).toObject();
    if (cfg.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Section « installed » ou « web » introuvable dans le JSON.");
        return false;
    }

    m_clientId = cfg.value(QStringLiteral("client_id")).toString();
    m_clientSecret = cfg.value(QStringLiteral("client_secret")).toString();
    m_tokenUri = cfg.value(QStringLiteral("token_uri")).toString(QStringLiteral("https://oauth2.googleapis.com/token"));
    m_authUri = cfg.value(QStringLiteral("auth_uri")).toString(QStringLiteral("https://accounts.google.com/o/oauth2/v2/auth"));

    if (m_clientId.isEmpty() || m_clientSecret.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("client_id ou client_secret manquant dans le fichier.");
        return false;
    }
    return true;
}

static QString tokenStorageWritePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return QDir(dir).absoluteFilePath(QStringLiteral("tokens.json"));
}

static QString legacyTokenStoragePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(dir).absoluteFilePath(QStringLiteral("google_calendar_tokens.json"));
}

static bool readTokensFromFile(const QString &path, QString *accessOut, QString *refreshOut, qint64 *expiryOut)
{
    if (!QFile::exists(path))
        return false;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    if (accessOut)
        *accessOut = o.value(QStringLiteral("access_token")).toString();
    if (refreshOut)
        *refreshOut = o.value(QStringLiteral("refresh_token")).toString();
    if (expiryOut)
        *expiryOut = static_cast<qint64>(o.value(QStringLiteral("access_token_expiry_epoch")).toDouble());
    return true;
}

bool GoogleCalendarService::loadStoredTokens()
{
    QString at;
    QString rt;
    qint64 exp = 0;

    const QString primary = tokenStorageWritePath();
    const QString besideExe = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("tokens.json"));

    if (readTokensFromFile(besideExe, &at, &rt, &exp) || readTokensFromFile(primary, &at, &rt, &exp)) {
        m_accessToken = at;
        m_refreshToken = rt;
        m_accessTokenExpirySecs = exp;
        return !m_accessToken.isEmpty() || !m_refreshToken.isEmpty();
    }

    /// Migration : ancien nom de fichier après mise à jour.
    if (readTokensFromFile(legacyTokenStoragePath(), &at, &rt, &exp)) {
        m_accessToken = at;
        m_refreshToken = rt;
        m_accessTokenExpirySecs = exp;
        return !m_accessToken.isEmpty() || !m_refreshToken.isEmpty();
    }

    return false;
}

void GoogleCalendarService::saveTokensFromJson(const QJsonObject &obj)
{
    const QString at = obj.value(QStringLiteral("access_token")).toString();
    const int expiresIn = obj.value(QStringLiteral("expires_in")).toInt(3600);
    const QString rt = obj.value(QStringLiteral("refresh_token")).toString();

    if (!at.isEmpty())
        m_accessToken = at;
    if (!rt.isEmpty())
        m_refreshToken = rt;
    m_accessTokenExpirySecs = epochSecondsPlus(qMax(60, expiresIn - 120));

    QJsonObject store;
    store.insert(QStringLiteral("access_token"), m_accessToken);
    store.insert(QStringLiteral("refresh_token"), m_refreshToken);
    store.insert(QStringLiteral("access_token_expiry_epoch"), m_accessTokenExpirySecs);

    const QString outPath = tokenStorageWritePath();
    QFile f(outPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(store).toJson(QJsonDocument::Indented));
        f.close();
    }
}

bool GoogleCalendarService::accessTokenNeedsRefresh() const
{
    if (m_accessToken.isEmpty())
        return true;
    return QDateTime::currentSecsSinceEpoch() >= m_accessTokenExpirySecs;
}

void GoogleCalendarService::closeRedirectServer()
{
    if (m_redirectServer) {
        m_redirectServer->close();
        m_redirectServer->deleteLater();
        m_redirectServer.clear();
    }
}

bool GoogleCalendarService::pickLoopbackPort(QString *redirectUriOut, QString *errorMessage)
{
    auto *srv = new QTcpServer(this);
    /// Port 0 = flux **Application de bureau** : l’OS attribue un port libre (RFC 8252 loopback).
    /// Pas d’URI fixe à déclarer manuellement pour le type « Desktop » dans Google Cloud.
    if (!srv->listen(QHostAddress::LocalHost, 0)) {
        srv->deleteLater();
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible d’écouter sur 127.0.0.1 (aucun port disponible).");
        return false;
    }
    m_redirectServer = srv;
    const quint16 port = srv->serverPort();
    m_redirectUri = QStringLiteral("http://127.0.0.1:%1/").arg(port);
    if (redirectUriOut)
        *redirectUriOut = m_redirectUri;
    connect(m_redirectServer, &QTcpServer::newConnection, this, &GoogleCalendarService::onRedirectServerNewConnection);
    return true;
}

void GoogleCalendarService::connectToGoogle()
{
    QString err;
    if (!loadCredentials(&err)) {
        emit authenticationFinished(false, err);
        return;
    }

    m_oauthLoopbackHandled = false;

    QString redirect;
    if (!pickLoopbackPort(&redirect, &err)) {
        emit authenticationFinished(false, err);
        return;
    }

    m_oauthState = randomStateString();

    QUrl url(m_authUri);
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("client_id"), m_clientId);
    q.addQueryItem(QStringLiteral("redirect_uri"), redirect);
    q.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
    q.addQueryItem(QStringLiteral("scope"), QString::fromUtf8(kGoogleCalendarScope));
    q.addQueryItem(QStringLiteral("access_type"), QStringLiteral("offline"));
    q.addQueryItem(QStringLiteral("prompt"), QStringLiteral("consent"));
    q.addQueryItem(QStringLiteral("state"), m_oauthState);
    url.setQuery(q);

    if (!QDesktopServices::openUrl(url)) {
        closeRedirectServer();
        err = QStringLiteral("Impossible d’ouvrir le navigateur (QDesktopServices::openUrl).");
        emit authenticationFinished(false, err);
        return;
    }
}

void GoogleCalendarService::onRedirectServerNewConnection()
{
    if (!m_redirectServer)
        return;
    /// Chrome ouvre souvent plusieurs TCP (navigation, favicon, etc.) : chaque socket doit être servi.
    /// Ne pas rejeter la 2ᵉ connexion — sinon ERR_CONNECTION_RESET sur la redirection qui porte le `code`.
    while (QTcpSocket *sock = m_redirectServer->nextPendingConnection()) {
        connect(sock, &QTcpSocket::readyRead, this, [this, sock]() {
        if (sock->property("oauthHandled").toBool())
            return;

        /// Accumuler les octets : le navigateur peut envoyer la requête HTTP en plusieurs paquets.
        QByteArray acc = sock->property("oauthAcc").toByteArray();
        acc += sock->readAll();
        sock->setProperty("oauthAcc", acc);
        const QString reqStr = QString::fromUtf8(acc);
        const int firstLineEnd = reqStr.indexOf(QStringLiteral("\r\n"));
        if (firstLineEnd < 0) {
            if (acc.size() > 65536) {
                if (!m_oauthLoopbackHandled) {
                    emit authenticationFinished(false, QStringLiteral("Requête OAuth loopback trop longue ou invalide."));
                    m_oauthLoopbackHandled = true;
                    closeRedirectServer();
                }
                sock->setProperty("oauthHandled", true);
                sock->disconnect(this);
                sock->deleteLater();
            }
            return;
        }

        auto respondHtml = [sock](const QString &html, int code = 200) {
            const QByteArray body = html.toUtf8();
            const QString status = (code == 200) ? QStringLiteral("200 OK") : QStringLiteral("400 Bad Request");
            QByteArray resp = QStringLiteral("HTTP/1.1 %1\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %2\r\nConnection: close\r\n\r\n")
                                  .arg(status)
                                  .arg(body.size())
                                  .toUtf8();
            resp += body;
            sock->write(resp);
            sock->flush();
            sock->disconnectFromHost();
        };

        auto respondEmpty = [sock]() {
            sock->write(QStringLiteral("HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n").toUtf8());
            sock->flush();
            sock->disconnectFromHost();
        };

        const QString firstLine = reqStr.left(firstLineEnd);
        QString resourcePath;
        if (firstLine.startsWith(QStringLiteral("GET "), Qt::CaseInsensitive)) {
            const int sp = firstLine.indexOf(QLatin1Char(' '), 4);
            const int httpAt = firstLine.indexOf(QStringLiteral(" HTTP/"), qMax(sp, 4));
            if (sp > 0 && httpAt > sp)
                resourcePath = firstLine.mid(sp + 1, httpAt - sp - 1);
        }

        if (resourcePath.isEmpty()) {
            respondEmpty();
            sock->setProperty("oauthHandled", true);
            sock->disconnect(this);
            connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
            return;
        }

        const QUrl u(QStringLiteral("http://127.0.0.1") + resourcePath);
        const QUrlQuery pq(u.query());

        const QString err = pq.queryItemValue(QStringLiteral("error"));
        const QString code = pq.queryItemValue(QStringLiteral("code"));
        const QString state = pq.queryItemValue(QStringLiteral("state"));

        /// Requêtes auxiliaires (favicon, etc.) : pas de `code` ni `error` OAuth → ne pas fermer le serveur ni invalider le flux.
        if (code.isEmpty() && err.isEmpty()) {
            respondEmpty();
            sock->setProperty("oauthHandled", true);
            sock->disconnect(this);
            connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
            return;
        }

        if (m_oauthLoopbackHandled) {
            respondHtml(QStringLiteral("<html><body><p>Autorisation déjà traitée.</p></body></html>"));
            sock->setProperty("oauthHandled", true);
            sock->disconnect(this);
            connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
            return;
        }

        if (!err.isEmpty()) {
            if (state != m_oauthState) {
                respondHtml(QStringLiteral("<html><body><h3>Erreur</h3><p>State invalide.</p></body></html>"), 400);
                m_oauthLoopbackHandled = true;
                emit authenticationFinished(false, QStringLiteral("Réponse OAuth invalide (state)."));
                sock->setProperty("oauthHandled", true);
                sock->disconnect(this);
                closeRedirectServer();
                sock->deleteLater();
                return;
            }
            respondHtml(QStringLiteral("<html><body><h3>Erreur OAuth</h3><p>%1</p></body></html>").arg(err.toHtmlEscaped()), 400);
            m_oauthLoopbackHandled = true;
            emit authenticationFinished(false, QStringLiteral("Autorisation refusée : %1").arg(err));
            sock->setProperty("oauthHandled", true);
            sock->disconnect(this);
            closeRedirectServer();
            sock->deleteLater();
            return;
        }

        if (state != m_oauthState) {
            respondHtml(QStringLiteral("<html><body><h3>Erreur</h3><p>State invalide.</p></body></html>"), 400);
            m_oauthLoopbackHandled = true;
            emit authenticationFinished(false, QStringLiteral("Réponse OAuth invalide (state)."));
            sock->setProperty("oauthHandled", true);
            sock->disconnect(this);
            closeRedirectServer();
            sock->deleteLater();
            return;
        }

        respondHtml(QStringLiteral(
            "<html><body style=\"font-family:Segoe UI,sans-serif;text-align:center;padding:2em;\">"
            "<h2>Connexion réussie</h2><p>Vous pouvez fermer cette fenêtre.</p></body></html>"));

        const QString redirectUsed = m_redirectUri;
        m_oauthLoopbackHandled = true;
        sock->setProperty("oauthHandled", true);
        sock->disconnect(this);
        closeRedirectServer();
        connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
        QTimer::singleShot(0, this, [this, code, redirectUsed]() {
            exchangeCodeForTokens(code, redirectUsed);
        });
    });
    }
}

void GoogleCalendarService::exchangeCodeForTokens(const QString &code, const QString &redirectUri)
{
    QUrl tokenUrl(m_tokenUri);
    QNetworkRequest rq(tokenUrl);
    rq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrlQuery body;
    body.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
    body.addQueryItem(QStringLiteral("code"), code);
    body.addQueryItem(QStringLiteral("client_id"), m_clientId);
    body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
    body.addQueryItem(QStringLiteral("redirect_uri"), redirectUri);

    const QByteArray postData = body.query(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *rep = m_nam.post(rq, postData);
    connect(rep, &QNetworkReply::finished, this, &GoogleCalendarService::onTokenReplyFinished);
}

void GoogleCalendarService::onTokenReplyFinished()
{
    auto *rep = qobject_cast<QNetworkReply *>(sender());
    if (!rep)
        return;
    rep->deleteLater();

    const int status = httpStatusCode(rep);
    const QByteArray body = rep->readAll();
    qDebug() << "[GoogleCalendar][Token] HTTP" << status << "| QtError" << rep->error()
             << rep->errorString();
    qDebug().noquote() << "[GoogleCalendar][Token] Body:" << QString::fromUtf8(body);

    if (rep->error() != QNetworkReply::NoError) {
        emit authenticationFinished(
            false, QStringLiteral("Échange code → token (HTTP %1) : %2").arg(status).arg(rep->errorString()));
        return;
    }

    const QJsonObject obj = QJsonDocument::fromJson(body).object();
    if (obj.contains(QStringLiteral("error"))) {
        const QString desc = obj.value(QStringLiteral("error_description")).toString();
        emit authenticationFinished(false,
                                    QStringLiteral("OAuth : %1 — %2")
                                        .arg(obj.value(QStringLiteral("error")).toString(), desc));
        return;
    }

    saveTokensFromJson(obj);
    emit authenticationFinished(true, QStringLiteral("Connecté. Jetons enregistrés localement."));
}

void GoogleCalendarService::syncCalendar()
{
    QString err;
    if (!loadCredentials(&err)) {
        emit errorOccurred(err);
        return;
    }

    if (m_refreshToken.isEmpty() && m_accessToken.isEmpty()) {
        emit errorOccurred(QStringLiteral("Non connecté : cliquez sur « Connecter Google Calendar »."));
        return;
    }

    qDebug() << "[GoogleCalendar][Sync] Access token empty?" << m_accessToken.isEmpty()
             << "| expiresAt(epoch):" << m_accessTokenExpirySecs
             << "| now(epoch):" << QDateTime::currentSecsSinceEpoch();

    if (accessTokenNeedsRefresh()) {
        if (m_refreshToken.isEmpty()) {
            emit errorOccurred(QStringLiteral("Token expiré : reconnectez-vous."));
            return;
        }
        QUrl tokenUrl(m_tokenUri);
        QNetworkRequest rq(tokenUrl);
        rq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
        QUrlQuery body;
        body.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
        body.addQueryItem(QStringLiteral("client_id"), m_clientId);
        body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
        body.addQueryItem(QStringLiteral("refresh_token"), m_refreshToken);
        const QByteArray postData = body.query(QUrl::FullyEncoded).toUtf8();
        QNetworkReply *rep = m_nam.post(rq, postData);
        connect(rep, &QNetworkReply::finished, this, [this, rep]() {
            const int status = httpStatusCode(rep);
            const QByteArray raw = rep->readAll();
            rep->deleteLater();
            qDebug() << "[GoogleCalendar][RefreshToken] HTTP" << status << "| QtError" << rep->error()
                     << rep->errorString();
            qDebug().noquote() << "[GoogleCalendar][RefreshToken] Body:" << QString::fromUtf8(raw);

            if (rep->error() != QNetworkReply::NoError) {
                if (rep->error() == QNetworkReply::HostNotFoundError || rep->error() == QNetworkReply::TimeoutError)
                    emit errorOccurred(QStringLiteral("Pas de connexion Internet (rafraîchissement token)."));
                else
                    emit errorOccurred(
                        QStringLiteral("Rafraîchissement token (HTTP %1) : %2").arg(status).arg(rep->errorString()));
                return;
            }
            const QJsonObject obj = QJsonDocument::fromJson(raw).object();
            if (obj.contains(QStringLiteral("error"))) {
                emit errorOccurred(QStringLiteral("Token révoqué ou invalide : reconnectez-vous."));
                return;
            }
            saveTokensFromJson(obj);
            requestEventsWithAccessToken();
        });
        return;
    }

    requestEventsWithAccessToken();
}

void GoogleCalendarService::requestEventsWithAccessToken()
{
    if (m_accessToken.isEmpty()) {
        emit errorOccurred(QStringLiteral("Access token vide."));
        return;
    }

    const QDateTime timeMin = QDateTime::currentDateTimeUtc().addDays(-7);
    const QDateTime timeMax = QDateTime::currentDateTimeUtc().addDays(60);

    QUrl url(QString::fromLatin1(kCalendarEventsEndpoint));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("singleEvents"), QStringLiteral("true"));
    q.addQueryItem(QStringLiteral("orderBy"), QStringLiteral("startTime"));
    q.addQueryItem(QStringLiteral("timeMin"), timeMin.toString(Qt::ISODateWithMs));
    q.addQueryItem(QStringLiteral("timeMax"), timeMax.toString(Qt::ISODateWithMs));
    url.setQuery(q);

    QNetworkRequest rq(url);
    rq.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + m_accessToken.toUtf8());
    qDebug() << "[GoogleCalendar][Events][Request] URL =" << url.toString(QUrl::FullyDecoded);

    QNetworkReply *rep = m_nam.get(rq);
    connect(rep, &QNetworkReply::finished, this, &GoogleCalendarService::onEventsReplyFinished);
}

void GoogleCalendarService::onEventsReplyFinished()
{
    auto *rep = qobject_cast<QNetworkReply *>(sender());
    if (!rep)
        return;
    const int status = httpStatusCode(rep);
    const QByteArray body = rep->readAll();
    qDebug() << "[GoogleCalendar][Events][Reply] HTTP" << status << "| QtError" << rep->error()
             << rep->errorString();
    qDebug().noquote() << "[GoogleCalendar][Events][Reply] Body:" << QString::fromUtf8(body);
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError) {
        if (status == 401) {
            emit errorOccurred(QStringLiteral("Google Calendar HTTP 401 : token expiré/invalide. Reconnectez-vous."));
            return;
        }
        if (status == 403) {
            emit errorOccurred(QStringLiteral("Google Calendar HTTP 403 : accès refusé (scope ou droits calendrier)."));
            return;
        }
        if (status == 404) {
            emit errorOccurred(QStringLiteral("Google Calendar HTTP 404 : calendrier introuvable (Calendar ID)."));
            return;
        }
        if (rep->error() == QNetworkReply::HostNotFoundError || rep->error() == QNetworkReply::TimeoutError
            || rep->error() == QNetworkReply::NetworkSessionFailedError) {
            emit errorOccurred(QStringLiteral("Pas de connexion Internet."));
        } else {
            emit errorOccurred(QStringLiteral("API Calendar (HTTP %1) : %2").arg(status).arg(rep->errorString()));
        }
        return;
    }

    const QJsonObject root = QJsonDocument::fromJson(body).object();
    if (root.contains(QStringLiteral("error"))) {
        const QJsonObject err = root.value(QStringLiteral("error")).toObject();
        emit errorOccurred(QStringLiteral("Google API : %1").arg(err.value(QStringLiteral("message")).toString()));
        return;
    }

    QVector<GoogleCalendarEventRow> rows;
    parseEventsJson(body, &rows);
    emit eventsReady(rows);
}

void GoogleCalendarService::parseEventsJson(const QByteArray &body, QVector<GoogleCalendarEventRow> *out) const
{
    if (!out)
        return;
    out->clear();
    const QJsonObject root = QJsonDocument::fromJson(body).object();
    const QJsonArray items = root.value(QStringLiteral("items")).toArray();
    for (const QJsonValue &v : items) {
        const QJsonObject ev = v.toObject();
        if (ev.value(QStringLiteral("status")).toString() == QStringLiteral("cancelled"))
            continue;

        const QString summary = ev.value(QStringLiteral("summary")).toString(QStringLiteral("(Sans titre)"));
        const QJsonObject start = ev.value(QStringLiteral("start")).toObject();
        QString when;
        QDate startDay;
        const QString dateOnly = start.value(QStringLiteral("date")).toString();
        if (!dateOnly.isEmpty()) {
            startDay = QDate::fromString(dateOnly, Qt::ISODate);
            when = startDay.isValid() ? startDay.toString(QStringLiteral("yyyy-MM-dd")) : dateOnly;
        } else {
            const QString iso = start.value(QStringLiteral("dateTime")).toString();
            QDateTime dt = QDateTime::fromString(iso, Qt::ISODateWithMs);
            if (!dt.isValid())
                dt = QDateTime::fromString(iso, Qt::ISODate);
            if (dt.isValid()) {
                const QDateTime local = dt.toLocalTime();
                startDay = local.date();
                when = local.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
            }
        }
        if (when.isEmpty())
            when = QStringLiteral("?");

        const QString transparency = ev.value(QStringLiteral("transparency")).toString();
        const bool isFree = (transparency == QStringLiteral("transparent"));

        GoogleCalendarEventRow row;
        row.displayText = QStringLiteral("%1 — %2").arg(when, summary);
        row.isBusy = !isFree;
        row.startDate = startDay;
        out->append(row);
    }
}

void GoogleCalendarService::createEvent(const QString &summary, const QDateTime &startUtc, const QDateTime &endUtc,
                                        const QString &description)
{
    QString err;
    if (!loadCredentials(&err)) {
        emit eventCreated(false, err);
        return;
    }
    if (m_refreshToken.isEmpty() && m_accessToken.isEmpty()) {
        const QString e = QStringLiteral("Non connecté : utilisez « Connecter Google Calendar ».");
        emit eventCreated(false, e);
        return;
    }
    if (!startUtc.isValid() || !endUtc.isValid() || endUtc <= startUtc) {
        const QString e = QStringLiteral("Dates invalides (fin doit être après le début).");
        emit eventCreated(false, e);
        return;
    }

    QJsonObject root;
    root.insert(QStringLiteral("summary"), summary);
    if (!description.trimmed().isEmpty())
        root.insert(QStringLiteral("description"), description.trimmed());

    QJsonObject startOb;
    startOb.insert(QStringLiteral("dateTime"), startUtc.toUTC().toString(Qt::ISODateWithMs));
    startOb.insert(QStringLiteral("timeZone"), QStringLiteral("UTC"));
    QJsonObject endOb;
    endOb.insert(QStringLiteral("dateTime"), endUtc.toUTC().toString(Qt::ISODateWithMs));
    endOb.insert(QStringLiteral("timeZone"), QStringLiteral("UTC"));
    root.insert(QStringLiteral("start"), startOb);
    root.insert(QStringLiteral("end"), endOb);

    const QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Compact);

    const auto sendPost = [this, payload]() {
        QUrl url(QString::fromLatin1(kCalendarEventsEndpoint));
        QNetworkRequest rq(url);
        rq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        rq.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + m_accessToken.toUtf8());
        qDebug() << "[GoogleCalendar][Create][Request] URL =" << url.toString(QUrl::FullyDecoded);
        QNetworkReply *rep = m_nam.post(rq, payload);
        connect(rep, &QNetworkReply::finished, this, &GoogleCalendarService::onCreateEventReplyFinished);
    };

    if (accessTokenNeedsRefresh()) {
        if (m_refreshToken.isEmpty()) {
            const QString e = QStringLiteral("Token expiré : reconnectez-vous.");
            emit eventCreated(false, e);
            return;
        }
        QUrl tokenUrl(m_tokenUri);
        QNetworkRequest rq(tokenUrl);
        rq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
        QUrlQuery body;
        body.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
        body.addQueryItem(QStringLiteral("client_id"), m_clientId);
        body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
        body.addQueryItem(QStringLiteral("refresh_token"), m_refreshToken);
        const QByteArray postData = body.query(QUrl::FullyEncoded).toUtf8();
        QNetworkReply *rep = m_nam.post(rq, postData);
        connect(rep, &QNetworkReply::finished, this, [this, rep, sendPost]() {
            rep->deleteLater();
            if (rep->error() != QNetworkReply::NoError) {
                emit eventCreated(false, QStringLiteral("Rafraîchissement token : %1").arg(rep->errorString()));
                return;
            }
            const QJsonObject obj = QJsonDocument::fromJson(rep->readAll()).object();
            if (obj.contains(QStringLiteral("error"))) {
                emit eventCreated(false, QStringLiteral("Token invalide : reconnectez-vous."));
                return;
            }
            saveTokensFromJson(obj);
            sendPost();
        });
        return;
    }

    sendPost();
}

void GoogleCalendarService::onCreateEventReplyFinished()
{
    auto *rep = qobject_cast<QNetworkReply *>(sender());
    if (!rep)
        return;
    const int status = httpStatusCode(rep);
    const QByteArray body = rep->readAll();
    qDebug() << "[GoogleCalendar][Create][Reply] HTTP" << status << "| QtError" << rep->error()
             << rep->errorString();
    qDebug().noquote() << "[GoogleCalendar][Create][Reply] Body:" << QString::fromUtf8(body);
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError) {
        if (status == 401) {
            emit eventCreated(false, QStringLiteral("Création événement refusée (HTTP 401) : reconnectez-vous."));
            return;
        }
        if (status == 403) {
            emit eventCreated(false, QStringLiteral("Création événement refusée (HTTP 403) : droits insuffisants."));
            return;
        }
        if (status == 404) {
            emit eventCreated(false, QStringLiteral("Création événement impossible (HTTP 404) : calendrier introuvable."));
            return;
        }
        emit eventCreated(false, QStringLiteral("Création événement (HTTP %1) : %2").arg(status).arg(rep->errorString()));
        return;
    }

    const QJsonObject obj = QJsonDocument::fromJson(body).object();
    if (obj.contains(QStringLiteral("error"))) {
        const QString msg = obj.value(QStringLiteral("error")).toObject().value(QStringLiteral("message")).toString();
        emit eventCreated(false, msg);
        return;
    }

    const QString id = obj.value(QStringLiteral("id")).toString();
    emit eventCreated(true, id.isEmpty() ? QStringLiteral("OK") : id);
}
