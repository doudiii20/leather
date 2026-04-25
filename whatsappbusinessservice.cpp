#include "whatsappbusinessservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QtGlobal>

namespace {

QString envTrim(const char *name)
{
    return QString::fromUtf8(qgetenv(name)).trimmed();
}

/// Expéditeur WhatsApp : `TWILIO_WHATSAPP_FROM` en priorité (ex. `whatsapp:+14155238886`), sinon `TWILIO_FROM_NUMBER` (+E164).
QString whatsAppFromEnvRaw()
{
    const QString w = envTrim("TWILIO_WHATSAPP_FROM");
    if (!w.isEmpty())
        return w;
    return envTrim("TWILIO_FROM_NUMBER");
}

QString onlyDigits(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (QChar c : s) {
        if (c.isDigit())
            out += c;
    }
    return out;
}

/// Un seul numéro / expéditeur : si la variable d'environnement contient deux valeurs (ex. sandbox + prod),
/// ne garder que la première (sinon onlyDigits() les colle et Twilio renvoie 21212).
QString firstWhatsAppFromToken(const QString &raw)
{
    QString t = raw.trimmed();
    if (t.startsWith(QStringLiteral("whatsapp:"), Qt::CaseInsensitive)) {
        const int prefixLen = int(QStringLiteral("whatsapp:").length());
        return QStringLiteral("whatsapp:%1")
            .arg(firstWhatsAppFromToken(t.mid(prefixLen).trimmed()));
    }
    for (int i = 0; i < t.size(); ++i) {
        const QChar c = t.at(i);
        if (c.isSpace() || c == QLatin1Char(',') || c == QLatin1Char(';'))
            return t.left(i).trimmed();
    }
    return t;
}

/// E.164 : au plus 15 chiffres (recommandation UIT), sans compter le préfixe pays en double.
bool isReasonableE164Digits(const QString &digits)
{
    return !digits.isEmpty() && digits.length() <= 15 && digits.at(0).isDigit();
}

/// Adresse Twilio WhatsApp : `whatsapp:+E164` (ex. whatsapp:+14155238886).
QString twilioWhatsAppAddress(const QString &raw)
{
    const QString s = firstWhatsAppFromToken(raw);
    if (s.isEmpty())
        return {};
    if (s.startsWith(QStringLiteral("whatsapp:"), Qt::CaseInsensitive)) {
        QString rest = s.mid(int(QStringLiteral("whatsapp:").length())).trimmed();
        if (rest.startsWith(QLatin1Char('+'))) {
            const QString d = onlyDigits(rest.mid(1));
            if (!isReasonableE164Digits(d))
                return {};
            return QStringLiteral("whatsapp:+%1").arg(d);
        }
        const QString d = onlyDigits(rest);
        if (!isReasonableE164Digits(d))
            return {};
        return QStringLiteral("whatsapp:+%1").arg(d);
    }
    QString digitsPart = s;
    if (digitsPart.startsWith(QLatin1Char('+')))
        digitsPart = digitsPart.mid(1);
    const QString d = onlyDigits(digitsPart);
    if (!isReasonableE164Digits(d))
        return {};
    return QStringLiteral("whatsapp:+%1").arg(d);
}

/// Corps `application/x-www-form-urlencoded` : les `+` dans les valeurs doivent être `%2B`,
/// sinon les serveurs les décodent comme des espaces (Twilio 21211 sur `whatsapp:+216…`).
QByteArray formUrlEncodePair(const QString &key, const QString &value)
{
    return QUrl::toPercentEncoding(key) + QByteArrayLiteral("=") + QUrl::toPercentEncoding(value);
}

QString twilioErrorSummary(const QByteArray &raw)
{
    QJsonParseError pe{};
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (!doc.isObject())
        return {};
    const QJsonObject o = doc.object();
    const int code = o.value(QStringLiteral("code")).toInt();
    const QString msg = o.value(QStringLiteral("message")).toString().trimmed();
    if (msg.isEmpty())
        return {};
    if (code != 0)
        return QStringLiteral("[Twilio %1] %2").arg(code).arg(msg);
    return msg;
}

} // namespace

WhatsAppBusinessService::WhatsAppBusinessService(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
}

