#include "produit.h"
#include "commercestore.h"

#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDebug>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <memory>

QString Produit::lastSqlError = "";

int Produit::parseProductIdText(const QString &text)
{
    bool ok = false;
    const QString t = text.trimmed();
    int n = t.toInt(&ok);
    if (ok)
        return n;
    const double d = t.toDouble(&ok);
    return ok ? int(qRound(d)) : -1;
}

namespace {
const QString kMsgConnexionFermee = QStringLiteral(
    "Connexion base de donnees fermee. Verifiez le DSN ODBC, l'utilisateur et le mot de passe.");

bool connexionBdOuverte()
{
    return QSqlDatabase::database().isOpen();
}
} // namespace

QString Produit::packStyle(const QString &qualite,
                           const QString &etat,
                           const QDate &dateFab,
                           const QString &typeDesign,
                           const QString &styleUtilisateur)
{
    const QString d = dateFab.isValid() ? dateFab.toString(Qt::ISODate) : QString();
    QString s = QStringLiteral("Q:%1|E:%2|D:%3|TD:%4|S:%5")
                    .arg(qualite, etat, d, typeDesign, styleUtilisateur);
    // Compatibilite Oracle: la colonne PRODUITS.STYLE est souvent definie en VARCHAR2(50).
    // On tronque preventivement pour eviter ORA-12899 (value too large for column).
    if (s.length() > 50)
        s = s.left(50);
    return s;
}

int Produit::nextAvailableId()
{
    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return 0;
    }
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

QString Produit::packedStyleToTableDisplay(const QString &packed)
{
    if (packed.trimmed().isEmpty())
        return QString();

    QString q, e, td, s;
    QDate d;
    unpackPackedStyle(packed, &q, &e, &d, &td, &s);
    if (!s.trimmed().isEmpty())
        return s.trimmed();
    if (!td.trimmed().isEmpty())
        return td.trimmed();
    if (!e.trimmed().isEmpty())
        return e.trimmed();
    if (!q.trimmed().isEmpty())
        return q.trimmed();

    if (packed.size() > 48)
        return packed.left(45) + QStringLiteral("...");
    return packed;
}

