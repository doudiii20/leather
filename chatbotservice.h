#ifndef CHATBOTSERVICE_H
#define CHATBOTSERVICE_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class ChatbotService : public QObject
{
    Q_OBJECT
public:
    explicit ChatbotService(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    /// Appelle l'API si OPENAI_API_KEY est definie, sinon reponse locale.
    void ask(const QString &userMessage, const QString &systemContext);

signals:
    void replyReady(const QString &assistantText);

private slots:
    void onOpenAiFinished();

private:
    QString localFallback(const QString &userMessage, const QString &context) const;
    void finishWithLocal(const QString &userMessage, const QString &context);

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_openAiReply = nullptr;
    QString m_pendingUserMessage;
    QString m_pendingContext;
};

#endif // CHATBOTSERVICE_H
