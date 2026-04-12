#ifndef RECOMMENDATIONSERVICE_H
#define RECOMMENDATIONSERVICE_H

#include "clientdata.h"
#include <QList>
#include <QString>

struct RecommendationItem
{
    int productId = 0;
    QString nom;
    QString categorie;
    double prix = 0.0;
    int stock = 0;
    double score = 0.0;
    QString raison;
};

class RecommendationService
{
public:
    /// Top N recommandations pour le client (regles + historique + stock)
    static bool recommendForClient(const ClientData &client, int maxItems, QList<RecommendationItem> &out,
                                   QString *errorMessage = nullptr);

    static QString formatAsText(const QList<RecommendationItem> &items);
};

#endif // RECOMMENDATIONSERVICE_H
