#include "commercestore.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

namespace {
static bool execSql(const QString &sql, QString *errorMessage)
{
    QSqlQuery q;
    if (q.exec(sql)) {
        return true;
    }
    if (errorMessage) {
        *errorMessage = q.lastError().text().trimmed();
        if (errorMessage->isEmpty()) {
            *errorMessage = "Erreur SQL.";
        }
    }
    return false;
}

static bool columnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME = UPPER(:t) AND COLUMN_NAME = UPPER(:c)");
    q.bindValue(":t", table);
    q.bindValue(":c", column);
    if (!q.exec() || !q.next()) {
        return false;
    }
    return q.value(0).toInt() > 0;
}
} // namespace

bool CommerceStore::ensureSchema(QString *errorMessage)
{
    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE PRODUITS ("
            "ID NUMBER PRIMARY KEY, SKU VARCHAR2(50) NOT NULL, NOM VARCHAR2(200) NOT NULL, "
            "CATEGORIE VARCHAR2(50) NOT NULL, TYPE_CUIR VARCHAR2(50), STYLE VARCHAR2(100), "
            "PRIX NUMBER(12,2) DEFAULT 0 NOT NULL, ACTIF NUMBER(1) DEFAULT 1 NOT NULL)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE STOCK ("
            "ID_PRODUIT NUMBER PRIMARY KEY, QTE_DISPONIBLE NUMBER DEFAULT 0 NOT NULL, "
            "QTE_RESERVEE NUMBER DEFAULT 0 NOT NULL, SEUIL_ALERTE NUMBER DEFAULT 5 NOT NULL, "
            "CONSTRAINT FK_STOCK_PRODUIT FOREIGN KEY (ID_PRODUIT) REFERENCES PRODUITS(ID) ON DELETE CASCADE)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE COMMANDES ("
            "ID NUMBER PRIMARY KEY, CLIENT_ID NUMBER NOT NULL, STATUT VARCHAR2(40) DEFAULT ''en_attente'' NOT NULL, "
            "DATE_COMMANDE DATE DEFAULT SYSDATE NOT NULL, TOTAL_TTC NUMBER(12,2) DEFAULT 0 NOT NULL, "
            "CANAL VARCHAR2(50), CONSTRAINT FK_CMD_CLIENT FOREIGN KEY (CLIENT_ID) REFERENCES CLIENT(ID) ON DELETE CASCADE)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE LIGNES_COMMANDE ("
            "ID NUMBER PRIMARY KEY, ID_COMMANDE NUMBER NOT NULL, ID_PRODUIT NUMBER NOT NULL, "
            "QUANTITE NUMBER DEFAULT 1 NOT NULL, PRIX_UNITAIRE NUMBER(12,2) NOT NULL, REMISE_LIGNE NUMBER(5,2) DEFAULT 0, "
            "CONSTRAINT FK_LIGNE_CMD FOREIGN KEY (ID_COMMANDE) REFERENCES COMMANDES(ID) ON DELETE CASCADE, "
            "CONSTRAINT FK_LIGNE_PROD FOREIGN KEY (ID_PRODUIT) REFERENCES PRODUITS(ID))'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE SUIVI_COMMANDE ("
            "ID NUMBER PRIMARY KEY, ID_COMMANDE NUMBER NOT NULL, STATUT VARCHAR2(40) NOT NULL, "
            "EVENT_TIME DATE DEFAULT SYSDATE NOT NULL, COMMENTAIRE VARCHAR2(400), "
            "CONSTRAINT FK_SUIVI_CMD FOREIGN KEY (ID_COMMANDE) REFERENCES COMMANDES(ID) ON DELETE CASCADE)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE INTERACTIONS_PRODUIT ("
            "ID NUMBER PRIMARY KEY, CLIENT_ID NUMBER NOT NULL, ID_PRODUIT NUMBER NOT NULL, "
            "TYPE_INTERACTION VARCHAR2(30) NOT NULL, POIDS NUMBER DEFAULT 1 NOT NULL, "
            "EVENT_TIME DATE DEFAULT SYSDATE NOT NULL, "
            "CONSTRAINT FK_INT_CLIENT FOREIGN KEY (CLIENT_ID) REFERENCES CLIENT(ID) ON DELETE CASCADE, "
            "CONSTRAINT FK_INT_PROD FOREIGN KEY (ID_PRODUIT) REFERENCES PRODUITS(ID) ON DELETE CASCADE)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    if (columnExists(QStringLiteral("PRODUITS"), QStringLiteral("ID"))
        && !columnExists(QStringLiteral("PRODUITS"), QStringLiteral("ACTIF"))) {
        if (!execSql(QStringLiteral("ALTER TABLE PRODUITS ADD (ACTIF NUMBER(1) DEFAULT 1 NOT NULL)"),
                       errorMessage)) {
            return false;
        }
    }

    return true;
}

