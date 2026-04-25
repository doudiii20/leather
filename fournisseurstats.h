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

/// Une ligne fournisseur pour pilotage risque + qualité des données (pas de SQL ici).
struct FournisseurPilotageRow {
    QString code;
    QString nom;
    int fiabilite = 0; ///< interprété comme score 0–100 quand parsable
    int slaJours = 0;
    int commandes = 0;
    QString email;
    QString zone;
};

class FournisseurStatsCalculator
{
public:
    static FournisseurStatsSnapshot compute(QList<FournisseurStatsInput> rows);
    /// Résumé texte : répartition risque (fiabilité + SLA) + indicateurs qualité des fiches.
    static QString pilotageSummaryPlain(const QList<FournisseurPilotageRow> &rows);
};

#endif // FOURNISSEURSTATS_H
