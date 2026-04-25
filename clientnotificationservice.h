#ifndef CLIENTNOTIFICATIONSERVICE_H
#define CLIENTNOTIFICATIONSERVICE_H

#include "clientdata.h"
#include <QObject>
#include <QString>

/// Utilitaires de notification client (liens questionnaire, texte de messages).
class ClientNotificationService : public QObject
{
    Q_OBJECT

public:
    enum class SmsPreset { RappelLivraison, CodeRetrait, RelanceDouce };

    explicit ClientNotificationService(QObject *parent = nullptr);

    static QString normalizePhoneE164(const QString &raw, const QString &defaultCallingPrefix);
    static QString surveyUrlFromTemplate(const QString &templateUrl, const ClientData &client);
    static QString surveyUrlForClient(const ClientData &client, QString *errorMessage = nullptr);
    static QString buildPresetMessage(SmsPreset preset, const ClientData &client, const QString &codeRetrait);
    static QString buildSurveySmsBody(const ClientData &client, QString *errorMessage = nullptr);
};

#endif // CLIENTNOTIFICATIONSERVICE_H
