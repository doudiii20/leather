#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <cmath>
#include <limits>
#include <QString>

struct ClientData
{
    int id = 0;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QString statutClient;
    QString canalAcquisition;
    QString modePaiementPrefere;
    double remiseAccordee = 0.0;
    double totalAchats = 0.0;
    int frequenceAchat = 0;
    int retardsPaiement = 0;
    QString categorie;
    double limiteCredit = 0.0;
    double soldeCreditUtilise = 0.0;
    int scoreClient = 0;
    int scoreRisque = 100;
    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();

    bool hasValidGeo() const
    {
        return std::isfinite(latitude) && std::isfinite(longitude);
    }
};

#endif // CLIENTDATA_H
