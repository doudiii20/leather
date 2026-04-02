#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Employe
{
private:
    QString cin;
    QString nom;
    QString prenom;
    QString sexe;
    double salaire;
    QString dateEmbauche;
    QString telephone;
    QString poste;
    QString adresse;
    QString email;

public:
    Employe();
    Employe(QString cin, QString nom, QString prenom,
            QString sexe, double salaire,
            QString dateEmbauche,
            QString telephone,
            QString poste,
            QString adresse,
            QString email);

    bool ajouter();
    bool modifier();
    bool supprimer(QString cin);
    QSqlQueryModel* afficher();
};

#endif