bool Produit::idExisteDeja(int id)
{
    if (id <= 0)
        return false;

    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return false;
    }

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
    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return false;
    }

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
    const QString catCol = CommerceStore::produitsCategorieColumnPhysical();
    const QString cuirCol = CommerceStore::produitsTypeCuirColumnPhysical();
    const bool hasSku = CommerceStore::produitsColumnExists(QStringLiteral("SKU"));
    const bool hasStyle = CommerceStore::produitsColumnExists(QStringLiteral("STYLE"));
    const bool hasPrix = CommerceStore::produitsColumnExists(QStringLiteral("PRIX"));
    const bool hasActif = CommerceStore::produitsColumnExists(QStringLiteral("ACTIF"));

    QString cols = QStringLiteral("ID");
    QString vals = QStringLiteral(":id");
    if (hasSku) {
        cols += QStringLiteral(", SKU");
        vals += QStringLiteral(", :sku");
    }
    cols += QStringLiteral(", %1, %2, %3").arg(nomCol, catCol, cuirCol);
    vals += QStringLiteral(", :nom, :cat, :cuir");
    if (hasStyle) {
        cols += QStringLiteral(", STYLE");
        vals += QStringLiteral(", :style");
    }
    if (hasPrix) {
        cols += QStringLiteral(", PRIX");
        vals += QStringLiteral(", :prix");
    }
    if (hasActif) {
        cols += QStringLiteral(", ACTIF");
        vals += QStringLiteral(", :actif");
    }

    QSqlQuery q;
    q.prepare(QStringLiteral("INSERT INTO PRODUITS (%1) VALUES (%2)").arg(cols, vals));
    q.bindValue(QStringLiteral(":id"), id);
    if (hasSku)
        q.bindValue(QStringLiteral(":sku"), sku);
    q.bindValue(QStringLiteral(":nom"), nom_produit);
    q.bindValue(QStringLiteral(":cat"), categorie);
    q.bindValue(QStringLiteral(":cuir"), type_cuir);
    if (hasStyle)
        q.bindValue(QStringLiteral(":style"), styleDb);
    if (hasPrix)
        q.bindValue(QStringLiteral(":prix"), 0.0);
    if (hasActif)
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
    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return false;
    }

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
    const QString catCol = CommerceStore::produitsCategorieColumnPhysical();
    const QString cuirCol = CommerceStore::produitsTypeCuirColumnPhysical();
    const bool hasSku = CommerceStore::produitsColumnExists(QStringLiteral("SKU"));
    const bool hasStyle = CommerceStore::produitsColumnExists(QStringLiteral("STYLE"));
    const bool hasPrix = CommerceStore::produitsColumnExists(QStringLiteral("PRIX"));
    const bool hasActif = CommerceStore::produitsColumnExists(QStringLiteral("ACTIF"));

    QString setSql = QStringLiteral("ID = :newid");
    if (hasSku)
        setSql += QStringLiteral(", SKU = :sku");
    setSql += QStringLiteral(", %1 = :nom, %2 = :cat, %3 = :cuir").arg(nomCol, catCol, cuirCol);
    if (hasStyle)
        setSql += QStringLiteral(", STYLE = :style");
    if (hasPrix)
        setSql += QStringLiteral(", PRIX = :prix");
    if (hasActif)
        setSql += QStringLiteral(", ACTIF = :actif");

    QSqlQuery q;
    q.prepare(QStringLiteral("UPDATE PRODUITS SET %1 WHERE ID = :oldid").arg(setSql));
    q.bindValue(QStringLiteral(":newid"), newId);
    q.bindValue(QStringLiteral(":oldid"), oldId);
    if (hasSku)
        q.bindValue(QStringLiteral(":sku"), sku);
    q.bindValue(QStringLiteral(":nom"), nom_produit);
    q.bindValue(QStringLiteral(":cat"), categorie);
    q.bindValue(QStringLiteral(":cuir"), type_cuir);
    if (hasStyle)
        q.bindValue(QStringLiteral(":style"), styleDb);
    if (hasPrix)
        q.bindValue(QStringLiteral(":prix"), 0.0);
    if (hasActif)
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
    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return false;
    }

    // Oracle ORA-02292 si des lignes de commande referencent encore ID_PRODUIT (FK_LIGNE_PROD).
    // On supprime d'abord les lignes connues puis STOCK, puis PRODUITS.
    const int pid = id_;
    auto deleteFromTableIfExists = [pid](const QString &table) -> bool {
        QSqlQuery q;
        q.prepare(QStringLiteral("DELETE FROM %1 WHERE ID_PRODUIT = :id").arg(table));
        q.bindValue(QStringLiteral(":id"), pid);
        if (q.exec())
            return true;
        const QString err = q.lastError().text();
        if (err.contains(QStringLiteral("ORA-00942"), Qt::CaseInsensitive) || err.contains(QStringLiteral("942")))
            return true; // table absente sur ce schema
        Produit::lastSqlError = err;
        return false;
    };

    for (const QString &tbl : {QStringLiteral("LIGNES_COMMANDE"), QStringLiteral("LIGNE_PROD")}) {
        if (!deleteFromTableIfExists(tbl))
            return false;
    }

    QSqlQuery qStock;
    qStock.prepare(QStringLiteral("DELETE FROM STOCK WHERE ID_PRODUIT = :id"));
    qStock.bindValue(QStringLiteral(":id"), id_);
    if (!qStock.exec()) {
        const QString err = qStock.lastError().text();
        if (!err.contains(QStringLiteral("ORA-00942"), Qt::CaseInsensitive) && !err.contains(QStringLiteral("942"))) {
            lastSqlError = err;
            return false;
        }
    }

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
    auto *model = new QSqlQueryModel();
    if (!connexionBdOuverte()) {
        lastSqlError = kMsgConnexionFermee;
        return model;
    }

    const QString nomPhys = CommerceStore::produitsLibelleColumnPhysical();
    const QString nomSelect = QStringLiteral("P.%1 AS NOM_PRODUIT").arg(nomPhys);
    const QString catPhys = CommerceStore::produitsCategorieColumnPhysical();
    const QString cuirPhys = CommerceStore::produitsTypeCuirColumnPhysical();
    const QString catExpr = QStringLiteral("P.%1 AS CATEGORIE").arg(catPhys);
    const QString cuirExpr = QStringLiteral("P.%1 AS TYPE_CUIR").arg(cuirPhys);
    const bool hasActif = CommerceStore::produitsColumnExists(QStringLiteral("ACTIF"));
    const QString etatExpr = hasActif
        ? QStringLiteral("CASE WHEN NVL(P.ACTIF, 1) = 1 THEN 'En stock' ELSE 'Inactif' END AS ETAT_PRODUIT")
        : QStringLiteral("'En stock' AS ETAT_PRODUIT");
    const bool hasStyle = CommerceStore::produitsColumnExists(QStringLiteral("STYLE"));
    const bool hasQrCode = CommerceStore::produitsColumnExists(QStringLiteral("QR_CODE"));
    const QString styleExpr = hasStyle ? QStringLiteral("P.STYLE") : QStringLiteral("CAST(NULL AS VARCHAR2(100)) AS STYLE");
    const QString qrExpr = hasQrCode ? QStringLiteral("P.QR_CODE") : QStringLiteral("CAST(NULL AS VARCHAR2(400)) AS QR_CODE");

    model->setQuery(QStringLiteral(
                          "SELECT P.ID, %1, %2, %3, "
                          "'' AS QUALITE, "
                          "NVL(S.QTE_DISPONIBLE, 0) AS QUANTITE_STOCK, "
                          "%4, "
                          "CAST(NULL AS DATE) AS DATE_FABRICATION, "
                          "'' AS TYPE_DESIGN, "
                          "%5, %6 "
                          "FROM PRODUITS P "
                          "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
                          "ORDER BY P.ID")
                          .arg(nomSelect, catExpr, cuirExpr, etatExpr, styleExpr, qrExpr));
    if (model->lastError().isValid())
        qDebug() << "Erreur affichage produits:" << model->lastError().text();
    return model;
}

