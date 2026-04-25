#include "matierepremiere.h"

#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFile>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTextStream>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

namespace {
bool execSql(const QString &sql, QString *errorMessage)
{
    QSqlQuery q;
    if (q.exec(sql))
        return true;
    if (errorMessage) {
        *errorMessage = q.lastError().text().trimmed();
        if (errorMessage->isEmpty())
            *errorMessage = QStringLiteral("Erreur SQL.");
    }
    return false;
}

bool dbOpen()
{
    return QSqlDatabase::database().isOpen();
}

static QString itemText(QTableWidget *t, int row, int col)
{
    if (!t || row < 0 || col < 0 || col >= t->columnCount())
        return {};
    if (QTableWidgetItem *it = t->item(row, col))
        return it->text();
    return {};
}
} // namespace

QString MatierePremiere::lastSqlError;

MatierePremiere::MatierePremiere() = default;

MatierePremiere::MatierePremiere(int id,
                                 const QString &ref,
                                 const QString &nomCuir,
                                 const QString &typeCuir,
                                 const QString &gamme,
                                 const QString &couleur,
                                 const QString &statut,
                                 double epaisseur,
                                 const QString &origine,
                                 int reserve)
    : m_id(id)
    , m_reference(ref)
    , m_nomCuir(nomCuir)
    , m_typeCuir(typeCuir)
    , m_gamme(gamme)
    , m_couleur(couleur)
    , m_statut(statut)
    , m_epaisseur(epaisseur)
    , m_origine(origine)
    , m_reserve(reserve)
{
}

bool MatierePremiere::ensureSchema(QString *errorMessage)
{
    if (!dbOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Base non connectee.");
        return false;
    }
    const QString create =
        QStringLiteral("BEGIN EXECUTE IMMEDIATE 'CREATE TABLE MATIERES_PREMIERES ("
                       "ID NUMBER PRIMARY KEY, "
                       "REFERENCE_INTERNE VARCHAR2(80) NOT NULL, "
                       "NOM_CUIR VARCHAR2(200) NOT NULL, "
                       "TYPE_CUIR VARCHAR2(120), "
                       "GAMME VARCHAR2(80), "
                       "COULEUR VARCHAR2(80), "
                       "STATUT VARCHAR2(40), "
                       "EPAISSEUR NUMBER(10,2), "
                       "ORIGINE VARCHAR2(120), "
                       "RESERVE NUMBER(12) DEFAULT 0"
                       ")'; EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;");
    return execSql(create, errorMessage);
}

void MatierePremiere::seedDemoIfEmpty(QString *errorMessage)
{
    if (!dbOpen())
        return;
    QSqlQuery c;
    if (!c.exec(QStringLiteral("SELECT COUNT(*) FROM MATIERES_PREMIERES")) || !c.next())
        return;
    if (c.value(0).toInt() > 0)
        return;
    const int id1 = nextAvailableId();
    if (id1 <= 0)
        return;
    MatierePremiere a(id1,
                      QStringLiteral("MP-DEMO-01"),
                      QStringLiteral("Cuir vachette pleine fleur"),
                      QStringLiteral("Vachette"),
                      QStringLiteral("Supreme"),
                      QStringLiteral("Naturel"),
                      QStringLiteral("Disponible"),
                      1.4,
                      QStringLiteral("Italie"),
                      42);
    if (!a.ajouter()) {
        if (errorMessage)
            *errorMessage = lastSqlError;
        return;
    }
    const int id2 = nextAvailableId();
    if (id2 <= id1)
        return;
    MatierePremiere b(id2,
                      QStringLiteral("MP-DEMO-02"),
                      QStringLiteral("Cuir croûte pigmentée"),
                      QStringLiteral("Croute"),
                      QStringLiteral("Economy"),
                      QStringLiteral("Noir"),
                      QStringLiteral("Disponible"),
                      1.1,
                      QStringLiteral("Maroc"),
                      8);
    b.ajouter();
}

