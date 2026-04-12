#include "produit.h"
#include "commercestore.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QString Produit::lastSqlError = "";

QString Produit::packStyle(const QString &qualite,
                           const QString &etat,
                           const QDate &dateFab,
                           const QString &typeDesign,
                           const QString &styleUtilisateur)
{
    const QString d = dateFab.isValid() ? dateFab.toString(Qt::ISODate) : QString();
    QString s = QStringLiteral("Q:%1|E:%2|D:%3|TD:%4|S:%5")
                    .arg(qualite, etat, d, typeDesign, styleUtilisateur);
    if (s.length() > 100)
        s = s.left(100);
    return s;
}

int Produit::nextAvailableId()
{
    QSqlQuery q;
    if (!q.exec(QStringLiteral("SELECT NVL(MAX(ID), 0) + 1 FROM PRODUITS")))
        return 1;
    if (q.next())
        return q.value(0).toInt();
    return 1;
}

void Produit::unpackPackedStyle(const QString &packed,
                                QString *qualite,
                                QString *etat,
                                QDate *dateFab,
                                QString *typeDesign,
                                QString *userStyle)
{
    if (qualite)
        *qualite = QString();
    if (etat)
        *etat = QString();
    if (dateFab)
        *dateFab = QDate();
    if (typeDesign)
        *typeDesign = QString();
    if (userStyle)
        *userStyle = QString();
    const QStringList parts = packed.split(QLatin1Char('|'));
    for (const QString &p : parts) {
        if (p.isEmpty())
            continue;
        const int eq = p.indexOf(QLatin1Char(':'));
        if (eq <= 0)
            continue;
        const QString key = p.left(eq);
        const QString val = p.mid(eq + 1);
        if (key == QLatin1String("Q") && qualite)
            *qualite = val;
        else if (key == QLatin1String("E") && etat)
            *etat = val;
        else if (key == QLatin1String("D") && dateFab)
            *dateFab = QDate::fromString(val, Qt::ISODate);
        else if (key == QLatin1String("TD") && typeDesign)
            *typeDesign = val;
        else if (key == QLatin1String("S") && userStyle)
            *userStyle = val;
    }
}

bool Produit::idExisteDeja(int id)
{
    if (id <= 0)
        return false;

    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM PRODUITS WHERE ID = :id"));
    q.bindValue(QStringLiteral(":id"), id);

    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        qDebug() << "Erreur verification ID :" << lastSqlError;
        return false;
    }

    if (q.next())
        return q.value(0).toInt() > 0;
    return false;
}

Produit::Produit(int id_,
                 QString nom_produit_,
                 QString categorie_,
                 QString type_cuir_,
                 QString qualite_,
                 int quantite_stock_,
                 QString etat_produit_,
                 QDate date_fabrication_,
                 QString type_design_,
                 QString style_)
    : id(id_)
    , nom_produit(nom_produit_)
    , categorie(categorie_)
    , type_cuir(type_cuir_)
    , qualite(qualite_)
    , quantite_stock(quantite_stock_)
    , etat_produit(etat_produit_)
    , date_fabrication(date_fabrication_)
    , type_design(type_design_)
    , style(style_)
{
}

bool Produit::ajouter()
{
    if (id <= 0) {
        lastSqlError = QStringLiteral("ID invalide. Saisir un ID > 0.");
        return false;
    }

    if (idExisteDeja(id)) {
        lastSqlError = QStringLiteral("ID %1 existe deja. Choisir un autre ID.").arg(id);
        return false;
    }

    const QString sku = QStringLiteral("SKU-%1").arg(id);
    const QString styleDb = packStyle(qualite, etat_produit, date_fabrication, type_design, style);
    const int actif = (etat_produit.contains(QStringLiteral("Défectueux"), Qt::CaseInsensitive)
                       || etat_produit.contains(QStringLiteral("Vendu"), Qt::CaseInsensitive))
        ? 0
        : 1;

    const QString nomCol = CommerceStore::produitsLibelleColumnPhysical();
    QSqlQuery q;
    q.prepare(QStringLiteral("INSERT INTO PRODUITS (ID, SKU, %1, CATEGORIE, TYPE_CUIR, STYLE, PRIX, ACTIF) "
                             "VALUES (:id, :sku, :nom, :cat, :cuir, :style, :prix, :actif)")
                  .arg(nomCol));
    q.bindValue(QStringLiteral(":id"), id);
    q.bindValue(QStringLiteral(":sku"), sku);
    q.bindValue(QStringLiteral(":nom"), nom_produit);
    q.bindValue(QStringLiteral(":cat"), categorie);
    q.bindValue(QStringLiteral(":cuir"), type_cuir);
    q.bindValue(QStringLiteral(":style"), styleDb);
    q.bindValue(QStringLiteral(":prix"), 0.0);
    q.bindValue(QStringLiteral(":actif"), actif);

    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }

    QSqlQuery qs;
    qs.prepare(
        QStringLiteral("INSERT INTO STOCK (ID_PRODUIT, QTE_DISPONIBLE, QTE_RESERVEE, SEUIL_ALERTE) "
                       "VALUES (:id, :q, 0, 5)"));
    qs.bindValue(QStringLiteral(":id"), id);
    qs.bindValue(QStringLiteral(":q"), quantite_stock);
    if (!qs.exec()) {
        lastSqlError = qs.lastError().text();
        QSqlQuery del;
        del.prepare(QStringLiteral("DELETE FROM PRODUITS WHERE ID = :id"));
        del.bindValue(QStringLiteral(":id"), id);
        del.exec();
        return false;
    }

    return true;
}

