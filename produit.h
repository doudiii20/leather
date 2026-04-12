#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

/// CRUD produits aligne sur le schema CommerceStore (PRODUITS + STOCK).
class Produit
{
public:
    Produit() {}
    Produit(int id,
            QString nom_produit,
            QString categorie,
            QString type_cuir,
            QString qualite,
            int quantite_stock,
            QString etat_produit,
            QDate date_fabrication,
            QString type_design,
            QString style);

    bool ajouter();
    bool modifier(int oldId, int newId);
    bool supprimer(int id);
    static QSqlQueryModel *afficher();
    static bool idExisteDeja(int id);
    /// Prochain ID libre (MAX(ID)+1), ou 1 si table vide.
    static int nextAvailableId();
    /// Decompresse le champ STYLE (cf. packStyle).
    static void unpackPackedStyle(const QString &packed,
                                  QString *qualite,
                                  QString *etat,
                                  QDate *dateFab,
                                  QString *typeDesign,
                                  QString *userStyle);

    static QString lastSqlError;

private:
    static QString packStyle(const QString &qualite,
                              const QString &etat,
                              const QDate &dateFab,
                              const QString &typeDesign,
                              const QString &styleUtilisateur);

    int id = 0;
    QString nom_produit;
    QString categorie;
    QString type_cuir;
    QString qualite;
    int quantite_stock = 0;
    QString etat_produit;
    QDate date_fabrication;
    QString type_design;
    QString style;
};

#endif
