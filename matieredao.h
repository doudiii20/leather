#ifndef MATIEREDAO_H
#define MATIEREDAO_H

#include <QList>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "matierepremiere.h"

class MatiereDAO
{
public:
    MatiereDAO();

    // =======================
    // 🔌 Connexion Oracle
    // =======================
    static bool connecterOracle(const QString &username = "ranim",
                                const QString &password = "107108",
                                const QString &host = "localhost",
                                int port = 1521,
                                const QString &sid = "XE");

    static bool isConnected();

    // =======================
    // ➕ CRUD
    // =======================
    bool ajouter(const MatierePremiere &m);
    bool modifier(const MatierePremiere &m);
    bool supprimer(int id);
    QList<MatierePremiere> afficherTous();

    // =======================
    // 🔽 Tri
    // =====================
    // champ : "RESERVE", "EPAISSEUR", "GAMME"
    // ordre : "ASC" ou "DESC"
    QList<MatierePremiere> trierPar(const QString &champ, const QString &ordre = "ASC");

    // =======================
    // 🔍 Recherche
    // =======================
    QList<MatierePremiere> rechercher(const QString &motCle);

    // =======================
    // 🎯 Filtres
    // =======================
    QList<MatierePremiere> filtrerDisponibles();
    QList<MatierePremiere> filtrerSeuilCritique(int seuil = 10);

    // =======================
    // 📄 Export
    // =======================
    bool exporterPDF(const QList<MatierePremiere> &liste, const QString &cheminFichier);
    bool exporterExcel(const QList<MatierePremiere> &liste, const QString &cheminFichier);

    // =======================
    // 📊 Statistiques
    // =======================
    QMap<QString, int> statsParType();

    // =======================
    // 🔢 ID auto
    // =======================
    int prochainId();
private:
    MatierePremiere fromQuery(const QSqlQuery &q);
    QList<MatierePremiere> executerRequete(const QString &sql, const QVariantList &params = {});
};

#endif // MATIEREDAO_H
