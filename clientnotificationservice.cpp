#include "clientnotificationservice.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <QtGlobal>

namespace {

/// Corrige des collages errones (ex. .../page.htmlquestionnaire) ou .../page.htmlclientId= sans « ? ».
QString normalizeSurveyUrlTemplate(QString s)
{
    s = s.trimmed();
    const int dotHtml = s.indexOf(QStringLiteral(".html"), 0, Qt::CaseInsensitive);
    if (dotHtml < 0)
        return s;
    const int afterHtml = dotHtml + 5; // ".html"
    if (afterHtml >= s.size())
        return s;
    QString rest = s.mid(afterHtml);
    if (rest.startsWith(QStringLiteral("questionnaire"), Qt::CaseInsensitive))
        return s.left(afterHtml);

    int k = afterHtml;
    while (k < s.size() && s.at(k).isSpace())
        ++k;
    if (k >= s.size())
        return s;
    const QChar c = s.at(k);
    if (c != QLatin1Char('?') && c != QLatin1Char('#')) {
        const QString tail = s.mid(k);
        if (tail.contains(QLatin1Char('=')) && !tail.startsWith(QLatin1Char('/')))
            return s.left(k) + QLatin1Char('?') + tail;
    }
    return s;
}

bool looksLikePrivateOrLocalHost(const QString &hostLower)
{
    if (hostLower.isEmpty())
        return true;
    if (hostLower == QLatin1String("localhost") || hostLower.endsWith(QLatin1String(".local")))
        return true;
    if (hostLower == QLatin1String("127.0.0.1") || hostLower == QLatin1String("::1"))
        return true;
    if (hostLower.startsWith(QLatin1String("192.168.")))
        return true;
    if (hostLower.startsWith(QLatin1String("10.")))
        return true;
    if (hostLower.startsWith(QLatin1String("172."))) {
        const QStringList p = hostLower.split(QLatin1Char('.'));
        if (p.size() >= 2) {
            bool ok = false;
            const int second = p.at(1).toInt(&ok);
            if (ok && second >= 16 && second <= 31)
                return true;
        }
    }
    return false;
}

QString envTrim(const char *name)
{
    return QString::fromUtf8(qgetenv(name)).trimmed();
}

QString readSurveyUrlFromSidecarFile()
{
    const QStringList fileNames = {QStringLiteral("leather_survey_url.txt"), QStringLiteral("survey_url.txt")};
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString curDir = QDir::currentPath();
    const QStringList dirs = {appDir,
                              QDir(appDir).filePath(QStringLiteral("Resources")),
                              curDir,
                              QDir(curDir).filePath(QStringLiteral("Resources"))};
    for (const QString &dir : dirs) {
        if (dir.isEmpty())
            continue;
        for (const QString &name : fileNames) {
            const QString path = QDir(dir).filePath(name);
            QFile f(path);
            if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith(QChar(0xFEFF)))
                    line = line.mid(1).trimmed();
                if (!line.isEmpty() && !line.startsWith(QLatin1Char('#')))
                    return normalizeSurveyUrlTemplate(line);
            }
        }
    }
    return {};
}

QString surveyTemplateFromSettings()
{
    QString tpl = normalizeSurveyUrlTemplate(envTrim("LEATHER_SURVEY_URL_TEMPLATE"));
    if (!tpl.isEmpty())
        return tpl;
    tpl = readSurveyUrlFromSidecarFile();
    if (!tpl.isEmpty())
        return tpl;
    QSettings s;
    s.beginGroup(QStringLiteral("ClientNotif"));
    tpl = normalizeSurveyUrlTemplate(s.value(QStringLiteral("surveyUrlTemplate")).toString().trimmed());
    // Ancien reglage (ouverture locale du HTML) : ne pas bloquer si un fichier HTTPS est ajoute plus tard.
    if (tpl.contains(QLatin1String("file://"), Qt::CaseInsensitive)
        || tpl.startsWith(QLatin1String("file:"), Qt::CaseInsensitive))
        return {};
    return tpl;
}
} // namespace

QString ClientNotificationService::normalizePhoneE164(const QString &raw, const QString &defaultCallingPrefix)
{
    QString s = raw;
    s.remove(QLatin1Char(' '));
    if (s.isEmpty())
        return {};
    if (s.startsWith(QLatin1String("00")))
        s = QLatin1Char('+') + s.mid(2);
    if (s.startsWith(QLatin1Char('+')))
        return s;
    while (s.startsWith(QLatin1Char('0')))
        s = s.mid(1);
    const QString pfx = defaultCallingPrefix.trimmed();
    if (!pfx.isEmpty()) {
        QString p = pfx;
        if (!p.startsWith(QLatin1Char('+')))
            p.prepend(QLatin1Char('+'));
        return p + s;
    }
    return {}; // exiger +indicatif dans la fiche ou passer LEATHER_PHONE_PREFIX (+216, +33, ...)
}