void Produit::clearEditorFields(const ProduitEditorWidgets &w)
{
    if (w.idProduit)
        w.idProduit->clear();
    if (w.nomProduit)
        w.nomProduit->clear();
    if (w.categorie)
        w.categorie->clear();
    if (w.typeCuir)
        w.typeCuir->clear();
    if (w.quantiteStock)
        w.quantiteStock->clear();
    if (w.style)
        w.style->clear();
    if (w.qualite)
        w.qualite->setCurrentIndex(0);
    if (w.etat)
        w.etat->setCurrentIndex(0);
    if (w.typeDesign)
        w.typeDesign->setCurrentIndex(0);
    if (w.dateFabrication)
        w.dateFabrication->setDate(QDate::currentDate());
}

int Produit::fillEditorFromTableRow(const ProduitEditorWidgets &w, QTableWidget *table, int row)
{
    if (!table || row < 0 || !table->item(row, 0))
        return -1;

    auto cell = [table, row](int col) -> QString {
        QTableWidgetItem *it = table->item(row, col);
        if (!it)
            return QString();
        if (col == 9) {
            const QString raw = it->data(Qt::UserRole).toString();
            if (!raw.isEmpty())
                return raw;
        }
        return it->text();
    };

    const int selectedId = parseProductIdText(cell(0));
    if (selectedId <= 0)
        return -1;
    if (w.idProduit)
        w.idProduit->setText(QString::number(selectedId));
    if (w.nomProduit)
        w.nomProduit->setText(cell(1));
    if (w.categorie)
        w.categorie->setText(cell(2));
    if (w.typeCuir)
        w.typeCuir->setText(cell(3));

    const QString packedStyle = cell(9);
    QString pq, ptd, ps;
    QDate pdf;
    unpackPackedStyle(packedStyle, &pq, nullptr, &pdf, &ptd, &ps);

    const QString qualiteAff = cell(4).isEmpty() ? pq : cell(4);
    if (w.qualite && !qualiteAff.isEmpty())
        w.qualite->setCurrentText(qualiteAff);

    bool qtyOk = false;
    const int qty = cell(5).toInt(&qtyOk);
    if (w.quantiteStock)
        w.quantiteStock->setText(qtyOk ? QString::number(qty) : cell(5));

    const QString etatAff = cell(6);
    if (w.etat && !etatAff.isEmpty()) {
        const int ix = w.etat->findText(etatAff);
        if (ix >= 0)
            w.etat->setCurrentIndex(ix);
        else
            w.etat->setCurrentText(etatAff);
    }

    const QString tdAff = cell(8).isEmpty() ? ptd : cell(8);
    if (w.typeDesign && !tdAff.isEmpty())
        w.typeDesign->setCurrentText(tdAff);

    QDate dateFab;
    if (!cell(7).isEmpty()) {
        dateFab = QDate::fromString(cell(7), Qt::ISODate);
        if (!dateFab.isValid())
            dateFab = QDate::fromString(cell(7), QStringLiteral("dd/MM/yyyy"));
    }
    if (!dateFab.isValid())
        dateFab = pdf;
    if (w.dateFabrication) {
        if (dateFab.isValid())
            w.dateFabrication->setDate(dateFab);
        else
            w.dateFabrication->setDate(QDate::currentDate());
    }

    if (w.style)
        w.style->setText(ps.isEmpty() ? packedStyle : ps);

    return selectedId;
}

