#ifndef COMMERCESTORE_H
#define COMMERCESTORE_H

#include <QString>
#include <QList>
#include <QSet>

struct ProductRow
{
    int id = 0;
    QString sku;
    QString nom;
    QString categorie;
    QString typeCuir;
    QString style;
    double prix = 0.0;
    bool actif = true;
    int qteDisponible = 0;
};

struct OrderSummary
{
    int idCommande = 0;
    int clientId = 0;
    QString statut;
    QString dateCommande;
    double totalTtc = 0.0;
    QString canal;
};

struct TrackingEvent
{
    QString statut;
    QString eventTime;
    QString commentaire;
};

class CommerceStore
{
public:
    /// Colonne libelle sur PRODUITS : "NOM" (schema app) ou "NOM_PRODUIT" (anciennes bases).
    static QString produitsLibelleColumnPhysical();
    static bool produitsColumnExists(const QString &columnName);

    static bool ensureSchema(QString *errorMessage = nullptr);

    static bool loadActiveProductsWithStock(QList<ProductRow> &out, QString *errorMessage = nullptr);
    static bool loadCategoriesPurchasedByClient(int clientId, QSet<QString> &categories, QString *errorMessage = nullptr);
    static bool loadOrdersForClient(int clientId, QList<OrderSummary> &out, QString *errorMessage = nullptr);
    static bool loadLatestOrderIdForClient(int clientId, int &orderId, QString *errorMessage = nullptr);
    static bool loadTrackingForOrder(int orderId, QList<TrackingEvent> &out, QString *errorMessage = nullptr);
    static bool productById(int productId, ProductRow &out, QString *errorMessage = nullptr);

    /// Contexte texte pour le chatbot (produits + commandes client)
    static QString buildChatbotContext(int clientId, const QString &clientNom, const QString &clientEmail,
                                       QString *errorMessage = nullptr);

    /// Crée une commande simple + ligne + suivi initial (MVP) et retourne id commande
    static bool createSampleOrder(int clientId, int productId, int quantite, int *outOrderId = nullptr,
                                  QString *errorMessage = nullptr);

    static void seedDemoCatalogIfEmpty(QString *errorMessage = nullptr);
};

#endif // COMMERCESTORE_H