QString ClientNotificationService::surveyUrlFromTemplate(const QString &templateUrl, const ClientData &client)
{
    QString out = normalizeSurveyUrlTemplate(templateUrl.trimmed());
    auto enc = [](const QString &v) { return QString::fromUtf8(QUrl::toPercentEncoding(v)); };
    out.replace(QStringLiteral("{id}"), QString::number(client.id));
    out.replace(QStringLiteral("{nom}"), enc(client.nom));
    out.replace(QStringLiteral("{prenom}"), enc(client.prenom));
    out.replace(QStringLiteral("{email}"), enc(client.email));
    out = out.trimmed();

    // Garantir la presence du parametre email, meme si le modele ne contient pas {email}.
    QUrl url(out);
    if (url.isValid() && !url.isRelative()) {
        QUrlQuery q(url);
        if (!q.hasQueryItem(QStringLiteral("email")))
            q.addQueryItem(QStringLiteral("email"), client.email.trimmed());
        url.setQuery(q);
        out = url.toString(QUrl::FullyEncoded);
    }
    return out;
}

QString ClientNotificationService::surveyUrlForClient(const ClientData &client, QString *errorMessage)
{
    QString tpl = surveyTemplateFromSettings().trimmed();
    if (tpl.contains(QLatin1String("file://"), Qt::CaseInsensitive)
        || tpl.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Lien local (file://). Utilisez une URL https (fichier ou reglage app).");
        return {};
    }
    if (tpl.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral(
                "URL https absente (leather_survey_url.txt, LEATHER_SURVEY_URL_TEMPLATE ou reglage app).");
        return {};
    }
    const QString url = surveyUrlFromTemplate(tpl, client);
    const QUrl pu(url);
    if (!pu.isValid() || pu.isRelative() || pu.host().isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("URL invalide (modele ou marqueurs).");
        return {};
    }
    const QString sch = pu.scheme().toLower();
    if (sch != QLatin1String("http") && sch != QLatin1String("https")) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Utilisez http ou https.");
        return {};
    }
    if (url.contains(QLatin1String("file://"), Qt::CaseInsensitive)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("URL file:// refusee.");
        return {};
    }
    const QString hostLower = pu.host().toLower();
    if (looksLikePrivateOrLocalHost(hostLower)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("URL locale / privee : utiliser une adresse publique.");
        return {};
    }
    if (errorMessage)
        errorMessage->clear();
    return url;
}

QString ClientNotificationService::buildPresetMessage(SmsPreset preset, const ClientData &client,
                                                      const QString &codeRetrait)
{
    const QString prenom = client.prenom.trimmed().isEmpty() ? client.nom.trimmed() : client.prenom.trimmed();
    switch (preset) {
    case SmsPreset::RappelLivraison:
        return QStringLiteral("Bonjour %1, votre commande Leather House est en preparation / en route. "
                              "Vous serez informe(e) des que la livraison est confirmee. Merci pour votre confiance.")
            .arg(prenom);
    case SmsPreset::CodeRetrait:
        return QStringLiteral(
                   "Bonjour %1, code de retrait magasin Leather House : %2. Presentez ce SMS a l'accueil. Merci.")
            .arg(prenom, codeRetrait.trimmed());
    case SmsPreset::RelanceDouce:
        return QStringLiteral("Bonjour %1, petit rappel Leather House concernant un reglement en attente. "
                              "En cas de question, contactez-nous. Bonne journee.")
            .arg(prenom);
    }
    return {};
}

QString ClientNotificationService::buildSurveySmsBody(const ClientData &client, QString *errorMessage)
{
    QString err;
    const QString url = surveyUrlForClient(client, &err);
    if (url.isEmpty()) {
        if (errorMessage)
            *errorMessage = err;
        return {};
    }
    const QString prenom = client.prenom.trimmed().isEmpty() ? client.nom.trimmed() : client.prenom.trimmed();
    return QStringLiteral("Bonjour %1, merci pour votre achat chez Leather House. "
                          "Votre avis nous aide a progresser (1 min) : %2")
        .arg(prenom, url);
}

ClientNotificationService::ClientNotificationService(QObject *parent)
    : QObject(parent)
{
}
