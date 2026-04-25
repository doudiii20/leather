#ifndef CLIENT_H
#define CLIENT_H

#include "clientdata.h"
#include <QList>
#include <QPair>

struct CreditCheckResult
{
    bool allowed = false;
    double restant = 0.0;
    QString message;
};

class Client
{
public:
    static bool ensureSchema(QString *errorMessage = nullptr);

    static bool ajouter(ClientData &client, QString *errorMessage = nullptr);
    static bool modifier(const ClientData &client, QString *errorMessage = nullptr);
    static bool supprimer(int id, QString *errorMessage = nullptr);
    static bool chargerTous(QList<ClientData> &clients, QString *errorMessage = nullptr);
    static bool chargerParId(int id, ClientData &client, QString *errorMessage = nullptr);
    static bool rechercherParMotCle(const QString &motCle, QList<ClientData> &clients, QString *errorMessage = nullptr);
    static bool rechercherEtFiltrer(const QString &motCle,
                                    const QString &categorie,
                                    const QString &statut,
                                    const QString &niveauRisque,
                                    QList<ClientData> &clients,
                                    QString *errorMessage = nullptr);

    static bool enregistrerPaiement(int clientId, double montant, const QString &note, QString *errorMessage = nullptr);
    static bool historiquePaiements(int clientId, QList<QPair<QString, QString>> &rows, QString *errorMessage = nullptr);
    static CreditCheckResult verifierBlocageCommande(int clientId, double montantCommande, QString *errorMessage = nullptr);
    static QString genererExplicationIA(const ClientData &client);

    static bool validerClient(const ClientData &client, QString *errorMessage = nullptr);
    static void recalculerScoresEtCategorie(ClientData &client);

    static bool enregistrerCoordonneesGeo(int id, double latitude, double longitude, QString *errorMessage = nullptr);
};

#endif // CLIENT_H