#include "employe.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QTime>
#include <QFile>
#include <QIODevice>
#include <QVariant>
#include <QHeaderView>
#include <QLineEdit>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
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
    QSqlQuery q(QSqlDatabase::database());
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

class DateTableItem : public QTableWidgetItem
{
public:
    explicit DateTableItem(const QString &display, const QDate &date)
        : QTableWidgetItem(display)
    {
        setData(Qt::UserRole, date);
    }

    bool operator<(const QTableWidgetItem &other) const override
    {
        const QDate a = data(Qt::UserRole).toDate();
        const QDate b = other.data(Qt::UserRole).toDate();
        if (a.isValid() && b.isValid())
            return a < b;
        return QTableWidgetItem::operator<(other);
    }
};

class NumericTableItem : public QTableWidgetItem
{
public:
    explicit NumericTableItem(const QString &display)
        : QTableWidgetItem(display)
    {
        QString normalized = display;
        normalized.replace(QLatin1Char(','), QLatin1Char('.'));
        bool ok = false;
        const double value = normalized.toDouble(&ok);
        setData(Qt::UserRole, ok ? value : 0.0);
    }

    bool operator<(const QTableWidgetItem &other) const override
    {
        const double a = data(Qt::UserRole).toDouble();
        const double b = other.data(Qt::UserRole).toDouble();
        return a < b;
    }
};

static int colFromSearchKey(const QString &key)
{
    const QString k = key.toLower();
    if (k == QLatin1String("cin"))
        return 0;
    if (k == QLatin1String("nom"))
        return 1;
    if (k == QLatin1String("prenom"))
        return 2;
    if (k == QLatin1String("sexe"))
        return 3;
    if (k == QLatin1String("salaire"))
        return 4;
    if (k == QLatin1String("date"))
        return 5;
    if (k == QLatin1String("tel") || k == QLatin1String("telephone"))
        return 6;
    if (k == QLatin1String("poste"))
        return 7;
    if (k == QLatin1String("adresse"))
        return 8;
    if (k == QLatin1String("email"))
        return 9;
    return -1;
}

} // namespace

Employe::Employe() = default;

Employe::Employe(const QString &cin,
                   const QString &nom,
                   const QString &prenom,
                   const QString &sexe,
                   double salaire,
                   const QDate &dateEmbauche,
                   const QString &telephone,
                   const QString &poste,
                   const QString &adresse,
                   const QString &email)
    : m_cin(cin)
    , m_nom(nom)
    , m_prenom(prenom)
    , m_sexe(sexe)
    , m_salaire(salaire)
    , m_dateEmbauche(dateEmbauche)
    , m_telephone(telephone)
    , m_poste(poste)
    , m_adresse(adresse)
    , m_email(email)
{
}

bool Employe::ensureSchema(QString *errorMessage)
{
    if (!dbOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Base non connectee.");
        return false;
    }
    const QString create = QStringLiteral(
        "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE EMPLOYES ("
        "CIN VARCHAR2(8) PRIMARY KEY, "
        "NOM VARCHAR2(100) NOT NULL, "
        "PRENOM VARCHAR2(100) NOT NULL, "
        "SEXE VARCHAR2(20) NOT NULL, "
        "SALAIRE NUMBER(12,2) NOT NULL, "
        "DATE_EMBAUCHE DATE NOT NULL, "
        "TELEPHONE VARCHAR2(8) NOT NULL, "
        "POSTE VARCHAR2(100) NOT NULL, "
        "ADRESSE VARCHAR2(200) NOT NULL, "
        "EMAIL VARCHAR2(150) NOT NULL)'; "
        "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;");
    return execSql(create, errorMessage);
}