int MatierePremiere::nextAvailableId()
{
    if (!dbOpen()) {
        lastSqlError = QStringLiteral("Connexion fermee.");
        return 0;
    }
    QSqlQuery q;
    if (!q.exec(QStringLiteral("SELECT NVL(MAX(ID), 0) + 1 FROM MATIERES_PREMIERES")))
        return 1;
    return q.next() ? q.value(0).toInt() : 1;
}

bool MatierePremiere::idExiste(int id)
{
    if (!dbOpen() || id <= 0)
        return false;
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM MATIERES_PREMIERES WHERE ID=:id"));
    q.bindValue(QStringLiteral(":id"), id);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

bool MatierePremiere::populateTable(QTableWidget *table, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Tableau invalide.");
        return false;
    }
    if (!dbOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Base non connectee.");
        return false;
    }
    QSqlQuery q;
    if (!q.exec(QStringLiteral(
            "SELECT ID, REFERENCE_INTERNE, NOM_CUIR, TYPE_CUIR, GAMME, COULEUR, EPAISSEUR, ORIGINE, RESERVE, STATUT "
            "FROM MATIERES_PREMIERES ORDER BY ID"))) {
        lastSqlError = q.lastError().text();
        if (errorMessage)
            *errorMessage = lastSqlError;
        return false;
    }
    table->setRowCount(0);
    while (q.next()) {
        const int row = table->rowCount();
        table->insertRow(row);
        const int id = q.value(0).toInt();
        const QString ref = q.value(1).toString();
        const QString nom = q.value(2).toString();
        const QString type = q.value(3).toString();
        const QString gamme = q.value(4).toString();
        const QString couleur = q.value(5).toString();
        const double ep = q.value(6).toDouble();
        const QString orig = q.value(7).toString();
        const int res = q.value(8).toInt();
        const QString stat = q.value(9).toString();

        const bool critique = res <= 10;
        const auto mk = [critique](const QString &txt) -> QTableWidgetItem * {
            auto *it = new QTableWidgetItem(txt);
            it->setTextAlignment(Qt::AlignCenter);
            if (critique)
                it->setForeground(QColor(200, 50, 50));
            return it;
        };
        table->setItem(row, 0, mk(QString::number(id)));
        table->setItem(row, 1, mk(ref));
        table->setItem(row, 2, mk(nom));
        table->setItem(row, 3, mk(type));
        table->setItem(row, 4, mk(gamme));
        table->setItem(row, 5, mk(couleur));
        table->setItem(row, 6, mk(QString::number(ep, 'f', 2)));
        table->setItem(row, 7, mk(orig));
        table->setItem(row, 8, mk(QStringLiteral("-")));
        table->setItem(row, 9, mk(QString::number(res)));
        table->setItem(row, 10, mk(QStringLiteral("-")));
        table->setItem(row, 11, mk(QStringLiteral("-")));
        table->setItem(row, 12, mk(stat.isEmpty() ? QStringLiteral("Disponible") : stat));
    }
    return true;
}

void MatierePremiere::filterTable(QTableWidget *table, const QString &needle)
{
    if (!table)
        return;
    const QString n = needle.trimmed().toUpper();
    for (int r = 0; r < table->rowCount(); ++r) {
        bool show = n.isEmpty();
        if (!show) {
            for (int c = 0; c < table->columnCount(); ++c) {
                if (table->item(r, c) && table->item(r, c)->text().toUpper().contains(n)) {
                    show = true;
                    break;
                }
            }
        }
        table->setRowHidden(r, !show);
    }
}

void MatierePremiere::clearEditorFields(const MatierePremiereEditorWidgets &w)
{
    if (w.idMp)
        w.idMp->clear();
    if (w.reference)
        w.reference->clear();
    if (w.nomCuir)
        w.nomCuir->clear();
    if (w.typeCuir)
        w.typeCuir->clear();
    if (w.gamme) {
        w.gamme->setCurrentIndex(0);
        if (w.gamme->isEditable())
            w.gamme->setEditText(QString());
    }
    if (w.couleur)
        w.couleur->clear();
    if (w.statut)
        w.statut->clear();
    if (w.epaisseur)
        w.epaisseur->clear();
    if (w.origine)
        w.origine->clear();
    if (w.reserve)
        w.reserve->clear();
    if (w.fournisseurAffiche)
        w.fournisseurAffiche->clear();
    if (w.prixAffiche)
        w.prixAffiche->clear();
    if (w.dateAchatAffiche)
        w.dateAchatAffiche->setDate(QDate::currentDate());
}

