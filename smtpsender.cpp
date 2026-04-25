#include "smtpsender.h"

#include <QByteArray>
#include <QSslSocket>
#include <QRegularExpression>
#include <QtGlobal>

namespace LeatherSmtp {

bool isDevSkipMode()
{
    return QString::fromUtf8(qgetenv("LEATHER_SMTP_DEV")).trimmed() == QStringLiteral("1");
}

static QString envOr(const char *key, const QString &def)
{
    const QString v = QString::fromUtf8(qgetenv(key)).trimmed();
    return v.isEmpty() ? def : v;
}

static QString normalizedAppPassword(const QString &raw);

struct SmtpAccountConfig {
    QString host;
    int port = 587;
    QString user;
    QString pass;
    QString from;
    QString configHint;
};

static SmtpAccountConfig smtpAccountForProfile(Profile profile)
{
    SmtpAccountConfig cfg;

    const QString baseHost = envOr("LEATHER_SMTP_HOST", QStringLiteral("smtp.gmail.com"));
    const int basePortRaw = qEnvironmentVariableIntValue("LEATHER_SMTP_PORT");
    const int basePort = basePortRaw > 0 ? basePortRaw : 587;
    const QString baseUser = envOr("LEATHER_SMTP_USER", QString());
    const QString basePass = normalizedAppPassword(envOr("LEATHER_SMTP_PASS", QString()));
    const QString baseFrom = envOr("LEATHER_SMTP_FROM", baseUser);

    cfg.host = baseHost;
    cfg.port = basePort;
    cfg.user = baseUser;
    cfg.pass = basePass;
    cfg.from = baseFrom;
    cfg.configHint = QStringLiteral("LEATHER_SMTP_USER / LEATHER_SMTP_PASS");

    if (profile == Profile::ResetPassword) {
        cfg.host = envOr("LEATHER_SMTP_RESET_HOST", baseHost);
        const int p = qEnvironmentVariableIntValue("LEATHER_SMTP_RESET_PORT");
        cfg.port = p > 0 ? p : basePort;
        cfg.user = envOr("LEATHER_SMTP_RESET_USER", baseUser);
        cfg.pass = normalizedAppPassword(envOr("LEATHER_SMTP_RESET_PASS", basePass));
        cfg.from = envOr("LEATHER_SMTP_RESET_FROM", cfg.user.isEmpty() ? baseFrom : cfg.user);
        cfg.configHint = QStringLiteral("LEATHER_SMTP_RESET_USER / LEATHER_SMTP_RESET_PASS");
        return cfg;
    }

    if (profile == Profile::Survey) {
        cfg.host = envOr("LEATHER_SMTP_SURVEY_HOST", baseHost);
        const int p = qEnvironmentVariableIntValue("LEATHER_SMTP_SURVEY_PORT");
        cfg.port = p > 0 ? p : basePort;
        cfg.user = envOr("LEATHER_SMTP_SURVEY_USER", baseUser);
        cfg.pass = normalizedAppPassword(envOr("LEATHER_SMTP_SURVEY_PASS", basePass));
        cfg.from = envOr("LEATHER_SMTP_SURVEY_FROM", cfg.user.isEmpty() ? baseFrom : cfg.user);
        cfg.configHint = QStringLiteral("LEATHER_SMTP_SURVEY_USER / LEATHER_SMTP_SURVEY_PASS");
        return cfg;
    }

    return cfg;
}

static QString normalizedAppPassword(const QString &raw)
{
    QString p = raw.trimmed();
    // Google affiche parfois le mot de passe d'application en groupes avec espaces.
    p.remove(QLatin1Char(' '));
    p.remove(QLatin1Char('\t'));
    p.remove(QLatin1Char('\r'));
    p.remove(QLatin1Char('\n'));
    return p;
}

static QString readSmtpChunk(QSslSocket &sock, int waitMs = 8000)
{
    QString acc;
    if (sock.waitForReadyRead(waitMs))
        acc += QString::fromUtf8(sock.readAll());
    // petites lectures supplementaires si le serveur fragmente
    for (int i = 0; i < 8 && sock.waitForReadyRead(80); ++i)
        acc += QString::fromUtf8(sock.readAll());
    return acc;
}

static QString lastLineCode(const QString &buf)
{
    const QStringList lines = buf.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    return lines.isEmpty() ? QString() : lines.last().trimmed();
}

bool sendEmail(const QString &to,
               const QString &subject,
               const QString &plainBody,
               Profile profile,
               QString *errorMessage)
{
    if (isDevSkipMode())
        return true;

    const SmtpAccountConfig cfg = smtpAccountForProfile(profile);
    const QString host = cfg.host;
    const int port = cfg.port;
    const QString user = cfg.user;
    const QString pass = cfg.pass;
    const QString from = cfg.from;

    if (user.isEmpty() || pass.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral(
                "SMTP non configure. Variables requises pour ce type d'envoi: %1 (mot de passe d'application). "
                "Fallback global possible: LEATHER_SMTP_USER / LEATHER_SMTP_PASS. "
                "Optionnellement LEATHER_SMTP_HOST, LEATHER_SMTP_PORT (587 STARTTLS), LEATHER_SMTP_FROM. "
                "Tests sans envoi reel: LEATHER_SMTP_DEV=1.")
                                .arg(cfg.configHint);
        }
        return false;
    }

