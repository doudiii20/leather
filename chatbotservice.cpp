#include "chatbotservice.h"
#include "chatbotreplyformat.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>
#include <QDebug>
#include <QtGlobal>

ChatbotService::ChatbotService(QNetworkAccessManager *networkManager, QObject *parent)
    : QObject(parent)
    , m_nam(networkManager)
{
}

void ChatbotService::ask(const QString &userMessage, const QString &systemContext, bool forceLocal)
{
    Q_UNUSED(forceLocal)
    m_pendingUserMessage = userMessage;
    m_pendingContext = systemContext;

    if (m_openAiReply) {
        m_openAiReply->abort();
        m_openAiReply->deleteLater();
        m_openAiReply = nullptr;
    }

    if (!m_nam) {
        emit replyReady(QStringLiteral("Service IA indisponible: gestionnaire reseau non initialise."));
        return;
    }
    const QString ollamaUrl = QString::fromUtf8(qgetenv("OLLAMA_CHAT_URL")).trimmed().isEmpty()
                                  ? QStringLiteral("http://localhost:11434/api/chat")
                                  : QString::fromUtf8(qgetenv("OLLAMA_CHAT_URL")).trimmed();
    const QString model = QString::fromUtf8(qgetenv("OLLAMA_MODEL")).trimmed();
    const QString useModel = model.isEmpty() ? QStringLiteral("llama3") : model;

    const QString sys =
        "Tu es l'assistant client d'une maroquinerie (Royal Leather House). "
        "Reponds en francais, de facon courte et professionnelle. "
        "Utilise uniquement le contexte fourni pour les faits (prix, stock, commandes). "
        "Si une information manque, dis-le clairement.\n\nContexte:\n"
        + systemContext;

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", sys}});
    messages.append(QJsonObject{{"role", "user"}, {"content", userMessage}});

    QJsonObject root;
    root["model"] = useModel;
    root["messages"] = messages;
    root["stream"] = false;

    QNetworkRequest req{QUrl(ollamaUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    req.setTransferTimeout(120000); // 120s: les modèles locaux peuvent être lents au premier run.
#endif

    m_openAiReply = m_nam->post(req, QJsonDocument(root).toJson(QJsonDocument::Compact));
    auto *timeoutTimer = new QTimer(m_openAiReply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, m_openAiReply, [reply = m_openAiReply]() {
        if (!reply || !reply->isRunning())
            return;
        reply->setProperty("timed_out", true);
        reply->abort();
    });
    timeoutTimer->start(120000);
    connect(m_openAiReply, &QNetworkReply::finished, this, &ChatbotService::onReplyFinished);
}

void ChatbotService::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != m_openAiReply) {
        if (reply)
            reply->deleteLater();
        return;
    }
    m_openAiReply = nullptr;

    const QByteArray raw = reply->readAll();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool timedOut = reply->property("timed_out").toBool();
        qDebug() << "[Chatbot][Ollama] error =" << reply->error() << reply->errorString()
                 << "| http =" << httpStatus << "| timedOut =" << timedOut;
        qDebug().noquote() << "[Chatbot][Ollama] raw:" << QString::fromUtf8(raw);

        if (timedOut) {
            emit replyReady(QStringLiteral(
                "Erreur : délai dépassé (120s). Le modèle local met trop de temps à répondre.\n"
                "Vérifiez Ollama, ou utilisez un modèle plus léger (ex: llama3.2:3b)."));
            return;
        }
        emit replyReady(QStringLiteral(
            "Échec Ollama local (%1). Vérifiez que le service tourne sur http://localhost:11434.")
                            .arg(reply->errorString()));
        return;
    }

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        emit replyReady(QStringLiteral("Echec Ollama local: reponse JSON invalide."));
        return;
    }

    const QJsonObject o = doc.object();
    const QString content = o.value("message").toObject().value("content").toString().trimmed();
    if (content.isEmpty()) {
        emit replyReady(QStringLiteral("Echec Ollama local: aucune reponse retournee."));
        return;
    }

    emit replyReady(formatChatbotDialogueLineBreaks(content));
}
