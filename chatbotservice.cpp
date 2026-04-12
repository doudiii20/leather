#include "chatbotservice.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QtGlobal>

ChatbotService::ChatbotService(QNetworkAccessManager *networkManager, QObject *parent)
    : QObject(parent)
    , m_nam(networkManager)
{
}

QString ChatbotService::localFallback(const QString &userMessage, const QString &context) const
{
    const QString u = userMessage.toLower();

    if (u.contains("bonjour") || u.contains("salut") || u.contains("hello"))
        return "Bonjour ! Je suis l'assistant Royal Leather House. Posez-moi une question sur nos produits en cuir, les prix, le stock ou le suivi de commande.";

    if (u.contains("prix") || u.contains("cout") || u.contains("€") || u.contains("eur")) {
        return "Les prix du catalogue sont dans le contexte ci-dessous (nom | categorie | prix | stock). "
               "Indiquez le produit qui vous interesse pour plus de precision.";
    }

    if (u.contains("stock") || u.contains("disponib") || u.contains("rupture")) {
        return "La disponibilite est indiquee pour chaque article dans le contexte (colonne stock). "
               "Si un produit est en faible quantite, nous pouvons vous proposer une alternative assortie.";
    }

    if (u.contains("commande") || u.contains("suivi") || u.contains("livraison") || u.contains("expedition")) {
        return "Le suivi de vos commandes recentes figure dans le contexte. "
               "Pour une commande precise, indiquez son numero (#ID) ou la date.";
    }

    if (u.contains("portefeuille") || u.contains("ceinture") || u.contains("sac")) {
        return "Nous proposons des portefeuilles, ceintures et sacs en cuir. Les suggestions personnalisees "
               "apparaissent dans le panneau « Produits recommandes » selon votre profil.";
    }

    if (u.contains("merci"))
        return "Avec plaisir. Autre question sur nos articles ou votre commande ?";

    return "Je peux vous aider sur le catalogue (prix/stock), le suivi de commande et les assortiments cuir.\n\n"
           "Contexte actuel:\n"
        + context;
}

void ChatbotService::finishWithLocal(const QString &userMessage, const QString &context)
{
    emit replyReady(localFallback(userMessage, context));
}

void ChatbotService::ask(const QString &userMessage, const QString &systemContext)
{
    m_pendingUserMessage = userMessage;
    m_pendingContext = systemContext;

    if (m_openAiReply) {
        m_openAiReply->abort();
        m_openAiReply->deleteLater();
        m_openAiReply = nullptr;
    }

    const QString apiKey = QString::fromUtf8(qgetenv("OPENAI_API_KEY")).trimmed();
    if (apiKey.isEmpty() || !m_nam) {
        finishWithLocal(userMessage, systemContext);
        return;
    }

    const QString model = QString::fromUtf8(qgetenv("OPENAI_MODEL")).trimmed();
    const QString useModel = model.isEmpty() ? QStringLiteral("gpt-4o-mini") : model;

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
    root["temperature"] = 0.4;

    QNetworkRequest req(QUrl("https://api.openai.com/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    m_openAiReply = m_nam->post(req, QJsonDocument(root).toJson(QJsonDocument::Compact));
    connect(m_openAiReply, &QNetworkReply::finished, this, &ChatbotService::onOpenAiFinished);
}

void ChatbotService::onOpenAiFinished()
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
        emit replyReady(localFallback(m_pendingUserMessage, m_pendingContext)
                        + "\n\n(Mode secours: API IA indisponible.)");
        return;
    }

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        finishWithLocal(m_pendingUserMessage, m_pendingContext);
        return;
    }

    const QJsonObject o = doc.object();
    const QJsonArray choices = o.value("choices").toArray();
    if (choices.isEmpty()) {
        finishWithLocal(m_pendingUserMessage, m_pendingContext);
        return;
    }

    const QJsonObject msg = choices.at(0).toObject().value("message").toObject();
    const QString content = msg.value("content").toString().trimmed();
    if (content.isEmpty()) {
        finishWithLocal(m_pendingUserMessage, m_pendingContext);
        return;
    }

    emit replyReady(content);
}
