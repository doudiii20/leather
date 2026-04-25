#include "chatmanager.h"
#include "chatbotreplyformat.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
const int kApiTimeoutMs = 120000;
}

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void ChatManager::sendTextMessage(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        emit errorOccurred("Please enter a message first.");
        return;
    }

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = trimmed;
    m_conversation.append(userMessage);

    emit requestStarted("Talking to llama3...");
    postToOllama("llama3", m_conversation, "Ollama text chat failed");
}

void ChatManager::sendImageAnalysis(const QString &imagePath, const QString &prompt)
{
    const QString encodedImage = readImageAsBase64(imagePath);
    if (encodedImage.isEmpty()) {
        emit errorOccurred("Could not read the selected image.");
        return;
    }

    const QString effectivePrompt = prompt.trimmed().isEmpty()
                                        ? QStringLiteral("Describe this image in detail.")
                                        : prompt.trimmed();

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = effectivePrompt;
    userMessage["images"] = QJsonArray{encodedImage};
    m_conversation.append(userMessage);

    emit requestStarted("Analyzing image with llava...");
    postToOllama("llava", m_conversation, "Ollama image analysis failed");
}

void ChatManager::sendImageGeneration(const QString &prompt)
{
    const QString effectivePrompt = prompt.trimmed();
    if (effectivePrompt.isEmpty()) {
        emit errorOccurred("Please enter an image prompt.");
        emit requestFinished();
        return;
    }

    const QString apiKey = qEnvironmentVariable("OPENAI_API_KEY").trimmed();
    if (apiKey.isEmpty()) {
        emit errorOccurred("OPENAI_API_KEY is missing. Define it to enable image generation.");
        emit requestFinished();
        return;
    }

    emit requestStarted("Generating image...");

    QNetworkRequest request(QUrl("https://api.openai.com/v1/images/generations"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject payload;
    payload["model"] = QStringLiteral("gpt-image-1");
    payload["prompt"] = effectivePrompt;
    payload["size"] = QStringLiteral("1024x1024");
    payload["quality"] = QStringLiteral("medium");

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });
    timeoutTimer->start(kApiTimeoutMs);

    connect(reply, &QNetworkReply::finished, this, [this, reply, effectivePrompt]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            QString message = QStringLiteral("Image generation request failed.");
            if (reply->error() == QNetworkReply::OperationCanceledError)
                message = QStringLiteral("Image generation timed out.");
            if (!reply->errorString().isEmpty())
                message += QStringLiteral("\nDetails: ") + reply->errorString();
            emit errorOccurred(message);
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit errorOccurred("Invalid image API JSON response.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QJsonArray items = doc.object().value("data").toArray();
        if (items.isEmpty()) {
            emit errorOccurred("Image API returned no image.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QJsonObject first = items.first().toObject();
        const QString b64 = first.value("b64_json").toString();
        if (!b64.isEmpty()) {
            const QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
            if (raw.isEmpty()) {
                emit errorOccurred("Image decoding failed.");
                emit requestFinished();
                reply->deleteLater();
                return;
            }
            emit imageGeneratedReady(raw, effectivePrompt);
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QString url = first.value("url").toString();
        if (url.isEmpty()) {
            emit errorOccurred("Image API returned neither base64 nor URL.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        fetchGeneratedImageFromUrl(QUrl(url), effectivePrompt);
        reply->deleteLater();
    });
}

const QJsonArray &ChatManager::conversation() const
{
    return m_conversation;
}

void ChatManager::postToOllama(const QString &model, const QJsonArray &messages, const QString &fallbackErrorContext)
{
    QNetworkRequest request(QUrl("http://localhost:11434/api/chat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    request.setTransferTimeout(kApiTimeoutMs);
#endif

    QJsonObject payload;
    payload["model"] = model;
    payload["messages"] = messages;
    payload["stream"] = false;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    timeoutTimer->start(kApiTimeoutMs);

    connect(reply, &QNetworkReply::finished, this, [this, reply, fallbackErrorContext]() {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();
        const bool timedOut = reply->error() == QNetworkReply::OperationCanceledError;

        if (reply->error() != QNetworkReply::NoError) {
            QString message = "Cannot reach local AI service. Make sure Ollama is running.";
            if (timedOut) {
                message = "Request timed out after 120s. The local model took too long to respond.";
            } else if (!reply->errorString().isEmpty()) {
                message += "\nDetails: " + reply->errorString();
            }
            if (!data.isEmpty()) {
                message += "\nRaw response: " + QString::fromUtf8(data);
            }
            emit errorOccurred(message);
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit errorOccurred(fallbackErrorContext + ": invalid JSON response.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QJsonObject root = doc.object();
        const QJsonObject messageObj = root.value("message").toObject();
        const QString content = messageObj.value("content").toString().trimmed();

        if (statusCode >= 400 || content.isEmpty()) {
            emit errorOccurred(fallbackErrorContext + ": empty or error response from API.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QString formatted = formatChatbotDialogueLineBreaks(content);

        QJsonObject assistantMessage;
        assistantMessage["role"] = "assistant";
        assistantMessage["content"] = formatted;
        m_conversation.append(assistantMessage);

        emit assistantReplyReady(formatted);
        emit requestFinished();
        reply->deleteLater();
    });
}

void ChatManager::fetchGeneratedImageFromUrl(const QUrl &url, const QString &prompt)
{
    if (!url.isValid()) {
        emit errorOccurred("Generated image URL is invalid.");
        emit requestFinished();
        return;
    }

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(url));
    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });
    timeoutTimer->start(kApiTimeoutMs);

    connect(reply, &QNetworkReply::finished, this, [this, reply, prompt]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError || data.isEmpty()) {
            emit errorOccurred("Could not download generated image.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }
        emit imageGeneratedReady(data, prompt);
        emit requestFinished();
        reply->deleteLater();
    });
}

QString ChatManager::readImageAsBase64(const QString &imagePath) const
{
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromLatin1(file.readAll().toBase64());
}
