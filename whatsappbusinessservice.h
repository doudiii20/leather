#ifndef WHATSAPPBUSINESSSERVICE_H
#define WHATSAPPBUSINESSSERVICE_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

/// Envoi de messages WhatsApp via l'API REST Twilio (canal whatsapp:).
/// Identifiants : TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN, et l'expéditeur WhatsApp
/// TWILIO_WHATSAPP_FROM (ex. whatsapp:+14155238886) ; si vide, TWILIO_FROM_NUMBER (+E164) est utilisé.
class WhatsAppBusinessService : public QObject
{
    Q_OBJECT
public:
    explicit WhatsAppBusinessService(QNetworkAccessManager *nam, QObject *parent = nullptr);

    static bool isConfigured(QString *missingHint = nullptr);

    /// `toDigits` : numéro international sans '+' (ex. 21626135083).
    /// `previewUrl` : conservé pour compatibilité avec l'appelant (Meta utilisait preview_url) ; ignoré côté Twilio.
    void sendTextMessage(const QString &toDigits, const QString &body, bool previewUrl = true);

signals:
    void sendFinished(bool ok, const QString &detail);

private:
    void onReplyFinished();

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_reply = nullptr;
};

#endif // WHATSAPPBUSINESSSERVICE_H