bool WhatsAppBusinessService::isConfigured(QString *missingHint)
{
    const QString sid = envTrim("TWILIO_ACCOUNT_SID");
    if (sid.isEmpty()) {
        if (missingHint)
            *missingHint = QStringLiteral("TWILIO_ACCOUNT_SID manquant (identifiant compte Twilio).");
        return false;
    }
    const QString token = envTrim("TWILIO_AUTH_TOKEN");
    if (token.isEmpty()) {
        if (missingHint)
            *missingHint = QStringLiteral("TWILIO_AUTH_TOKEN manquant.");
        return false;
    }
    const QString fromRaw = whatsAppFromEnvRaw();
    if (fromRaw.isEmpty()) {
        if (missingHint) {
            *missingHint = QStringLiteral(
                "TWILIO_WHATSAPP_FROM manquant (ex. whatsapp:+14155238886 pour le sandbox). "
                "Si ce numéro est déjà dans TWILIO_FROM_NUMBER au format +E164, vous pouvez laisser "
                "TWILIO_WHATSAPP_FROM vide pour le réutiliser.");
        }
        return false;
    }
    if (twilioWhatsAppAddress(fromRaw).isEmpty()) {
        if (missingHint) {
            *missingHint =
                QStringLiteral("TWILIO_WHATSAPP_FROM invalide : un seul numéro E.164 (ex. +14155238886), "
                               "15 chiffres max après le « + ». Vérifiez qu'il n'y a pas deux numéros collés "
                               "dans le fichier .env.");
        }
        return false;
    }
    if (missingHint)
        missingHint->clear();
    return true;
}

void WhatsAppBusinessService::sendTextMessage(const QString &toDigits, const QString &body, bool previewUrl)
{
    Q_UNUSED(previewUrl);

    QString hint;
    if (!isConfigured(&hint)) {
        emit sendFinished(false, hint);
        return;
    }
    if (!m_nam) {
        emit sendFinished(false, QStringLiteral("QNetworkAccessManager indisponible."));
        return;
    }

    const QString digits = onlyDigits(toDigits);
    if (digits.isEmpty() || !digits.at(0).isDigit()) {
        emit sendFinished(false, QStringLiteral("Numéro destinataire invalide (chiffres uniquement, sans '+')."));
        return;
    }

    if (m_reply) {
        QObject::disconnect(m_reply, nullptr, this, nullptr);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    const QString sid = envTrim("TWILIO_ACCOUNT_SID");
    const QString token = envTrim("TWILIO_AUTH_TOKEN");
    const QString fromAddr = twilioWhatsAppAddress(whatsAppFromEnvRaw());
    const QString toAddr = QStringLiteral("whatsapp:+%1").arg(digits);

    const QUrl url(QStringLiteral("https://api.twilio.com/2010-04-01/Accounts/%1/Messages.json").arg(sid));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    const QString credentials = QStringLiteral("%1:%2").arg(sid, token);
    const QByteArray basic = credentials.toUtf8().toBase64();
    req.setRawHeader("Authorization", QByteArrayLiteral("Basic ") + basic);

    const QByteArray postData = formUrlEncodePair(QStringLiteral("From"), fromAddr)
        + QByteArrayLiteral("&") + formUrlEncodePair(QStringLiteral("To"), toAddr)
        + QByteArrayLiteral("&") + formUrlEncodePair(QStringLiteral("Body"), body);

    qDebug() << "[Twilio WhatsApp] POST" << url.toString() << "To" << toAddr;
    m_reply = m_nam->post(req, postData);
    connect(m_reply, &QNetworkReply::finished, this, &WhatsAppBusinessService::onReplyFinished);
}

void WhatsAppBusinessService::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != m_reply) {
        if (reply)
            reply->deleteLater();
        return;
    }
    m_reply = nullptr;

    const QByteArray raw = reply->readAll();
    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto nerr = reply->error();
    const QString errStr = reply->errorString();
    reply->deleteLater();

    qDebug() << "[Twilio WhatsApp] HTTP" << code << "network:" << int(nerr) << errStr;
    qDebug() << "[Twilio WhatsApp] Response:" << raw;

    const QString twilioErr = twilioErrorSummary(raw);

    if (code >= 200 && code < 300 && nerr == QNetworkReply::NoError) {
        QJsonParseError pe{};
        const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
        QString msgId;
        if (doc.isObject())
            msgId = doc.object().value(QStringLiteral("sid")).toString();
        emit sendFinished(true,
                          msgId.isEmpty() ? QStringLiteral("Message envoyé.")
                                          : QStringLiteral("Message envoyé (id: %1).").arg(msgId));
        return;
    }

    QString detail = twilioErr;
    if (detail.isEmpty() && !raw.isEmpty())
        detail = QString::fromUtf8(raw.left(500));
    if (detail.isEmpty())
        detail = errStr;
    emit sendFinished(false,
                       QStringLiteral("HTTP %1\n%2").arg(QString::number(code), detail));
}