void CommerceStore::seedDemoCatalogIfEmpty(QString *errorMessage)
{
    QSqlQuery cnt;
    if (!cnt.exec("SELECT COUNT(*) FROM PRODUITS") || !cnt.next()) {
        if (errorMessage)
            *errorMessage = cnt.lastError().text();
        return;
    }
    if (cnt.value(0).toInt() > 0) {
        return;
    }

    struct Seed {
        int id;
        const char *sku;
        const char *nom;
        const char *cat;
        const char *cuir;
        const char *style;
        double prix;
        int stock;
    };
    const Seed seeds[] = {
        {1, "PF-001", "Portefeuille cuir pleine fleur", "Portefeuille", "Vachette", "Classique", 89.90, 40},
        {2, "CE-002", "Ceinture artisanale marron", "Ceinture", "Vachette", "Boucle metal", 59.00, 55},
        {3, "SC-003", "Sac cabas cuir camel", "Sac", "Cuir grainé", "Grand format", 189.00, 15},
        {4, "PF-004", "Porte-cartes minimaliste", "Portefeuille", "Veau", "Slim", 45.00, 60},
        {5, "CE-005", "Ceinture noire premium", "Ceinture", "Veau", "Lisse", 72.50, 30},
        {6, "SC-006", "Sac bandoulière compact", "Sac", "Vachette", "Urban", 129.00, 22},
    };

    for (const Seed &s : seeds) {
        QSqlQuery ins;
        ins.prepare(
            "INSERT INTO PRODUITS (ID, SKU, NOM, CATEGORIE, TYPE_CUIR, STYLE, PRIX, ACTIF) "
            "VALUES (:id, :sku, :nom, :cat, :cuir, :style, :prix, 1)");
        ins.bindValue(":id", s.id);
        ins.bindValue(":sku", QString::fromUtf8(s.sku));
        ins.bindValue(":nom", QString::fromUtf8(s.nom));
        ins.bindValue(":cat", QString::fromUtf8(s.cat));
        ins.bindValue(":cuir", QString::fromUtf8(s.cuir));
        ins.bindValue(":style", QString::fromUtf8(s.style));
        ins.bindValue(":prix", s.prix);
        if (!ins.exec()) {
            if (errorMessage)
                *errorMessage = ins.lastError().text();
            return;
        }
        QSqlQuery st;
        st.prepare("INSERT INTO STOCK (ID_PRODUIT, QTE_DISPONIBLE, QTE_RESERVEE, SEUIL_ALERTE) VALUES (:id, :q, 0, 5)");
        st.bindValue(":id", s.id);
        st.bindValue(":q", s.stock);
        if (!st.exec()) {
            if (errorMessage)
                *errorMessage = st.lastError().text();
            return;
        }
    }
}

