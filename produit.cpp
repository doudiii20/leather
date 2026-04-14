#include "produit.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QString Produit::lastSqlError = "";

bool Produit::idExisteDeja(int id)
{
    if (id <= 0) return false;

    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM PRODUITS WHERE ID = :id");
    q.bindValue(":id", id);

    if (!q.exec())
    {
        lastSqlError = q.lastError().text();
        qDebug() << "Erreur verification ID :" << lastSqlError;
        return false;
    }

    if (q.next()) return q.value(0).toInt() > 0;
    return false;
}

Produit::Produit(int id,
                 QString nom_produit,
                 QString categorie,
                 QString type_cuir,
                 QString qualite,
                 int quantite_stock,
                 QString etat_produit,
                 QDate date_fabrication,
                 QString type_design,
                 QString style)
{
    this->id = id;
    this->nom_produit = nom_produit;
    this->categorie = categorie;
    this->type_cuir = type_cuir;
    this->qualite = qualite;
    this->quantite_stock = quantite_stock;
    this->etat_produit = etat_produit;
    this->date_fabrication = date_fabrication;
    this->type_design = type_design;
    this->style = style;
}

// ================= AJOUTER =================
bool Produit::ajouter()
{
    if (id <= 0)
    {
        lastSqlError = "ID invalide. Saisir un ID > 0.";
        return false;
    }

    if (idExisteDeja(id))
    {
        lastSqlError = QString("ID %1 existe déjà (contrainte UNIQUE). Choisir un autre ID.")
                           .arg(id);
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO PRODUITS "
        "(ID, NOM_PRODUIT, CATEGORIE, TYPE_CUIR, QUALITE, "
        "QUANTITE_STOCK, ETAT_PRODUIT, DATE_FABRICATION, TYPE_DESIGN, STYLE) "
        "VALUES (:id, :nom, :cat, :cuir, :qualite, "
        ":qte, :etat, :datefab, :design, :style)"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom_produit);
    query.bindValue(":cat", categorie);
    query.bindValue(":cuir", type_cuir);
    query.bindValue(":qualite", qualite);
    query.bindValue(":qte", quantite_stock);
    query.bindValue(":etat", etat_produit);
    query.bindValue(":datefab", date_fabrication);
    query.bindValue(":design", type_design);
    query.bindValue(":style", style);

    if (!query.exec())
    {
        lastSqlError = query.lastError().text();
        qDebug() << "Erreur ajout :" << lastSqlError;
        return false;
    }
    return true;
}

// ================= MODIFIER =================
bool Produit::modifier(int oldId, int newId)
{
    if (oldId <= 0 || newId <= 0)
    {
        lastSqlError = "ID invalide.";
        return false;
    }

    if (newId != oldId && idExisteDeja(newId))
    {
        lastSqlError = QString("ID %1 existe déjà (contrainte UNIQUE). Choisir un autre ID.")
                           .arg(newId);
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE PRODUITS SET "
        "ID = :newid, "
        "NOM_PRODUIT = :nom, "
        "CATEGORIE = :cat, "
        "TYPE_CUIR = :cuir, "
        "QUALITE = :qualite, "
        "QUANTITE_STOCK = :qte, "
        "ETAT_PRODUIT = :etat, "
        "DATE_FABRICATION = :datefab, "
        "TYPE_DESIGN = :design, "
        "STYLE = :style "
        "WHERE ID = :oldid"
        );

    query.bindValue(":newid", newId);
    query.bindValue(":oldid", oldId);
    query.bindValue(":nom", nom_produit);
    query.bindValue(":cat", categorie);
    query.bindValue(":cuir", type_cuir);
    query.bindValue(":qualite", qualite);
    query.bindValue(":qte", quantite_stock);
    query.bindValue(":etat", etat_produit);
    query.bindValue(":datefab", date_fabrication);
    query.bindValue(":design", type_design);
    query.bindValue(":style", style);

    if (!query.exec())
    {
        lastSqlError = query.lastError().text();
        qDebug() << "Erreur modification :" << lastSqlError;
        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        lastSqlError = "Aucune ligne modifiée (ID introuvable).";
        return false;
    }
    return true;
}

// ================= SUPPRIMER =================
bool Produit::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM PRODUITS WHERE ID = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        lastSqlError = query.lastError().text();
        qDebug() << "Erreur suppression :" << lastSqlError;
        return false;
    }
    return true;
}

// ================= AFFICHER =================
QSqlQueryModel* Produit::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT ID, NOM_PRODUIT, CATEGORIE, TYPE_CUIR, QUALITE, "
        "QUANTITE_STOCK, ETAT_PRODUIT, DATE_FABRICATION, TYPE_DESIGN, STYLE "
        "FROM PRODUITS"
        );

    if (model->lastError().isValid())
        qDebug() << "Erreur affichage :" << model->lastError().text();

    return model;
}
