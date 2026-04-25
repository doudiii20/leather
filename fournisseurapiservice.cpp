#include "fournisseurapiservice.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QtGlobal>

FournisseurApiService::FournisseurApiService(QNetworkAccessManager *networkManager, QObject *parent)
    : QObject(parent)
    , m_nam(networkManager)
{
}

void FournisseurApiService::requestPerformanceAnalystApi(const QString &supplierContext)
{
    requestApi(ApiKind::PerformanceAnalyst, supplierContext);
}

void FournisseurApiService::requestRiskContinuityApi(const QString &supplierContext)
{
    requestApi(ApiKind::RiskContinuity, supplierContext);
}

void FournisseurApiService::requestApi(ApiKind kind, const QString &supplierContext)
{
    if (!m_nam) {
        emit apiReplyReady(apiName(kind), QStringLiteral("Service IA indisponible: gestionnaire reseau non initialise."));
        return;
    }
    const QString ollamaUrl = QString::fromUtf8(qgetenv("OLLAMA_CHAT_URL")).trimmed().isEmpty()
                                  ? QStringLiteral("http://localhost:11434/api/chat")
                                  : QString::fromUtf8(qgetenv("OLLAMA_CHAT_URL")).trimmed();
    const QString model = QString::fromUtf8(qgetenv("OLLAMA_MODEL")).trimmed();
    const QString useModel = model.isEmpty() ? QStringLiteral("llama3") : model;

    QJsonArray messages;
    messages.append(QJsonObject{
        {"role", "system"},
        {"content",
         QStringLiteral("Tu es consultant senior procurement pour une entreprise cuir. "
                        "Reponds en francais, concret, actionnable, et tres court.")}
    });
    messages.append(QJsonObject{{"role", "user"}, {"content", promptForKind(kind) + "\n\nContexte:\n" + supplierContext}});

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), useModel);
    payload.insert(QStringLiteral("stream"), false);
    payload.insert(QStringLiteral("messages"), messages);

    QNetworkRequest req{QUrl(ollamaUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_nam->post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    reply->setProperty("api_kind", static_cast<int>(kind));
    reply->setProperty("api_context", supplierContext);
    connect(reply, &QNetworkReply::finished, this, &FournisseurApiService::onReplyFinished);
}

void FournisseurApiService::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    const ApiKind kind = static_cast<ApiKind>(reply->property("api_kind").toInt());
    const QByteArray raw = reply->readAll();
    const bool hasError = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    if (hasError) {
        emit apiReplyReady(apiName(kind), QStringLiteral("Echec Ollama local: demarrez Ollama (llama3) sur http://localhost:11434."));
        return;
    }

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        emit apiReplyReady(apiName(kind), QStringLiteral("Echec Ollama local: reponse JSON invalide."));
        return;
    }

    const QString content = doc.object().value(QStringLiteral("message")).toObject().value(QStringLiteral("content")).toString().trimmed();
    if (content.isEmpty()) {
        emit apiReplyReady(apiName(kind), QStringLiteral("Echec Ollama local: aucune reponse retournee."));
        return;
    }
    emit apiReplyReady(apiName(kind), content);
}

QString FournisseurApiService::apiName(ApiKind kind) const
{
    if (kind == ApiKind::PerformanceAnalyst)
        return QStringLiteral("API Supplier Performance Analyst");
    return QStringLiteral("API Risk & Continuity Manager");
}

QString FournisseurApiService::promptForKind(ApiKind kind) const
{
    if (kind == ApiKind::PerformanceAnalyst) {
        return QStringLiteral(
            "Agis comme Supplier Performance Analyst. "
            "Donne: 1) score de fiabilite (0-100), 2) top 3 actions prioritaires, "
            "3) 3 KPI a suivre cette semaine.");
    }
    return QStringLiteral(
        "Agis comme Risk & Continuity Manager. "
        "Donne: 1) niveau de risque global, 2) top 3 risques + mitigation, "
        "3) plan de continuité en 4 points.");
}

