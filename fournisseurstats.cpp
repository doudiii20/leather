#include "fournisseurstats.h"

#include <algorithm>

FournisseurStatsSnapshot FournisseurStatsCalculator::compute(QList<FournisseurStatsInput> rows)
{
    FournisseurStatsSnapshot s;
    s.totalFournisseurs = rows.size();
    if (rows.isEmpty()) {
        s.plusActifNom = "-";
        s.moinsActifNom = "-";
        return s;
    }
    int sum = 0;
    int maxC = rows.first().commandes;
    int minC = rows.first().commandes;
    QString maxNom = rows.first().nom;
    QString minNom = rows.first().nom;
    for (const auto &r : rows) {
        sum += r.commandes;
        if (r.commandes > maxC) { maxC = r.commandes; maxNom = r.nom; }
        if (r.commandes < minC) { minC = r.commandes; minNom = r.nom; }
    }
    s.totalCommandes = sum;
    s.plusActifCommandes = maxC;
    s.plusActifNom = maxNom.isEmpty() ? "-" : maxNom;
    s.moinsActifCommandes = minC;
    s.moinsActifNom = minNom.isEmpty() ? "-" : minNom;
    std::sort(rows.begin(), rows.end(), [](const FournisseurStatsInput &a, const FournisseurStatsInput &b) {
        if (a.commandes != b.commandes) return a.commandes > b.commandes;
        return a.nom.localeAwareCompare(b.nom) < 0;
    });
    s.ranked = std::move(rows);
    return s;
}
