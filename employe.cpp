#include "employe.h"
#include <QSqlError>
#include <QDebug>

Employe::Employe()
{
    salaire = 0;
}

Employe::Employe(QString cin, QString nom, QString prenom, QString sexe, double salaire, QString dateEmbauche, QString telephone, QString poste, QString adresse, QString email)
{
    this->cin = cin;
    this->nom = nom;
    this->prenom = prenom;
    this->sexe = sexe;
    this->salaire = salaire;
    this->dateEmbauche = dateEmbauche;
    this->telephone = telephone;
    this->poste = poste;
    this->adresse = adresse;
    this->email = email;
}

bool Employe::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYE (CIN,NOM,PRENOM,SEXE,SALAIRE,DATE_EMBAUCHE,TELEPHONE,POSTE,ADRESSE,EMAIL) VALUES (:cin,:nom,:prenom,:sexe,:salaire,TO_DATE(:date,'DD/MM/YYYY'),:tel,:poste,:adresse,:email)");

    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":sexe", sexe);
    query.bindValue(":salaire", salaire);
    query.bindValue(":date", dateEmbauche);
    query.bindValue(":tel", telephone);
    query.bindValue(":poste", poste);
    query.bindValue(":adresse", adresse);
    query.bindValue(":email", email);

    if(query.exec())
        return true;
    else
    {
        qDebug() << query.lastError();
        return false;
    }
}

bool Employe::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET NOM=:nom,PRENOM=:prenom,SEXE=:sexe,SALAIRE=:salaire,DATE_EMBAUCHE=TO_DATE(:date,'DD/MM/YYYY'),TELEPHONE=:tel,POSTE=:poste,ADRESSE=:adresse,EMAIL=:email WHERE CIN=:cin");

    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":sexe", sexe);
    query.bindValue(":salaire", salaire);
    query.bindValue(":date", dateEmbauche);
    query.bindValue(":tel", telephone);
    query.bindValue(":poste", poste);
    query.bindValue(":adresse", adresse);
    query.bindValue(":email", email);

    if(query.exec())
        return true;
    else
    {
        qDebug() << query.lastError();
        return false;
    }
}

bool Employe::supprimer(QString cin)
{
    QSqlQuery query;
    query.prepare("DELETE FROM EMPLOYE WHERE CIN=:cin");
    query.bindValue(":cin", cin);

    if(query.exec())
        return true;
    else
    {
        qDebug() << query.lastError();
        return false;
    }
}

QSqlQueryModel* Employe::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM EMPLOYE");
    return model;
}