int MatierePremiere::fillEditorFromTableRow(const MatierePremiereEditorWidgets &w, QTableWidget *table, int row)
{
    if (!table || row < 0 || row >= table->rowCount())
        return -1;
    if (w.idMp)
        w.idMp->setText(itemText(table, row, 0));
    if (w.reference)
        w.reference->setText(itemText(table, row, 1));
    if (w.nomCuir)
        w.nomCuir->setText(itemText(table, row, 2));
    if (w.typeCuir)
        w.typeCuir->setText(itemText(table, row, 3));
    if (w.gamme) {
        const QString g = itemText(table, row, 4);
        int idx = w.gamme->findText(g, Qt::MatchFixedString);
        if (idx < 0 && w.gamme->isEditable())
            w.gamme->setEditText(g);
        else
            w.gamme->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    if (w.couleur)
        w.couleur->setText(itemText(table, row, 5));
    if (w.epaisseur)
        w.epaisseur->setText(itemText(table, row, 6));
    if (w.origine)
        w.origine->setText(itemText(table, row, 7));
    if (w.fournisseurAffiche)
        w.fournisseurAffiche->setText(itemText(table, row, 8));
    if (w.reserve)
        w.reserve->setText(itemText(table, row, 9));
    if (w.prixAffiche)
        w.prixAffiche->setText(itemText(table, row, 10));
    if (w.statut)
        w.statut->setText(itemText(table, row, 12));
    return itemText(table, row, 0).toInt();
}

QString MatierePremiere::statsSummaryPlain(QString *errorMessage)
{
    if (!dbOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Base non connectee.");
        return {};
    }
    QString out;
    QSqlQuery q;
    if (q.exec(QStringLiteral("SELECT NVL(SUM(RESERVE),0) FROM MATIERES_PREMIERES")) && q.next())
        out += QStringLiteral("Total stock (unites) : %1\n").arg(q.value(0).toLongLong());
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM MATIERES_PREMIERES")) && q.next())
        out += QStringLiteral("References : %1\n").arg(q.value(0).toInt());
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM MATIERES_PREMIERES WHERE RESERVE<=10")) && q.next())
        out += QStringLiteral("Lignes sous seuil (<=10) : %1\n").arg(q.value(0).toInt());
    return out.trimmed();
}

bool MatierePremiere::ajouter() const
{
    if (!dbOpen()) {
        lastSqlError = QStringLiteral("Connexion fermee.");
        return false;
    }
    if (m_nomCuir.trimmed().isEmpty() || m_reference.trimmed().isEmpty()) {
        lastSqlError = QStringLiteral("Reference et nom du cuir requis.");
        return false;
    }
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "INSERT INTO MATIERES_PREMIERES "
        "(ID, REFERENCE_INTERNE, NOM_CUIR, TYPE_CUIR, GAMME, COULEUR, STATUT, EPAISSEUR, ORIGINE, RESERVE) "
        "VALUES (:id, :ref, :nom, :typ, :gam, :col, :st, :ep, :or, :re)"));
    q.bindValue(QStringLiteral(":id"), m_id);
    q.bindValue(QStringLiteral(":ref"), m_reference.trimmed());
    q.bindValue(QStringLiteral(":nom"), m_nomCuir.trimmed());
    q.bindValue(QStringLiteral(":typ"), m_typeCuir.trimmed());
    q.bindValue(QStringLiteral(":gam"), m_gamme.trimmed());
    q.bindValue(QStringLiteral(":col"), m_couleur.trimmed());
    q.bindValue(QStringLiteral(":st"), m_statut.trimmed().isEmpty() ? QStringLiteral("Disponible") : m_statut.trimmed());
    q.bindValue(QStringLiteral(":ep"), m_epaisseur);
    q.bindValue(QStringLiteral(":or"), m_origine.trimmed());
    q.bindValue(QStringLiteral(":re"), m_reserve);
    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }
    return true;
}

