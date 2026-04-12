#ifndef FOURNISSEURSTATS_H
#define FOURNISSEURSTATS_H

#include <QList>
#include <QString>

struct FournisseurStatsInput {
    QString id;
    QString nom;
    int commandes = 0;
};

struct FournisseurStatsSnapshot {
    int totalFournisseurs = 0;
    int totalCommandes = 0;
    QString plusActifNom;
    int plusActifCommandes = 0;
    QString moinsActifNom;
    int moinsActifCommandes = 0;
    QList<FournisseurStatsInput> ranked;
};

class FournisseurStatsCalculator
{
public:
    static FournisseurStatsSnapshot compute(QList<FournisseurStatsInput> rows);
};

#endif // FOURNISSEURSTATS_H
