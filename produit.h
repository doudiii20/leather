#ifndef PRODUIT_H
#define PRODUIT_H
#include <QString>
#include <QDate>
#include <QSqlQueryModel>

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
    QSqlQueryModel* afficher();
    static bool idExisteDeja(int id);

    static QString lastSqlError;

private:
    int id;
    QString nom_produit;
    QString categorie;
    QString type_cuir;
    QString qualite;
    int quantite_stock;
    QString etat_produit;
    QDate date_fabrication;
    QString type_design;
    QString style;
};
#endif
