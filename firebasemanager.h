#ifndef FIREBASEMANAGER_H
#define FIREBASEMANAGER_H

#include <QObject>
#include <QJsonArray>

class QNetworkAccessManager;
class MatierePremiere;

class FirebaseManager : public QObject
{
    Q_OBJECT

public:
    explicit FirebaseManager(const QString &apiKey,
                             const QString &projectId,
                             QNetworkAccessManager *networkManager,
                             QObject *parent = nullptr);

    bool isConfigured() const;
    void postMatierePremiere(const MatierePremiere &matiere);
    void fetchMatieresPremieres();

signals:
    void postFinished(bool success, const QString &detail);
    void fetchFinished(bool success, const QJsonArray &documents, const QString &detail);

private:
    QString buildCollectionUrl() const;

    QString m_apiKey;
    QString m_projectId;
    QNetworkAccessManager *m_networkManager = nullptr;
};

#endif // FIREBASEMANAGER_H
