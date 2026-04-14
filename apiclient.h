#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QString>

                                                   class QNetworkAccessManager;

/*
  Configuration (variables d'environnement du systeme ou Run de Qt Creator) :

  Chatbot gratuit (Groq - https://console.groq.com , compte et cle gratuits) :
    GROQ_API_KEY     = gsk_...
    GROQ_MODEL       = optionnel (defaut : llama-3.1-8b-instant)
    Endpoint utilise : https://api.groq.com/openai/v1/chat/completions

  Generation d'images (recommande pour fiabilite) :
    OPENAI_API_KEY   = sk-...
    Endpoint utilise : https://api.openai.com/v1/images/generations (gpt-image-1)
    HF_API_KEY       = hf_... (optionnel, backup)

  Alerte e-mail (Web3Forms - https://web3forms.com ) :
    Plus besoin d'IP ou de comptes complexes, l'email d'alerte arrivera directement sur
    la boite email reliee a la cle (Access Key).
*/

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    /** API compatible OpenAI (Groq, etc.) : URL complete .../chat/completions */
    void postChatCompletion(const QString &apiKey,
                            const QString &chatCompletionsUrl,
                            const QString &model,
                            const QString &systemPrompt,
                            const QString &userMessage);

    void postWeb3FormsMail(const QString &accessKey,
                           const QString &subject,
                           const QString &plainBody);

    void generateImageFromPrompt(const QString &promptText);

signals:
    void chatCompleted(const QString &replyText, const QString &errorMessage);
    void emailCompleted(bool success, const QString &errorMessage);
    void imageGenerated(const QByteArray &imageBytes, const QString &errorMessage);

private:
    QNetworkAccessManager *nam;
};

#endif