bool CommerceStore::loadActiveProductsWithStock(QList<ProductRow> &out, QString *errorMessage)
{
    out.clear();
    const bool hasActif = columnExists(QStringLiteral("PRODUITS"), QStringLiteral("ACTIF"));
    const QString sql = hasActif
        ? QStringLiteral(
              "SELECT P.ID, P.SKU, P.NOM, P.CATEGORIE, P.TYPE_CUIR, P.STYLE, P.PRIX, P.ACTIF, "
              "NVL(S.QTE_DISPONIBLE,0) FROM PRODUITS P "
              "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID WHERE NVL(P.ACTIF,1)=1 ORDER BY P.ID")
        : QStringLiteral(
              "SELECT P.ID, P.SKU, P.NOM, P.CATEGORIE, P.TYPE_CUIR, P.STYLE, P.PRIX, "
              "1 AS ACTIF, NVL(S.QTE_DISPONIBLE,0) FROM PRODUITS P "
              "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID ORDER BY P.ID");
    QSqlQuery q;
    if (!q.exec(sql)) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    while (q.next()) {
        ProductRow r;
        r.id = q.value(0).toInt();
        r.sku = q.value(1).toString();
        r.nom = q.value(2).toString();
        r.categorie = q.value(3).toString();
        r.typeCuir = q.value(4).toString();
        r.style = q.value(5).toString();
        r.prix = q.value(6).toDouble();
        r.actif = q.value(7).toInt() != 0;
        r.qteDisponible = q.value(8).toInt();
        out.append(r);
    }
    return true;
}

bool CommerceStore::loadCategoriesPurchasedByClient(int clientId, QSet<QString> &categories, QString *errorMessage)
{
    categories.clear();
    QSqlQuery q;
    q.prepare(
        "SELECT DISTINCT P.CATEGORIE FROM LIGNES_COMMANDE L "
        "JOIN COMMANDES C ON C.ID = L.ID_COMMANDE "
        "JOIN PRODUITS P ON P.ID = L.ID_PRODUIT "
        "WHERE C.CLIENT_ID = :cid");
    q.bindValue(":cid", clientId);
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    while (q.next()) {
        categories.insert(q.value(0).toString());
    }
    return true;
}

bool CommerceStore::loadOrdersForClient(int clientId, QList<OrderSummary> &out, QString *errorMessage)
{
    out.clear();
    QSqlQuery q;
    q.prepare(
        "SELECT ID, CLIENT_ID, STATUT, TO_CHAR(DATE_COMMANDE, 'YYYY-MM-DD HH24:MI'), TOTAL_TTC, NVL(CANAL,'') "
        "FROM COMMANDES WHERE CLIENT_ID=:cid ORDER BY DATE_COMMANDE DESC");
    q.bindValue(":cid", clientId);
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    while (q.next()) {
        OrderSummary o;
        o.idCommande = q.value(0).toInt();
        o.clientId = q.value(1).toInt();
        o.statut = q.value(2).toString();
        o.dateCommande = q.value(3).toString();
        o.totalTtc = q.value(4).toDouble();
        o.canal = q.value(5).toString();
        out.append(o);
    }
    return true;
}

