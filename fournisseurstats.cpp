#include "fournisseurstats.h"

#include <QRegularExpression>

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

QString FournisseurStatsCalculator::pilotageSummaryPlain(const QList<FournisseurPilotageRow> &rows)
{
    if (rows.isEmpty())
        return QStringLiteral("Pilotage — Risque : aucun fournisseur.\nPilotage — Qualite des donnees : N/A.");

    static const QRegularExpression kEmailRx(
        QStringLiteral("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$"),
        QRegularExpression::CaseInsensitiveOption);

    int favorable = 0;
    int modere = 0;
    int aSurveiller = 0;
    int fichesCompletes = 0;
    int anomaliesEmail = 0;
    int anomaliesScore = 0;
    int anomaliesSla = 0;
    int anomaliesZone = 0;
    int anomaliesNom = 0;

    for (const FournisseurPilotageRow &r : rows) {
        const QString email = r.email.trimmed();
        const QString zone = r.zone.trimmed();
        const QString nom = r.nom.trimmed();

        const bool emailOk = kEmailRx.match(email).hasMatch();
        const bool scoreOk = r.fiabilite >= 0 && r.fiabilite <= 100;
        const bool slaOk = r.slaJours >= 1 && r.slaJours <= 365;
        const bool zoneOk = zone.size() >= 2;
        const bool nomOk = nom.size() >= 2;

        if (!emailOk)
            ++anomaliesEmail;
        if (!scoreOk)
            ++anomaliesScore;
        if (!slaOk)
            ++anomaliesSla;
        if (!zoneOk)
            ++anomaliesZone;
        if (!nomOk)
            ++anomaliesNom;

        if (emailOk && scoreOk && slaOk && zoneOk && nomOk)
            ++fichesCompletes;

        if (!scoreOk || r.fiabilite < 40 || r.slaJours > 10)
            ++aSurveiller;
        else if (r.fiabilite >= 80 && r.slaJours <= 7)
            ++favorable;
        else
            ++modere;
    }

    const int n = rows.size();
    const int pctComplet = n > 0 ? (fichesCompletes * 100) / n : 0;

    QString out;
    out += QStringLiteral("--- Analyste risque fournisseur ---\n");
    out += QStringLiteral("Favorable (score >=80 & SLA <=7j) : %1\n").arg(favorable);
    out += QStringLiteral("Modere : %1\n").arg(modere);
    out += QStringLiteral("A surveiller (score <40 ou SLA >10j ou score invalide) : %1\n").arg(aSurveiller);
    out += QStringLiteral("\n--- Data steward (qualite des fiches) ---\n");
    out += QStringLiteral("Fiches completes : %1% (%2 / %3)\n").arg(pctComplet).arg(fichesCompletes).arg(n);
    if (anomaliesEmail + anomaliesScore + anomaliesSla + anomaliesZone + anomaliesNom > 0) {
        out += QStringLiteral("Ecarts : email %1 | score hors 0-100 %2 | SLA hors 1-365j %3 | zone %4 | raison sociale %5\n")
                   .arg(anomaliesEmail)
                   .arg(anomaliesScore)
                   .arg(anomaliesSla)
                   .arg(anomaliesZone)
                   .arg(anomaliesNom);
    }
    return out.trimmed();
}
