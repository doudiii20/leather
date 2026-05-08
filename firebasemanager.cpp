#include "firebasemanager.h"

#include "matierepremiere.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

FirebaseManager::FirebaseManager(const QString &apiKey,
                                 const QString &projectId,
                                 QNetworkAccessManager *networkManager,
                                 QObject *parent)
    : QObject(parent)
    , m_apiKey(apiKey.trimmed())
    , m_projectId(projectId.trimmed())
    , m_networkManager(networkManager)
{
}

bool FirebaseManager::isConfigured() const
{
    return !m_apiKey.isEmpty() && !m_projectId.isEmpty() && m_apiKey != QStringLiteral("XXXXX")
           && m_projectId != QStringLiteral("XXXXX");
}

QString FirebaseManager::buildCollectionUrl() const
{
    return QStringLiteral("https://firestore.googleapis.com/v1/projects/%1/databases/(default)/documents/matieresPremieres?key=%2")
        .arg(m_projectId, m_apiKey);
}

void FirebaseManager::postMatierePremiere(const MatierePremiere &matiere)
{
    if (!m_networkManager || !isConfigured()) {
        emit postFinished(false, QStringLiteral("Configuration Firebase invalide (apiKey/projectId)."));
        return;
    }

    const QUrl url(buildCollectionUrl());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject fields;
    fields.insert(QStringLiteral("id"), QJsonObject{{QStringLiteral("integerValue"), QString::number(matiere.getId())}});
    fields.insert(QStringLiteral("reference"), QJsonObject{{QStringLiteral("stringValue"), matiere.getReference()}});
    fields.insert(QStringLiteral("nomCuir"), QJsonObject{{QStringLiteral("stringValue"), matiere.getNomCuir()}});
    fields.insert(QStringLiteral("typeCuir"), QJsonObject{{QStringLiteral("stringValue"), matiere.getTypeCuir()}});
    fields.insert(QStringLiteral("gamme"), QJsonObject{{QStringLiteral("stringValue"), matiere.getGamme()}});
    fields.insert(QStringLiteral("couleur"), QJsonObject{{QStringLiteral("stringValue"), matiere.getCouleur()}});
    fields.insert(QStringLiteral("statut"), QJsonObject{{QStringLiteral("stringValue"), matiere.getStatut()}});
    fields.insert(QStringLiteral("email"), QJsonObject{{QStringLiteral("stringValue"), matiere.getEmail()}});
    fields.insert(QStringLiteral("reserve"), QJsonObject{{QStringLiteral("integerValue"), QString::number(matiere.getReserve())}});

    const QJsonObject root{{QStringLiteral("fields"), fields}};
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(root).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray rawBody = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit postFinished(false,
                              QStringLiteral("POST Firebase echoue: %1 | %2")
                                  .arg(reply->errorString(), QString::fromUtf8(rawBody)));
            reply->deleteLater();
            return;
        }

        const QJsonDocument json = QJsonDocument::fromJson(rawBody);
        const QString docName = json.object().value(QStringLiteral("name")).toString();
        emit postFinished(true, docName.isEmpty() ? QStringLiteral("Document cree.") : docName);
        reply->deleteLater();
    });
}

void FirebaseManager::fetchMatieresPremieres()
{
    if (!m_networkManager || !isConfigured()) {
        emit fetchFinished(false, QJsonArray(), QStringLiteral("Configuration Firebase invalide (apiKey/projectId)."));
        return;
    }

    const QUrl url(buildCollectionUrl());
    QNetworkRequest request{url};
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray rawBody = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit fetchFinished(false,
                               QJsonArray(),
                               QStringLiteral("GET Firebase echoue: %1 | %2")
                                   .arg(reply->errorString(), QString::fromUtf8(rawBody)));
            reply->deleteLater();
            return;
        }

        const QJsonDocument json = QJsonDocument::fromJson(rawBody);
        const QJsonArray documents = json.object().value(QStringLiteral("documents")).toArray();
        emit fetchFinished(true, documents, QStringLiteral("%1 documents recuperes.").arg(documents.size()));
        reply->deleteLater();
    });
}