bool CommerceStore::loadLatestOrderIdForClient(int clientId, int &orderId, QString *errorMessage)
{
    orderId = 0;
    QSqlQuery q;
    q.prepare("SELECT MAX(ID) FROM COMMANDES WHERE CLIENT_ID=:cid");
    q.bindValue(":cid", clientId);
    if (!q.exec() || !q.next()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    orderId = q.value(0).toInt();
    return true;
}

bool CommerceStore::loadTrackingForOrder(int orderId, QList<TrackingEvent> &out, QString *errorMessage)
{
    out.clear();
    QSqlQuery q;
    q.prepare(
        "SELECT STATUT, TO_CHAR(EVENT_TIME, 'YYYY-MM-DD HH24:MI'), NVL(COMMENTAIRE,'') "
        "FROM SUIVI_COMMANDE WHERE ID_COMMANDE=:id ORDER BY EVENT_TIME DESC");
    q.bindValue(":id", orderId);
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    while (q.next()) {
        TrackingEvent e;
        e.statut = q.value(0).toString();
        e.eventTime = q.value(1).toString();
        e.commentaire = q.value(2).toString();
        out.append(e);
    }
    return true;
}

bool CommerceStore::productById(int productId, ProductRow &out, QString *errorMessage)
{
    const bool hasActif = columnExists(QStringLiteral("PRODUITS"), QStringLiteral("ACTIF"));
    const QString sql = hasActif
        ? QStringLiteral(
              "SELECT P.ID, P.SKU, P.NOM, P.CATEGORIE, P.TYPE_CUIR, P.STYLE, P.PRIX, P.ACTIF, NVL(S.QTE_DISPONIBLE,0) "
              "FROM PRODUITS P LEFT JOIN STOCK S ON S.ID_PRODUIT=P.ID WHERE P.ID=:id")
        : QStringLiteral(
              "SELECT P.ID, P.SKU, P.NOM, P.CATEGORIE, P.TYPE_CUIR, P.STYLE, P.PRIX, "
              "1 AS ACTIF, NVL(S.QTE_DISPONIBLE,0) "
              "FROM PRODUITS P LEFT JOIN STOCK S ON S.ID_PRODUIT=P.ID WHERE P.ID=:id");
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":id", productId);
    if (!q.exec() || !q.next()) {
        if (errorMessage)
            *errorMessage = q.lastError().text().isEmpty() ? QString("Produit introuvable.") : q.lastError().text();
        return false;
    }
    out.id = q.value(0).toInt();
    out.sku = q.value(1).toString();
    out.nom = q.value(2).toString();
    out.categorie = q.value(3).toString();
    out.typeCuir = q.value(4).toString();
    out.style = q.value(5).toString();
    out.prix = q.value(6).toDouble();
    out.actif = q.value(7).toInt() != 0;
    out.qteDisponible = q.value(8).toInt();
    return true;
}

QString CommerceStore::buildChatbotContext(int clientId, const QString &clientNom, const QString &clientEmail,
                                           QString *errorMessage)
{
    QString ctx;
    ctx += "Client: " + clientNom + " (ID " + QString::number(clientId) + ")\n";
    ctx += "Email: " + clientEmail + "\n\n";

    QList<ProductRow> prods;
    if (!loadActiveProductsWithStock(prods, errorMessage)) {
        ctx += "Catalogue: indisponible.\n";
        return ctx;
    }
    ctx += "Catalogue (nom | categorie | prix | stock):\n";
    for (const ProductRow &p : prods) {
        if (p.qteDisponible <= 0)
            continue;
        ctx += "- " + p.nom + " | " + p.categorie + " | " + QString::number(p.prix, 'f', 2) + " EUR | stock "
            + QString::number(p.qteDisponible) + "\n";
    }

    QList<OrderSummary> orders;
    if (loadOrdersForClient(clientId, orders, nullptr) && !orders.isEmpty()) {
        ctx += "\nCommandes recentes:\n";
        for (int i = 0; i < orders.size() && i < 5; ++i) {
            const OrderSummary &o = orders.at(i);
            ctx += "- #" + QString::number(o.idCommande) + " " + o.statut + " " + o.dateCommande + " "
                + QString::number(o.totalTtc, 'f', 2) + " EUR\n";
        }
        int lastId = orders.first().idCommande;
        QList<TrackingEvent> tr;
        if (loadTrackingForOrder(lastId, tr, nullptr) && !tr.isEmpty()) {
            ctx += "\nDernier suivi commande #" + QString::number(lastId) + ":\n";
            for (const TrackingEvent &e : tr) {
                ctx += "  * " + e.statut + " @ " + e.eventTime;
                if (!e.commentaire.isEmpty())
                    ctx += " — " + e.commentaire;
                ctx += "\n";
            }
        }
    } else {
        ctx += "\nAucune commande enregistree pour ce client.\n";
    }
    return ctx;
}

