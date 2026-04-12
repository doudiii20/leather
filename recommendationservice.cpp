#include "recommendationservice.h"
#include "commercestore.h"

#include <QtGlobal>
#include <QSet>
#include <QMap>
#include <algorithm>

static double scoreProductForClient(const ProductRow &p, const ClientData &c, const QSet<QString> &pastCats)
{
    double s = 50.0;

    if (p.qteDisponible <= 0)
        return -1.0;

    // Fidelite / categorie client
    if (c.categorie == "VIP")
        s += 15.0;
    else if (c.categorie == "Gold")
        s += 10.0;
    else if (c.categorie == "Silver")
        s += 5.0;

    // Risque bas = plus de confiance pour upsell
    if (c.scoreRisque >= 70)
        s += 8.0;
    else if (c.scoreRisque < 40)
        s -= 10.0;

    // Historique categories achetees
    if (pastCats.contains(p.categorie))
        s += 20.0;

    // Cross-sell rules
    if (pastCats.contains("Portefeuille")) {
        if (p.categorie == "Ceinture" || p.categorie == "Sac")
            s += 12.0;
    }
    if (pastCats.contains("Sac")) {
        if (p.categorie == "Portefeuille" || p.categorie == "Ceinture")
            s += 10.0;
    }
    if (pastCats.contains("Ceinture")) {
        if (p.categorie == "Portefeuille")
            s += 8.0;
    }

    // Frequence / achats
    s += qMin(15.0, c.frequenceAchat * 0.5);
    s += qMin(10.0, c.totalAchats / 500.0);

    // Stock confortable
    if (p.qteDisponible >= 20)
        s += 5.0;
    else if (p.qteDisponible < 5)
        s -= 5.0;

    // Prix vs credit restant (soft)
    const double restant = c.limiteCredit - c.soldeCreditUtilise;
    if (p.prix > restant && restant >= 0 && c.limiteCredit > 0)
        s -= 15.0;

    return s;
}

static QString reasonFor(const ProductRow &p, const ClientData &c, const QSet<QString> &pastCats, double score)
{
    QStringList parts;
    if (pastCats.contains(p.categorie))
        parts << "deja achete dans cette famille";
    if (c.categorie == "VIP" || c.categorie == "Gold")
        parts << "aligne profil " + c.categorie;
    if (p.categorie == "Ceinture" && pastCats.contains("Portefeuille"))
        parts << "assorti portefeuille";
    if (p.categorie == "Sac" && pastCats.contains("Portefeuille"))
        parts << "complement sac";
    if (parts.isEmpty())
        parts << "pique d'interet cuir";
    parts << ("score " + QString::number(score, 'f', 1));
    return parts.join(" · ");
}

bool RecommendationService::recommendForClient(const ClientData &client, int maxItems,
                                               QList<RecommendationItem> &out, QString *errorMessage)
{
    out.clear();
    if (client.id <= 0) {
        if (errorMessage)
            *errorMessage = "Client invalide.";
        return false;
    }

    QList<ProductRow> products;
    if (!CommerceStore::loadActiveProductsWithStock(products, errorMessage))
        return false;

    QSet<QString> pastCats;
    CommerceStore::loadCategoriesPurchasedByClient(client.id, pastCats, nullptr);

    QList<QPair<double, ProductRow>> ranked;
    for (const ProductRow &p : products) {
        const double sc = scoreProductForClient(p, client, pastCats);
        if (sc < 0)
            continue;
        ranked.append(qMakePair(sc, p));
    }

    std::sort(ranked.begin(), ranked.end(),
              [](const QPair<double, ProductRow> &a, const QPair<double, ProductRow> &b) { return a.first > b.first; });

    const int n = qMin(maxItems, ranked.size());
    for (int i = 0; i < n; ++i) {
        const double sc = ranked.at(i).first;
        const ProductRow &p = ranked.at(i).second;
        RecommendationItem r;
        r.productId = p.id;
        r.nom = p.nom;
        r.categorie = p.categorie;
        r.prix = p.prix;
        r.stock = p.qteDisponible;
        r.score = sc;
        r.raison = reasonFor(p, client, pastCats, sc);
        out.append(r);
    }
    return true;
}

QString RecommendationService::formatAsText(const QList<RecommendationItem> &items)
{
    if (items.isEmpty())
        return "Aucune recommandation (stock vide ou donnees insuffisantes).";

    QString t = "Recommandations personnalisees:\n\n";
    int k = 1;
    for (const RecommendationItem &r : items) {
        t += QString::number(k++) + ". " + r.nom + " (" + r.categorie + ")\n";
        t += "   Prix: " + QString::number(r.prix, 'f', 2) + " EUR | Stock: " + QString::number(r.stock) + "\n";
        t += "   " + r.raison + "\n\n";
    }
    return t.trimmed();
}
