#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QJsonArray>

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

class ChatManager : public QObject
{
    Q_OBJECT

public:
    explicit ChatManager(QObject *parent = nullptr);
    void sendTextMessage(const QString &text);
    void sendImageAnalysis(const QString &imagePath, const QString &prompt);
    void sendImageGeneration(const QString &prompt);
    const QJsonArray &conversation() const;

signals:
    void requestStarted(const QString &label);
    void assistantReplyReady(const QString &text);
    void imageGeneratedReady(const QByteArray &imageData, const QString &prompt);
    void errorOccurred(const QString &message);
    void requestFinished();

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QJsonArray m_conversation;

    void postToOllama(const QString &model, const QJsonArray &messages, const QString &fallbackErrorContext);
    void fetchGeneratedImageFromUrl(const QUrl &url, const QString &prompt);
    QString readImageAsBase64(const QString &imagePath) const;
};

#endif // CHATMANAGER_H