bool CommerceStore::createSampleOrder(int clientId, int productId, int quantite, int *outOrderId,
                                      QString *errorMessage)
{
    ProductRow p;
    if (!productById(productId, p, errorMessage))
        return false;
    if (quantite <= 0) {
        if (errorMessage)
            *errorMessage = "Quantite invalide.";
        return false;
    }
    if (p.qteDisponible < quantite) {
        if (errorMessage)
            *errorMessage = "Stock insuffisant.";
        return false;
    }

    QSqlQuery qMax;
    if (!qMax.exec("SELECT NVL(MAX(ID),0)+1 FROM COMMANDES") || !qMax.next()) {
        if (errorMessage)
            *errorMessage = qMax.lastError().text();
        return false;
    }
    const int cmdId = qMax.value(0).toInt();

    const double ligneTotal = p.prix * quantite;

    QSqlQuery insC;
    insC.prepare(
        "INSERT INTO COMMANDES (ID, CLIENT_ID, STATUT, DATE_COMMANDE, TOTAL_TTC, CANAL) "
        "VALUES (:id, :cid, 'confirmee', SYSDATE, :tot, 'app')");
    insC.bindValue(":id", cmdId);
    insC.bindValue(":cid", clientId);
    insC.bindValue(":tot", ligneTotal);
    if (!insC.exec()) {
        if (errorMessage)
            *errorMessage = insC.lastError().text();
        return false;
    }

    QSqlQuery qMaxL;
    if (!qMaxL.exec("SELECT NVL(MAX(ID),0)+1 FROM LIGNES_COMMANDE") || !qMaxL.next()) {
        if (errorMessage)
            *errorMessage = qMaxL.lastError().text();
        return false;
    }
    const int ligneId = qMaxL.value(0).toInt();

    QSqlQuery insL;
    insL.prepare(
        "INSERT INTO LIGNES_COMMANDE (ID, ID_COMMANDE, ID_PRODUIT, QUANTITE, PRIX_UNITAIRE, REMISE_LIGNE) "
        "VALUES (:id, :cmd, :pid, :q, :pu, 0)");
    insL.bindValue(":id", ligneId);
    insL.bindValue(":cmd", cmdId);
    insL.bindValue(":pid", productId);
    insL.bindValue(":q", quantite);
    insL.bindValue(":pu", p.prix);
    if (!insL.exec()) {
        if (errorMessage)
            *errorMessage = insL.lastError().text();
        return false;
    }

    QSqlQuery qMaxS;
    if (!qMaxS.exec("SELECT NVL(MAX(ID),0)+1 FROM SUIVI_COMMANDE") || !qMaxS.next()) {
        if (errorMessage)
            *errorMessage = qMaxS.lastError().text();
        return false;
    }
    const int suiviId = qMaxS.value(0).toInt();

    QSqlQuery insS;
    insS.prepare(
        "INSERT INTO SUIVI_COMMANDE (ID, ID_COMMANDE, STATUT, EVENT_TIME, COMMENTAIRE) "
        "VALUES (:id, :cmd, 'confirmee', SYSDATE, 'Commande creee depuis l''app')");
    insS.bindValue(":id", suiviId);
    insS.bindValue(":cmd", cmdId);
    if (!insS.exec()) {
        if (errorMessage)
            *errorMessage = insS.lastError().text();
        return false;
    }

    QSqlQuery updSt;
    updSt.prepare("UPDATE STOCK SET QTE_DISPONIBLE = QTE_DISPONIBLE - :q WHERE ID_PRODUIT=:pid");
    updSt.bindValue(":q", quantite);
    updSt.bindValue(":pid", productId);
    if (!updSt.exec()) {
        if (errorMessage)
            *errorMessage = updSt.lastError().text();
        return false;
    }

    if (outOrderId)
        *outOrderId = cmdId;
    return true;
}