void Employe::seedDemoIfEmpty(QString *errorMessage)
{
    if (!dbOpen())
        return;
    // Oracle ODBC: ne pas lancer un INSERT tant que le curseur du COUNT est actif (S1010).
    int count = 0;
    {
        QSqlQuery c(QSqlDatabase::database());
        c.setForwardOnly(true);
        if (!c.exec(QStringLiteral("SELECT COUNT(*) FROM EMPLOYES")) || !c.next())
            return;
        count = c.value(0).toInt();
    }

    if (count > 0)
        return;

    Employe demo(QStringLiteral("12345678"),
                   QStringLiteral("Ben"),
                   QStringLiteral("Salah"),
                   QStringLiteral("Homme"),
                   1850.0,
                   QDate::currentDate().addMonths(-2),
                   QStringLiteral("22123456"),
                   QStringLiteral("Magasinier"),
                   QStringLiteral("Tunis"),
                   QStringLiteral("demo.employe@example.com"));
    demo.ajouter(errorMessage);
}

bool Employe::populateTable(QTableWidget *table, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Table invalide.");
        return false;
    }
    if (!dbOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Base non connectee.");
        return false;
    }

    const bool prevSort = table->isSortingEnabled();
    table->setSortingEnabled(false);
    table->setRowCount(0);

    QSqlQuery query(QSqlDatabase::database());
    query.setForwardOnly(true);
    if (!query.exec(QStringLiteral(
            "SELECT CIN,NOM,PRENOM,SEXE,SALAIRE,TO_CHAR(DATE_EMBAUCHE,'DD/MM/YYYY'),TELEPHONE,POSTE,ADRESSE,EMAIL "
            "FROM EMPLOYES ORDER BY CIN"))) {
        if (errorMessage)
            *errorMessage = query.lastError().text().trimmed();
        table->setSortingEnabled(prevSort);
        return false;
    }

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 10; ++col) {
            const QString val = query.value(col).toString();
            if (col == 5) {
                const QDate d = QDate::fromString(val, QStringLiteral("dd/MM/yyyy"));
                table->setItem(row, col, new DateTableItem(val, d));
            } else if (col == 0 || col == 4 || col == 6) {
                table->setItem(row, col, new NumericTableItem(val));
            } else {
                table->setItem(row, col, new QTableWidgetItem(val));
            }
        }
        ++row;
    }
    table->setSortingEnabled(prevSort);
    return true;
}