bool MatierePremiere::modifier(int oldId, int newId) const
{
    if (!dbOpen()) {
        lastSqlError = QStringLiteral("Connexion fermee.");
        return false;
    }
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "UPDATE MATIERES_PREMIERES SET ID=:nid, REFERENCE_INTERNE=:ref, NOM_CUIR=:nom, TYPE_CUIR=:typ, "
        "GAMME=:gam, COULEUR=:col, STATUT=:st, EPAISSEUR=:ep, ORIGINE=:or, RESERVE=:re WHERE ID=:oid"));
    q.bindValue(QStringLiteral(":nid"), newId);
    q.bindValue(QStringLiteral(":ref"), m_reference.trimmed());
    q.bindValue(QStringLiteral(":nom"), m_nomCuir.trimmed());
    q.bindValue(QStringLiteral(":typ"), m_typeCuir.trimmed());
    q.bindValue(QStringLiteral(":gam"), m_gamme.trimmed());
    q.bindValue(QStringLiteral(":col"), m_couleur.trimmed());
    q.bindValue(QStringLiteral(":st"), m_statut.trimmed().isEmpty() ? QStringLiteral("Disponible") : m_statut.trimmed());
    q.bindValue(QStringLiteral(":ep"), m_epaisseur);
    q.bindValue(QStringLiteral(":or"), m_origine.trimmed());
    q.bindValue(QStringLiteral(":re"), m_reserve);
    q.bindValue(QStringLiteral(":oid"), oldId);
    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }
    return true;
}

bool MatierePremiere::supprimer(int id)
{
    if (!dbOpen()) {
        lastSqlError = QStringLiteral("Connexion fermee.");
        return false;
    }
    QSqlQuery q;
    q.prepare(QStringLiteral("DELETE FROM MATIERES_PREMIERES WHERE ID=:id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!q.exec()) {
        lastSqlError = q.lastError().text();
        return false;
    }
    return true;
}

bool MatierePremiere::exportPdfFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Tableau invalide.");
        return false;
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);

    QString html;
    html += QStringLiteral("<html><body style='font-family:Segoe UI,Arial;'>");
    html += QStringLiteral("<h2 style='color:#582900;'>Royal Leather House — Matieres premieres</h2>");
    html += QStringLiteral("<table border='1' cellspacing='0' cellpadding='4' style='border-collapse:collapse;font-size:9px;'>");
    html += QStringLiteral("<tr style='background:#582900;color:#f8f1e3;'>");
    for (int c = 0; c < table->columnCount(); ++c) {
        html += QStringLiteral("<th>%1</th>").arg(table->horizontalHeaderItem(c)
                                                       ? table->horizontalHeaderItem(c)->text().toHtmlEscaped()
                                                       : QString());
    }
    html += QStringLiteral("</tr>");
    for (int r = 0; r < table->rowCount(); ++r) {
        if (table->isRowHidden(r))
            continue;
        html += QStringLiteral("<tr>");
        for (int c = 0; c < table->columnCount(); ++c) {
            const QString cell = table->item(r, c) ? table->item(r, c)->text() : QString();
            html += QStringLiteral("<td>%1</td>").arg(cell.toHtmlEscaped());
        }
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table></body></html>");
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    return true;
}

bool MatierePremiere::exportCsvFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Tableau invalide.");
        return false;
    }
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Ecriture impossible.");
        return false;
    }
    QTextStream out(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
    out << "\xEF\xBB\xBF";
    for (int c = 0; c < table->columnCount(); ++c) {
        if (c)
            out << QLatin1Char(';');
        out << (table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text() : QString());
    }
    out << QLatin1Char('\n');
    for (int r = 0; r < table->rowCount(); ++r) {
        if (table->isRowHidden(r))
            continue;
        for (int c = 0; c < table->columnCount(); ++c) {
            if (c)
                out << QLatin1Char(';');
            out << (table->item(r, c) ? table->item(r, c)->text() : QString());
        }
        out << QLatin1Char('\n');
    }
    return true;
}
