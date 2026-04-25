#include "imagemanager.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
const int kApiTimeoutMs = 60000;
}

ImageManager::ImageManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void ImageManager::generateImage(const QString &prompt)
{
    const QString trimmed = prompt.trimmed();
    if (trimmed.isEmpty()) {
        emit errorOccurred("Please provide an image prompt.");
        return;
    }

    emit requestStarted("Generating image...");

    QNetworkRequest request(QUrl("http://127.0.0.1:7860/sdapi/v1/txt2img"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["prompt"] = trimmed;
    payload["steps"] = 20;
    payload["width"] = 512;
    payload["height"] = 512;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning())
            reply->abort();
    });
    timeoutTimer->start(kApiTimeoutMs);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray data = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            QString message = "Cannot reach Stable Diffusion API. Start WebUI/API on 127.0.0.1:7860.";
            if (reply->error() == QNetworkReply::OperationCanceledError)
                message = "Image generation timed out. Try a simpler prompt.";
            else if (!reply->errorString().isEmpty())
                message += "\nDetails: " + reply->errorString();

            emit errorOccurred(message);
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit errorOccurred("Invalid response from Stable Diffusion API.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        const QJsonArray images = doc.object().value("images").toArray();
        if (images.isEmpty()) {
            emit errorOccurred("No generated image received.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        QByteArray decoded = QByteArray::fromBase64(images.first().toString().toLatin1());
        QImage image;
        if (!image.loadFromData(decoded)) {
            emit errorOccurred("Failed to decode generated image data.");
            emit requestFinished();
            reply->deleteLater();
            return;
        }

        emit imageReady(image);
        emit descriptionReady("Image generated successfully.");
        emit requestFinished();
        reply->deleteLater();
    });
}