bool Produit::populateProductTable(QTableWidget *table, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Tableau produits indisponible.");
        return false;
    }
    if (!connexionBdOuverte()) {
        if (errorMessage)
            *errorMessage = kMsgConnexionFermee;
        return false;
    }

    std::unique_ptr<QSqlQueryModel> model(Produit::afficher());
    if (!model) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Modele SQL indisponible.");
        return false;
    }
    if (model->lastError().isValid()) {
        if (errorMessage)
            *errorMessage = model->lastError().text();
        return false;
    }

    table->setSortingEnabled(false);
    table->setRowCount(0);

    const int n = model->rowCount();
    for (int r = 0; r < n; ++r) {
        table->insertRow(r);
        for (int c = 0; c < 11; ++c) {
            const QVariant v = model->data(model->index(r, c));
            QString txt;
            if (c == 7) {
                const QDate d = v.toDate();
                if (d.isValid()) {
                    txt = d.toString(Qt::ISODate);
                } else {
                    const QDateTime dt = v.toDateTime();
                    if (dt.isValid())
                        txt = dt.date().toString(Qt::ISODate);
                }
            } else {
                txt = v.toString();
            }
            if (c == 9) {
                auto *it = new QTableWidgetItem(packedStyleToTableDisplay(txt));
                if (!txt.isEmpty()) {
                    it->setData(Qt::UserRole, txt);
                    it->setToolTip(txt);
                }
                table->setItem(r, c, it);
            } else if (c == 0) {
                const int pid = parseProductIdText(v.toString());
                table->setItem(r, c, new QTableWidgetItem(pid > 0 ? QString::number(pid) : txt.trimmed()));
            } else {
                table->setItem(r, c, new QTableWidgetItem(txt));
            }
        }
    }
    table->setSortingEnabled(true);
    return true;
}

void Produit::filterProductTable(QTableWidget *table, const QString &needle, int mode)
{
    if (!table)
        return;

    const QString q = needle.trimmed();
    if (q.isEmpty()) {
        for (int r = 0; r < table->rowCount(); ++r)
            table->setRowHidden(r, false);
        return;
    }

    int col = 1;
    if (mode == 1)
        col = 3;
    else if (mode == 2)
        col = 6;

    const QString needleUp = q.toUpper();
    for (int r = 0; r < table->rowCount(); ++r) {
        QString cell;
        if (col == 9) {
            QTableWidgetItem *it = table->item(r, col);
            if (it) {
                const QString raw = it->data(Qt::UserRole).toString();
                cell = raw.isEmpty() ? it->text() : raw;
            }
        } else {
            QTableWidgetItem *it = table->item(r, col);
            cell = it ? it->text() : QString();
        }
        table->setRowHidden(r, !cell.toUpper().contains(needleUp));
    }
}

int Produit::searchFilterModeFromComboText(const QString &comboCurrentText)
{
    const QString mode = comboCurrentText;
    if (mode.contains(QStringLiteral("cuir"), Qt::CaseInsensitive))
        return 1;
    if (mode.contains(QStringLiteral("tat"), Qt::CaseInsensitive)
        || mode.contains(QStringLiteral("état"), Qt::CaseInsensitive))
        return 2;
    return 0;
}

