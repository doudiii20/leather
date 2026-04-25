#ifndef SMTPSENDER_H
#define SMTPSENDER_H

#include <QString>

namespace LeatherSmtp {

enum class Profile {
    Default,
    ResetPassword,
    Survey
};

/// Envoie un e-mail texte via SMTP Gmail (STARTTLS, smtp.gmail.com:587).
/// Configuration via variables d'environnement (voir smtpsender.cpp).
bool sendEmail(const QString &to, const QString &subject, const QString &plainBody, QString *errorMessage = nullptr);
bool sendEmail(const QString &to,
               const QString &subject,
               const QString &plainBody,
               Profile profile,
               QString *errorMessage = nullptr);

/// Envoie le code de reinitialisation de mot de passe.
bool sendResetCode(const QString &toEmail, const QString &code, QString *errorMessage = nullptr);
bool sendResetCode(const QString &toEmail, int code, QString *errorMessage = nullptr);
bool sendSurveyEmail(const QString &toEmail,
                     const QString &subject,
                     const QString &plainBody,
                     QString *errorMessage = nullptr);

/// Si `LEATHER_SMTP_DEV=1`, n'envoie pas réellement et retourne true (le code doit être montré ailleurs).
bool isDevSkipMode();

} // namespace LeatherSmtp

#endif // SMTPSENDER_H
