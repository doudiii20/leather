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

    /// Appelle Ollama en local (llama3) avec le contexte metier.
    void ask(const QString &userMessage, const QString &systemContext, bool forceLocal = false);

signals:
    void replyReady(const QString &assistantText);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_openAiReply = nullptr;
    QString m_pendingUserMessage;
    QString m_pendingContext;
};

#endif // CHATBOTSERVICE_H