QString Produit::defectAlertsPlainText(QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    if (!connexionBdOuverte()) {
        return QStringLiteral("Connexion base fermee : impossible de charger les alertes.");
    }

    const QString nomPhys = CommerceStore::produitsLibelleColumnPhysical();
    const QString nomCol = QStringLiteral("P.%1").arg(nomPhys);
    const bool hasActif = CommerceStore::produitsColumnExists(QStringLiteral("ACTIF"));
    const bool hasStyle = CommerceStore::produitsColumnExists(QStringLiteral("STYLE"));

    QStringList whereParts;
    whereParts << QStringLiteral("NVL(S.QTE_DISPONIBLE, 0) <= NVL(S.SEUIL_ALERTE, 5)");
    if (hasActif)
        whereParts << QStringLiteral("NVL(P.ACTIF, 1) = 0");
    if (hasStyle) {
        whereParts << QStringLiteral("INSTR(NVL(P.STYLE, ' '), 'fectueux') > 0");
        whereParts << QStringLiteral("INSTR(UPPER(NVL(P.STYLE, ' ')), 'DEFECT') > 0");
    }

    const QString whereClause = whereParts.join(QStringLiteral(" OR "));

    QString actifSelect = QStringLiteral(", 1 AS ACTIF");
    if (hasActif)
        actifSelect = QStringLiteral(", NVL(P.ACTIF, 1) AS ACTIF");

    QString styleSelect = QStringLiteral(", CAST(' ' AS VARCHAR2(400)) AS STYLE");
    if (hasStyle)
        styleSelect = QStringLiteral(", NVL(P.STYLE, ' ') AS STYLE");

    const QString sql = QStringLiteral(
                           "SELECT %1 AS NOM, P.ID, NVL(S.QTE_DISPONIBLE, 0) AS QTE, NVL(S.SEUIL_ALERTE, 5) AS SEUIL%2%3 "
                           "FROM PRODUITS P "
                           "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
                           "WHERE (%4) "
                           "ORDER BY P.ID")
                           .arg(nomCol, actifSelect, styleSelect, whereClause);

    QSqlQuery query;
    if (!query.exec(sql)) {
        if (errorMessage)
            *errorMessage = query.lastError().text();
        return QString();
    }

    QStringList lines;
    lines << QStringLiteral("--- Alertes production / stock (%1) ---")
                 .arg(QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm")));
    int n = 0;
    while (query.next()) {
        ++n;
        const QString nom = query.value(0).toString();
        const int id = query.value(1).toInt();
        const int qte = query.value(2).toInt();
        const int seuil = query.value(3).toInt();
        const int actif = query.value(4).toInt();
        const QString style = query.value(5).toString();

        QStringList reasons;
        if (qte <= seuil)
            reasons << QStringLiteral("stock faible (%1 / seuil %2)").arg(QString::number(qte), QString::number(seuil));
        if (hasActif && actif == 0)
            reasons << QStringLiteral("produit inactif (ACTIF = 0)");
        if (hasStyle
            && (style.contains(QStringLiteral("fectueux"), Qt::CaseInsensitive)
                || style.contains(QStringLiteral("defect"), Qt::CaseInsensitive)))
            reasons << QStringLiteral("mention defaut (STYLE)");

        if (reasons.isEmpty())
            reasons << QStringLiteral("condition d'alerte en base");

        lines << QStringLiteral("• ID %1 — %2 : %3")
                     .arg(QString::number(id), nom, reasons.join(QStringLiteral(" ; ")));
    }

    if (n == 0) {
        lines << QString();
        lines << QStringLiteral("Aucune alerte : stocks au-dessus des seuils, produits actifs, pas de trace "
                                "\"Défectueux\" dans STYLE.");
    }

    return lines.join(QStringLiteral("\n"));
}

QString Produit::chatbotContextFromProductTable(QTableWidget *table, int maxRows)
{
    if (!table)
        return QStringLiteral("(Tableau produits indisponible.)");

    QStringList lines;
    lines << QStringLiteral("Contexte catalogue (lignes visibles du tableau produits) :");
    lines << QStringLiteral("Colonnes : ID | nom | categorie | type cuir | qualite | stock | etat");
    int n = 0;
    for (int r = 0; r < table->rowCount() && n < maxRows; ++r) {
        if (table->isRowHidden(r))
            continue;
        QStringList cols;
        for (int c = 0; c < 7; ++c) {
            QTableWidgetItem *it = table->item(r, c);
            cols << (it ? it->text() : QString());
        }
        lines << cols.join(QStringLiteral(" | "));
        ++n;
    }
    if (n == 0) {
        lines << QStringLiteral(
            "(Aucune ligne affichee : videz le filtre de recherche ou rouvrez la page Produits.)");
    }
    return lines.join(QLatin1Char('\n'));
}