void Employe::applySearchFilter(QTableWidget *table, const QString &text)
{
    if (!table)
        return;
    const QString t = text.trimmed();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const QStringList parts = t.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#else
    const QStringList parts = t.split(QLatin1Char(' '), QString::SkipEmptyParts);
#endif

    QList<QPair<int, QString>> fieldFilters;
    QStringList globalFilters;

    for (const QString &p : parts) {
        const int sep = p.indexOf(QLatin1Char(':'));
        if (sep > 0 && sep < p.size() - 1) {
            const QString key = p.left(sep).trimmed();
            const QString value = p.mid(sep + 1).trimmed();
            const int col = colFromSearchKey(key);
            if (col >= 0 && !value.isEmpty()) {
                fieldFilters.append({col, value});
                continue;
            }
        }
        globalFilters.append(p);
    }

    for (int r = 0; r < table->rowCount(); ++r) {
        bool match = t.isEmpty();
        if (!match && fieldFilters.isEmpty() && globalFilters.isEmpty())
            match = true;

        if (!match && !fieldFilters.isEmpty()) {
            bool okAll = true;
            for (const auto &f : fieldFilters) {
                const auto *it = table->item(r, f.first);
                const QString cell = it ? it->text() : QString();
                if (!cell.contains(f.second, Qt::CaseInsensitive)) {
                    okAll = false;
                    break;
                }
            }
            match = okAll;
        }

        if (match && !globalFilters.isEmpty()) {
            for (const QString &word : globalFilters) {
                bool foundWord = false;
                for (int c = 0; c < table->columnCount(); ++c) {
                    const auto *it = table->item(r, c);
                    if (it && it->text().contains(word, Qt::CaseInsensitive)) {
                        foundWord = true;
                        break;
                    }
                }
                if (!foundWord) {
                    match = false;
                    break;
                }
            }
        } else if (!match && fieldFilters.isEmpty() && !globalFilters.isEmpty()) {
            for (int c = 0; c < table->columnCount(); ++c) {
                const auto *it = table->item(r, c);
                if (it && it->text().contains(t, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        table->setRowHidden(r, !match);
    }
}

void Employe::clearEditor(const EmployeEditorWidgets &w)
{
    if (w.cin)
        w.cin->clear();
    if (w.nom)
        w.nom->clear();
    if (w.prenom)
        w.prenom->clear();
    if (w.sexe)
        w.sexe->setCurrentIndex(0);
    if (w.salaire)
        w.salaire->clear();
    if (w.dateEmbauche)
        w.dateEmbauche->setDate(QDate::currentDate());
    if (w.telephone)
        w.telephone->clear();
    if (w.poste)
        w.poste->clear();
    if (w.adresse)
        w.adresse->clear();
    if (w.email)
        w.email->clear();
}

int Employe::fillEditorFromTableRow(const EmployeEditorWidgets &w, QTableWidget *table, int row)
{
    if (!table || row < 0 || row >= table->rowCount())
        return -1;
    if (!table->item(row, 0))
        return -1;

    if (w.cin)
        w.cin->setText(table->item(row, 0)->text());
    if (w.nom)
        w.nom->setText(table->item(row, 1)->text());
    if (w.prenom)
        w.prenom->setText(table->item(row, 2)->text());
    if (w.sexe)
        w.sexe->setCurrentText(table->item(row, 3)->text());
    if (w.salaire)
        w.salaire->setText(table->item(row, 4)->text());
    if (w.dateEmbauche) {
        const QString ds = table->item(row, 5) ? table->item(row, 5)->text() : QString();
        const QDate d = QDate::fromString(ds, QStringLiteral("dd/MM/yyyy"));
        w.dateEmbauche->setDate(d.isValid() ? d : QDate::currentDate());
    }
    if (w.telephone)
        w.telephone->setText(table->item(row, 6)->text());
    if (w.poste)
        w.poste->setText(table->item(row, 7)->text());
    if (w.adresse)
        w.adresse->setText(table->item(row, 8)->text());
    if (w.email)
        w.email->setText(table->item(row, 9)->text());
    return 0;
}

void Employe::installInputValidators(const EmployeEditorWidgets &w)
{
    if (w.cin) {
        w.cin->setValidator(
            new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{0,8}$")), w.cin));
    }
    if (w.telephone) {
        w.telephone->setValidator(
            new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{0,8}$")), w.telephone));
    }
}

QString Employe::validateForm(const EmployeEditorWidgets &w)
{
    const QString cin = w.cin ? w.cin->text().trimmed() : QString();
    const QString nom = w.nom ? w.nom->text().trimmed() : QString();
    const QString prenom = w.prenom ? w.prenom->text().trimmed() : QString();
    QString salaireTxt = w.salaire ? w.salaire->text().trimmed() : QString();
    const QString tel = w.telephone ? w.telephone->text().trimmed() : QString();
    const QString poste = w.poste ? w.poste->text().trimmed() : QString();
    const QString adresse = w.adresse ? w.adresse->text().trimmed() : QString();
    const QString email = w.email ? w.email->text().trimmed() : QString();

    if (cin.isEmpty() || nom.isEmpty() || prenom.isEmpty() || salaireTxt.isEmpty() || tel.isEmpty() || poste.isEmpty()
        || adresse.isEmpty() || email.isEmpty()) {
        return QStringLiteral("Veuillez remplir tous les champs.");
    }

    if (!QRegularExpression(QStringLiteral("^\\d{8}$")).match(cin).hasMatch())
        return QStringLiteral("Le CIN doit contenir exactement 8 chiffres.");

    if (!QRegularExpression(QStringLiteral("^\\d{8}$")).match(tel).hasMatch())
        return QStringLiteral("Le numero de telephone doit contenir exactement 8 chiffres.");

    const QRegularExpression rxEmail(QStringLiteral("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$"));
    if (!rxEmail.match(email).hasMatch())
        return QStringLiteral("Veuillez saisir une adresse email valide (exemple@domaine.com).");

    const QString emailLower = email.toLower();
    if (emailLower.contains(QStringLiteral("@gamil.com")) || emailLower.contains(QStringLiteral("@gmial.com"))
        || emailLower.contains(QStringLiteral("@gmai.com")) || emailLower.contains(QStringLiteral("@gmali.com"))) {
        return QStringLiteral("Le domaine semble incorrect (ex: gmail.com).");
    }

    const QRegularExpression rxNom(QStringLiteral("^[\\p{L}\\s'-]+$"));
    if (!rxNom.match(nom).hasMatch() || !rxNom.match(prenom).hasMatch())
        return QStringLiteral("Nom et prenom : lettres, espaces ou tirets uniquement.");

    const QDate dateEmb = w.dateEmbauche ? w.dateEmbauche->date() : QDate::currentDate();
    if (dateEmb > QDate::currentDate())
        return QStringLiteral("La date d'embauche ne peut pas etre posterieure a aujourd'hui.");

    bool okSalaire = false;
    salaireTxt.replace(QLatin1Char(','), QLatin1Char('.'));
    const double salaire = salaireTxt.toDouble(&okSalaire);
    if (!okSalaire || salaire <= 0.0)
        return QStringLiteral("Le salaire doit etre un nombre positif.");

    return {};
}

bool Employe::cinExists(const QString &cin, QString *errorMessage)
{
    QSqlQuery q(QSqlDatabase::database());
    q.setForwardOnly(true);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM EMPLOYES WHERE CIN=:c"));
    q.bindValue(QStringLiteral(":c"), cin);
    if (!q.exec() || !q.next()) {
        if (errorMessage)
            *errorMessage = q.lastError().text().trimmed();
        return false;
    }
    return q.value(0).toInt() > 0;
}

bool Employe::ajouter(QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare(
        QStringLiteral("INSERT INTO EMPLOYES (CIN,NOM,PRENOM,SEXE,SALAIRE,DATE_EMBAUCHE,TELEPHONE,POSTE,ADRESSE,EMAIL) "
                       "VALUES (:cin,:nom,:prenom,:sexe,:salaire,:demb,:tel,:poste,:adresse,:email)"));
    query.bindValue(QStringLiteral(":cin"), m_cin);
    query.bindValue(QStringLiteral(":nom"), m_nom);
    query.bindValue(QStringLiteral(":prenom"), m_prenom);
    query.bindValue(QStringLiteral(":sexe"), m_sexe);
    query.bindValue(QStringLiteral(":salaire"), m_salaire);
    query.bindValue(QStringLiteral(":demb"), QVariant::fromValue(QDateTime(m_dateEmbauche, QTime(0, 0))));
    query.bindValue(QStringLiteral(":tel"), m_telephone);
    query.bindValue(QStringLiteral(":poste"), m_poste);
    query.bindValue(QStringLiteral(":adresse"), m_adresse);
    query.bindValue(QStringLiteral(":email"), m_email);

    if (query.exec())
        return true;
    if (errorMessage)
        *errorMessage = query.lastError().text().trimmed();
    return false;
}

bool Employe::modifier(QString *errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare(QStringLiteral(
        "UPDATE EMPLOYES SET NOM=:nom,PRENOM=:prenom,SEXE=:sexe,SALAIRE=:salaire,DATE_EMBAUCHE=:demb,"
        "TELEPHONE=:tel,POSTE=:poste,ADRESSE=:adresse,EMAIL=:email WHERE CIN=:cin"));
    query.bindValue(QStringLiteral(":cin"), m_cin);
    query.bindValue(QStringLiteral(":nom"), m_nom);
    query.bindValue(QStringLiteral(":prenom"), m_prenom);
    query.bindValue(QStringLiteral(":sexe"), m_sexe);
    query.bindValue(QStringLiteral(":salaire"), m_salaire);
    query.bindValue(QStringLiteral(":demb"), QVariant::fromValue(QDateTime(m_dateEmbauche, QTime(0, 0))));
    query.bindValue(QStringLiteral(":tel"), m_telephone);
    query.bindValue(QStringLiteral(":poste"), m_poste);
    query.bindValue(QStringLiteral(":adresse"), m_adresse);
    query.bindValue(QStringLiteral(":email"), m_email);

    if (query.exec())
        return true;
    if (errorMessage)
        *errorMessage = query.lastError().text().trimmed();
    return false;
}

bool Employe::supprimer(const QString &cin, QString *errorMessage)
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare(QStringLiteral("DELETE FROM EMPLOYES WHERE CIN=:cin"));
    query.bindValue(QStringLiteral(":cin"), cin);
    if (query.exec())
        return true;
    if (errorMessage)
        *errorMessage = query.lastError().text().trimmed();
    return false;
}

bool Employe::exportPdfFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Table invalide.");
        return false;
    }

    QPdfWriter writer(filePath);
    writer.setResolution(96);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setTitle(QStringLiteral("Liste des employes"));

    const QString exportDate = QDate::currentDate().toString(QStringLiteral("dd/MM/yyyy"));

    QString html;
    html += QStringLiteral("<div style='margin:40px auto; width:95%;'>");
    html += QStringLiteral("<div style='display:flex; justify-content:space-between; align-items:center;'>");
    html += QStringLiteral("<h1 style='font-family:Arial; font-weight:bold; font-size:20pt; margin:0;'>Liste des employes</h1>");
    html += QStringLiteral("<div style='font-family:Arial; font-size:10pt; text-align:right;'>");
    html += QStringLiteral("Date d'export : ") + exportDate.toHtmlEscaped();
    html += QStringLiteral("</div></div><hr style='margin:12pt 0;'>");
    html += QStringLiteral(
        "<table border='1' cellspacing='0' cellpadding='5' "
        "style='width:100%; font-size:9pt; font-family:Arial; border-collapse:collapse; table-layout:fixed;'>");
    html += QStringLiteral("<tr>");
    for (int c = 0; c < table->columnCount(); ++c) {
        const QString ht = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text() : QString();
        html += QStringLiteral("<th style='background-color:#eeeeee; text-align:left; padding:4px;'>")
            + ht.toHtmlEscaped() + QStringLiteral("</th>");
    }
    html += QStringLiteral("</tr>");

    bool alternate = false;
    for (int r = 0; r < table->rowCount(); ++r) {
        if (table->isRowHidden(r))
            continue;
        const QString bg = alternate ? QStringLiteral("#f9f9f9") : QStringLiteral("#ffffff");
        alternate = !alternate;
        html += QStringLiteral("<tr style='background-color:") + bg + QStringLiteral(";'>");
        for (int c = 0; c < table->columnCount(); ++c) {
            const QString cell = table->item(r, c) ? table->item(r, c)->text() : QString();
            const QString tdStyle = (c == 9)
                ? QStringLiteral("padding:4px; white-space:normal; word-break:break-word;")
                : QStringLiteral("padding:4px; white-space:nowrap;");
            html += QStringLiteral("<td style='") + tdStyle + QStringLiteral("'>") + cell.toHtmlEscaped()
                + QStringLiteral("</td>");
        }
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table></div>");

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(QSizeF(writer.width(), writer.height()));

    QPainter painter(&writer);
    doc.drawContents(&painter);
    return true;
}

bool Employe::exportCsvFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage)
{
    if (!table) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Table invalide.");
        return false;
    }
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible de creer le fichier.");
        return false;
    }
    QTextStream out(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
    out.setGenerateByteOrderMark(true);

    for (int c = 0; c < table->columnCount(); ++c) {
        if (c)
            out << QLatin1Char(';');
        QString ht = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text() : QString();
        ht.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        out << QLatin1Char('"') << ht << QLatin1Char('"');
    }
    out << QLatin1Char('\n');

    for (int r = 0; r < table->rowCount(); ++r) {
        if (table->isRowHidden(r))
            continue;
        for (int c = 0; c < table->columnCount(); ++c) {
            if (c)
                out << QLatin1Char(';');
            const QString cell = table->item(r, c) ? table->item(r, c)->text() : QString();
            QString escaped = cell;
            escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
            out << QLatin1Char('"') << escaped << QLatin1Char('"');
        }
        out << QLatin1Char('\n');
    }
    return true;
}