bool Produit::modifier(int oldId, int newId)
{
    if (oldId <= 0 || newId <= 0) {
        lastSqlError = QStringLiteral("ID invalide.");
        return false;
    }

    if (newId != oldId && idExisteDeja(newId)) {
        lastSqlError = QStringLiteral("ID %1 existe deja.").arg(newId);
        return false;
    }

    const QString sku = QStringLiteral("SKU-%1").arg(newId);
    const QString styleDb = packStyle(qualite, etat_produit, date_fabrication, type_design, style);
    const int actif = (etat_produit.contains(QStringLiteral("Défectueux"), Qt::CaseInsensitive)
                       || etat_produit.contains(QStringLiteral("Vendu"), Qt::CaseInsensitive))
        ? 0
        : 1;

    const QString nomCol = CommerceStore::produitsLibelleColumnPhysical();
    QSqlQuery q;
    q.prepare(QStringLiteral("UPDATE PRODUITS SET ID = :newid, SKU = :sku, %1 = :nom, CATEGORIE = :cat, "
                             "TYPE_CUIR = :cuir, STYLE = :style, PRIX = :prix, ACTIF = :actif WHERE ID = :oldid")
                  .arg(nomCol));
    q.bindValue(QStringLiteral(":newid"), newId);
    q.bindValue(QStringLiteral(":oldid"), oldId);
    q.bindValue(QStringLiteral(":sku"), sku);
    q.bindValue(QStringLiteral(":nom"), nom_produit);
    q.bindValue(QStringLiteral(":cat"), categorie);
    q.bindValue(QStringLiteral(":cuir"), type_cuir);
    q.bindValue(QStringLiteral(":style"), styleDb);
    q.bindValue(QStringLiteral(":prix"), 0.0);
    q.bindValue(QStringLiteral(":actif"), actif);

    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }

    if (newId != oldId) {
        QSqlQuery fk;
        fk.prepare(QStringLiteral("UPDATE STOCK SET ID_PRODUIT = :newid WHERE ID_PRODUIT = :oldid"));
        fk.bindValue(QStringLiteral(":newid"), newId);
        fk.bindValue(QStringLiteral(":oldid"), oldId);
        if (!fk.exec()) {
            lastSqlError = fk.lastError().text();
            return false;
        }
    }

    QSqlQuery qs;
    qs.prepare(QStringLiteral("UPDATE STOCK SET QTE_DISPONIBLE = :q WHERE ID_PRODUIT = :id"));
    qs.bindValue(QStringLiteral(":q"), quantite_stock);
    qs.bindValue(QStringLiteral(":id"), newId);
    if (!qs.exec()) {
        lastSqlError = qs.lastError().text();
        return false;
    }

    return true;
}

bool Produit::supprimer(int id_)
{
    QSqlQuery q;
    q.prepare(QStringLiteral("DELETE FROM PRODUITS WHERE ID = :id"));
    q.bindValue(QStringLiteral(":id"), id_);

    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel *Produit::afficher()
{
    const QString nomPhys = CommerceStore::produitsLibelleColumnPhysical();
    const QString nomSelect = QStringLiteral("P.%1 AS NOM_PRODUIT").arg(nomPhys);
    const bool hasActif = CommerceStore::produitsColumnExists(QStringLiteral("ACTIF"));
    const QString etatExpr = hasActif
        ? QStringLiteral("CASE WHEN NVL(P.ACTIF, 1) = 1 THEN 'En stock' ELSE 'Inactif' END AS ETAT_PRODUIT")
        : QStringLiteral("'En stock' AS ETAT_PRODUIT");
    const bool hasStyle = CommerceStore::produitsColumnExists(QStringLiteral("STYLE"));
    const QString styleExpr = hasStyle ? QStringLiteral("P.STYLE") : QStringLiteral("CAST(NULL AS VARCHAR2(100)) AS STYLE");

    auto *model = new QSqlQueryModel();
    model->setQuery(QStringLiteral(
                          "SELECT P.ID, %1, P.CATEGORIE, P.TYPE_CUIR, "
                          "'' AS QUALITE, "
                          "NVL(S.QTE_DISPONIBLE, 0) AS QUANTITE_STOCK, "
                          "%2, "
                          "CAST(NULL AS DATE) AS DATE_FABRICATION, "
                          "'' AS TYPE_DESIGN, "
                          "%3 "
                          "FROM PRODUITS P "
                          "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
                          "ORDER BY P.ID")
                          .arg(nomSelect, etatExpr, styleExpr));
    if (model->lastError().isValid())
        qDebug() << "Erreur affichage produits:" << model->lastError().text();
    return model;
}