    QSslSocket sock;
    sock.setPeerVerifyMode(QSslSocket::VerifyPeer);
    sock.connectToHost(host, static_cast<quint16>(port));
    if (!sock.waitForConnected(15000)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Connexion SMTP: %1").arg(sock.errorString());
        return false;
    }

    auto fail = [&](const QString &msg) -> bool {
        if (errorMessage)
            *errorMessage = msg;
        return false;
    };

    QString r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("220")))
        return fail(QStringLiteral("Accueil SMTP inattendu: %1").arg(lastLineCode(r).left(200)));

    sock.write(QStringLiteral("EHLO leather.local\r\n").toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("250")))
        return fail(QStringLiteral("EHLO refuse: %1").arg(lastLineCode(r).left(200)));

    sock.write(QStringLiteral("STARTTLS\r\n").toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("220")))
        return fail(QStringLiteral("STARTTLS refuse: %1").arg(lastLineCode(r).left(200)));

    sock.startClientEncryption();
    if (!sock.waitForEncrypted(15000))
        return fail(QStringLiteral("Negociation TLS echouee: %1").arg(sock.errorString()));

    sock.write(QStringLiteral("EHLO leather.local\r\n").toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("250")))
        return fail(QStringLiteral("EHLO apres STARTTLS refuse: %1").arg(lastLineCode(r).left(200)));

    sock.write(QStringLiteral("AUTH LOGIN\r\n").toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!r.contains(QLatin1String("334")))
        return fail(QStringLiteral("AUTH LOGIN: %1").arg(lastLineCode(r).left(200)));

    sock.write(user.toUtf8().toBase64() + "\r\n");
    sock.flush();
    r = readSmtpChunk(sock);
    if (!r.contains(QLatin1String("334")))
        return fail(QStringLiteral("SMTP utilisateur: %1").arg(lastLineCode(r).left(200)));

    sock.write(pass.toUtf8().toBase64() + "\r\n");
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("235"))) {
        return fail(QStringLiteral("Authentification refusee: %1\n"
                                   "Verifiez les variables de ce profil (%2) ou le fallback LEATHER_SMTP_USER/LEATHER_SMTP_PASS "
                                   "(mot de passe d'application Google, 16 caracteres, sans espaces).")
                        .arg(lastLineCode(r).left(300), cfg.configHint));
    }

    sock.write(QStringLiteral("MAIL FROM:<%1>\r\n").arg(from).toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("250")))
        return fail(QStringLiteral("MAIL FROM: %1").arg(lastLineCode(r).left(200)));

    sock.write(QStringLiteral("RCPT TO:<%1>\r\n").arg(to.trimmed()).toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("250")))
        return fail(QStringLiteral("RCPT TO: %1").arg(lastLineCode(r).left(200)));

    sock.write(QStringLiteral("DATA\r\n").toUtf8());
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("354")))
        return fail(QStringLiteral("DATA: %1").arg(lastLineCode(r).left(200)));

    const QByteArray data = QStringLiteral("Subject: %1\r\n"
                                            "MIME-Version: 1.0\r\n"
                                            "Content-Type: text/plain; charset=UTF-8\r\n"
                                            "Content-Transfer-Encoding: 8bit\r\n"
                                            "\r\n"
                                            "%2\r\n"
                                            ".\r\n")
                                  .arg(subject, plainBody)
                                  .toUtf8();
    sock.write(data);
    sock.flush();
    r = readSmtpChunk(sock);
    if (!lastLineCode(r).startsWith(QLatin1String("250")))
        return fail(QStringLiteral("Envoi message: %1").arg(lastLineCode(r).left(300)));

    sock.write(QStringLiteral("QUIT\r\n").toUtf8());
    sock.flush();
    sock.waitForDisconnected(2000);
    return true;
}

bool sendEmail(const QString &to, const QString &subject, const QString &plainBody, QString *errorMessage)
{
    return sendEmail(to, subject, plainBody, Profile::Default, errorMessage);
}

bool sendResetCode(const QString &toEmail, const QString &code, QString *errorMessage)
{
    const QString subject = QStringLiteral("Code de reinitialisation");
    const QString body = QStringLiteral(
                             "Bonjour,\n\n"
                             "Votre code de verification est : %1\n\n"
                             "Ce code est temporaire. Si vous n'etes pas a l'origine de cette demande, ignorez ce message.\n")
                             .arg(code);
    return sendEmail(toEmail, subject, body, Profile::ResetPassword, errorMessage);
}

bool sendResetCode(const QString &toEmail, int code, QString *errorMessage)
{
    return sendResetCode(toEmail, QString::number(code), errorMessage);
}

bool sendSurveyEmail(const QString &toEmail,
                     const QString &subject,
                     const QString &plainBody,
                     QString *errorMessage)
{
    return sendEmail(toEmail, subject, plainBody, Profile::Survey, errorMessage);
}

} // namespace LeatherSmtp
