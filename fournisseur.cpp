#include "fournisseur.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSpinBox>
#include <QTableWidgetItem>

namespace {
static bool columnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME=UPPER(:t) AND COLUMN_NAME=UPPER(:c)");
    q.bindValue(":t", table);
    q.bindValue(":c", column);
    if (!q.exec() || !q.next()) return false;
    return q.value(0).toInt() > 0;
}

static QString commandesColumnName()
{
    if (columnExists("FOURNISSEURS", "NB_COMMANDES")) return "NB_COMMANDES";
    if (columnExists("FOURNISSEURS", "COMMANDES")) return "COMMANDES";
    return "NB_COMMANDES";
}

static QString slaColumnName()
{
    if (columnExists("FOURNISSEURS", "SLA_JOURS")) return "SLA_JOURS";
    if (columnExists("FOURNISSEURS", "SLA")) return "SLA";
    return "SLA_JOURS";
}

static QString zoneColumnName()
{
    if (columnExists("FOURNISSEURS", "PAYS")) return "PAYS";
    if (columnExists("FOURNISSEURS", "ZONE_GEO")) return "ZONE_GEO";
    if (columnExists("FOURNISSEURS", "ZONE")) return "ZONE";
    return "PAYS";
}

static QString emailColumnName()
{
    if (columnExists("FOURNISSEURS", "EMAIL_ACHATS")) return "EMAIL_ACHATS";
    if (columnExists("FOURNISSEURS", "EMAIL")) return "EMAIL";
    if (columnExists("FOURNISSEURS", "MAIL")) return "MAIL";
    return "EMAIL_ACHATS";
}

static QString nomColumnName()
{
    if (columnExists("FOURNISSEURS", "NOM")) return "NOM";
    if (columnExists("FOURNISSEURS", "RAISON_SOCIALE")) return "RAISON_SOCIALE";
    if (columnExists("FOURNISSEURS", "RAISON")) return "RAISON";
    return "NOM";
}

static QString fiabiliteColumnName()
{
    if (columnExists("FOURNISSEURS", "PRENOM")) return "PRENOM";
    if (columnExists("FOURNISSEURS", "FIABILITE")) return "FIABILITE";
    if (columnExists("FOURNISSEURS", "TELEPHONE")) return "TELEPHONE";
    if (columnExists("FOURNISSEURS", "CONTACT")) return "CONTACT";
    return "PRENOM";
}

static QString codeColumnName()
{
    if (columnExists("FOURNISSEURS", "ID")) return "ID";
    if (columnExists("FOURNISSEURS", "CODE")) return "CODE";
    if (columnExists("FOURNISSEURS", "CIN")) return "CIN";
    return "ID";
}

static QString textExpr(const QString &column)
{
    return QStringLiteral("LOWER(TO_CHAR(%1))").arg(column);
}
}

FournisseurManager::FournisseurManager(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent), m_ui(ui)
{
    setupTable();
    setupFormControls();
    setupConnections();
    QString err;
    if (!ensureSchema(&err)) {
        QMessageBox::warning(nullptr, "Fournisseurs", "Preparation schema FOURNISSEURS impossible:\n" + err);
    }
    refreshTable(QString());
}

void FournisseurManager::setupTable()
{
    m_ui->employeeTable_2->setColumnCount(7);
    m_ui->employeeTable_2->setHorizontalHeaderLabels(
        {"Code", "Raison sociale", "Fiabilite", "Email achats", "Zone", "SLA (jours)", "Commandes"});
    m_ui->employeeTable_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->employeeTable_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->employeeTable_2->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->employeeTable_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void FournisseurManager::setupFormControls()
{
    m_ui->lineEditCIN_2->setMaxLength(20);
    m_ui->lineEditNom_2->setMaxLength(80);
    m_ui->lineEditPrenom_2->setMaxLength(30);
    m_ui->lineEdit->setMaxLength(120);
    m_ui->lineEditEmail_2->setMaxLength(50);
    m_ui->lineEdit_2->setMaxLength(3);

    if (!m_ui->formOuterLayout_2 || !m_ui->employeeFormBox_2) {
        return;
    }

    int insertIndex = m_ui->formOuterLayout_2->count();
    for (int i = 0; i < m_ui->formOuterLayout_2->count(); ++i) {
        QLayoutItem *it = m_ui->formOuterLayout_2->itemAt(i);
        if (it && it->layout() == m_ui->formBtnLayout_2) {
            insertIndex = i;
            break;
        }
    }

    auto *row = new QHBoxLayout();
    auto *lbl = new QLabel("Nb commandes", m_ui->employeeFormBox_2);
    m_spinCommandes = new QSpinBox(m_ui->employeeFormBox_2);
    m_spinCommandes->setRange(0, 1000000);
    m_spinCommandes->setValue(0);
    row->addWidget(lbl);
    row->addWidget(m_spinCommandes);
    m_ui->formOuterLayout_2->insertLayout(insertIndex, row);
}

void FournisseurManager::setupConnections()
{
    connect(m_ui->btnAjouter_5, &QPushButton::clicked, this, [this]() {
        QString err;
        if (!validateForm(&err)) {
            QMessageBox::warning(nullptr, "Fournisseur", err);
            return;
        }
        const FournisseurData d = readFormData();
        const bool exists = codeExistsDb(d.code, QString(), &err);
        if (!err.isEmpty()) {
            QMessageBox::warning(nullptr, "Fournisseur", err);
            return;
        }
        if (exists) {
            QMessageBox::warning(nullptr, "Fournisseur", "Code deja utilise.");
            return;
        }
        if (!ajouterDb(d, &err)) {
            QMessageBox::critical(nullptr, "Fournisseur", err);
            return;
        }
        refreshTable(m_ui->lineEditSearch_2->text());
        emit fournisseursChanged();
        clearForm();
        QMessageBox::information(nullptr, "Fournisseur", "Fournisseur ajoute.");
    });

    connect(m_ui->btnModifier_2, &QPushButton::clicked, this, [this]() {
        const int row = m_ui->employeeTable_2->currentRow();
        if (row < 0) {
            QMessageBox::information(nullptr, "Fournisseur", "Selectionnez un fournisseur.");
            return;
        }
        QTableWidgetItem *codeItem = m_ui->employeeTable_2->item(row, 0);
        if (!codeItem) {
            QMessageBox::warning(nullptr, "Fournisseur", "Ligne invalide.");
            return;
        }
        const QString selectedCode = codeItem->text().trimmed();
        QString err;
        if (!validateForm(&err)) {
            QMessageBox::warning(nullptr, "Fournisseur", err);
            return;
        }
        const FournisseurData d = readFormData();
        const bool exists = codeExistsDb(d.code, selectedCode, &err);
        if (!err.isEmpty()) {
            QMessageBox::warning(nullptr, "Fournisseur", err);
            return;
        }
        if (exists) {
            QMessageBox::warning(nullptr, "Fournisseur", "Un autre fournisseur utilise deja ce code.");
            return;
        }
        if (!modifierDb(selectedCode, d, &err)) {
            QMessageBox::critical(nullptr, "Fournisseur", err);
            return;
        }
        refreshTable(m_ui->lineEditSearch_2->text());
        emit fournisseursChanged();
        clearForm();
        QMessageBox::information(nullptr, "Fournisseur", "Fournisseur modifie.");
    });

    connect(m_ui->btnSupprimer_2, &QPushButton::clicked, this, [this]() {
        const int row = m_ui->employeeTable_2->currentRow();
        if (row < 0) {
            QMessageBox::information(nullptr, "Fournisseur", "Selectionnez un fournisseur.");
            return;
        }
        QTableWidgetItem *codeItem = m_ui->employeeTable_2->item(row, 0);
        if (!codeItem) {
            QMessageBox::warning(nullptr, "Fournisseur", "Code introuvable.");
            return;
        }
        QString err;
        if (!supprimerDb(codeItem->text().trimmed(), &err)) {
            QMessageBox::critical(nullptr, "Fournisseur", err);
            return;
        }
        refreshTable(m_ui->lineEditSearch_2->text());
        emit fournisseursChanged();
        clearForm();
        QMessageBox::information(nullptr, "Fournisseur", "Fournisseur supprime.");
    });

    connect(m_ui->employeeTable_2, &QTableWidget::cellClicked, this, [this](int row, int) {
        loadRowToForm(row);
    });
    connect(m_ui->btnRechercher_6, &QPushButton::clicked, this, [this]() {
        refreshTable(m_ui->lineEditSearch_2->text());
    });
    connect(m_ui->lineEditSearch_2, &QLineEdit::textChanged, this, [this](const QString &txt) {
        refreshTable(txt);
    });
    if (QComboBox *combo = m_ui->page->findChild<QComboBox *>("comboFiltreFournisseur")) {
        connect(combo, &QComboBox::currentTextChanged, this, [this](const QString &) {
            refreshTable(m_ui->lineEditSearch_2->text());
        });
    }
}

void FournisseurManager::clearForm()
{
    m_ui->lineEditCIN_2->clear();
    m_ui->lineEditNom_2->clear();
    m_ui->lineEditPrenom_2->clear();
    m_ui->lineEdit->clear();
    m_ui->lineEditEmail_2->clear();
    m_ui->lineEdit_2->clear();
    if (m_spinCommandes) m_spinCommandes->setValue(0);
}

void FournisseurManager::loadRowToForm(int row)
{
    if (row < 0 || row >= m_ui->employeeTable_2->rowCount()) return;
    FournisseurData d;
    d.code = m_ui->employeeTable_2->item(row, 0) ? m_ui->employeeTable_2->item(row, 0)->text() : "";
    d.raisonSociale = m_ui->employeeTable_2->item(row, 1) ? m_ui->employeeTable_2->item(row, 1)->text() : "";
    d.fiabilite = m_ui->employeeTable_2->item(row, 2) ? m_ui->employeeTable_2->item(row, 2)->text() : "";
    d.emailAchats = m_ui->employeeTable_2->item(row, 3) ? m_ui->employeeTable_2->item(row, 3)->text() : "";
    d.zone = m_ui->employeeTable_2->item(row, 4) ? m_ui->employeeTable_2->item(row, 4)->text() : "";
    d.slaJours = m_ui->employeeTable_2->item(row, 5) ? m_ui->employeeTable_2->item(row, 5)->text().toInt() : 0;
    d.commandes = m_ui->employeeTable_2->item(row, 6) ? m_ui->employeeTable_2->item(row, 6)->text().toInt() : 0;
    writeFormData(d);
}

void FournisseurManager::refreshTable(const QString &keyword)
{
    QList<FournisseurData> rows;
    QString err;
    if (!chargerDb(rows, keyword, &err)) {
        QMessageBox::warning(nullptr, "Fournisseurs", err);
        return;
    }

    QString filtre = "Tous";
    if (QComboBox *combo = m_ui->page->findChild<QComboBox *>("comboFiltreFournisseur")) {
        filtre = combo->currentText();
    }

    m_ui->employeeTable_2->setRowCount(0);
    for (const FournisseurData &d : rows) {
        bool keep = true;
        const int score = d.fiabilite.toInt();
        if (filtre == "Score >= 80") keep = (score >= 80);
        else if (filtre == "SLA <= 7") keep = (d.slaJours <= 7);
        else if (filtre == "Risque eleve") keep = (score < 40 || d.slaJours > 10);
        else if (filtre == "Risque faible") keep = (score >= 80 && d.slaJours <= 7);
        if (!keep) continue;

        const int row = m_ui->employeeTable_2->rowCount();
        m_ui->employeeTable_2->insertRow(row);
        m_ui->employeeTable_2->setItem(row, 0, new QTableWidgetItem(d.code));
        m_ui->employeeTable_2->setItem(row, 1, new QTableWidgetItem(d.raisonSociale));
        m_ui->employeeTable_2->setItem(row, 2, new QTableWidgetItem(d.fiabilite));
        m_ui->employeeTable_2->setItem(row, 3, new QTableWidgetItem(d.emailAchats));
        m_ui->employeeTable_2->setItem(row, 4, new QTableWidgetItem(d.zone));
        m_ui->employeeTable_2->setItem(row, 5, new QTableWidgetItem(QString::number(d.slaJours)));
        m_ui->employeeTable_2->setItem(row, 6, new QTableWidgetItem(QString::number(d.commandes)));
    }
}

bool FournisseurManager::validateForm(QString *errorMessage) const
{
    const QString code = m_ui->lineEditCIN_2->text().trimmed().toUpper();
    const QString nom = m_ui->lineEditNom_2->text().trimmed();
    const QString fiabilite = m_ui->lineEditPrenom_2->text().trimmed();
    const QString email = m_ui->lineEdit->text().trimmed();
    const QString zone = m_ui->lineEditEmail_2->text().trimmed();
    const QString sla = m_ui->lineEdit_2->text().trimmed();

    if (code.isEmpty()) { if (errorMessage) *errorMessage = "Code fournisseur obligatoire."; return false; }
    const QString codeCol = codeColumnName();
    if (codeCol == "ID") {
        if (!QRegularExpression("^[0-9]{1,10}$").match(code).hasMatch()) {
            if (errorMessage) *errorMessage = "ID fournisseur invalide (chiffres uniquement)."; return false;
        }
    } else if (!QRegularExpression("^[A-Z0-9_-]{2,20}$").match(code).hasMatch()) {
        if (errorMessage) *errorMessage = "Code invalide (2-20, lettres/chiffres/_/-)."; return false;
    }
    if (nom.isEmpty()) { if (errorMessage) *errorMessage = "Raison sociale obligatoire."; return false; }
    if (fiabilite.isEmpty()) { if (errorMessage) *errorMessage = "Fiabilite obligatoire."; return false; }
    if (!QRegularExpression("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$",
                            QRegularExpression::CaseInsensitiveOption).match(email).hasMatch()) {
        if (errorMessage) *errorMessage = "Format email invalide."; return false;
    }
    if (zone.isEmpty()) { if (errorMessage) *errorMessage = "Zone obligatoire."; return false; }
    if (!QRegularExpression("^[0-9]{1,3}$").match(sla).hasMatch() || sla.toInt() <= 0) {
        if (errorMessage) *errorMessage = "SLA invalide (nombre de jours > 0)."; return false;
    }
    return true;
}

FournisseurManager::FournisseurData FournisseurManager::readFormData() const
{
    FournisseurData d;
    d.code = m_ui->lineEditCIN_2->text().trimmed().toUpper();
    d.raisonSociale = m_ui->lineEditNom_2->text().trimmed();
    d.fiabilite = m_ui->lineEditPrenom_2->text().trimmed();
    d.emailAchats = m_ui->lineEdit->text().trimmed();
    d.zone = m_ui->lineEditEmail_2->text().trimmed();
    d.slaJours = m_ui->lineEdit_2->text().trimmed().toInt();
    if (d.slaJours <= 0) d.slaJours = 1;
    d.commandes = m_spinCommandes ? m_spinCommandes->value() : 0;
    return d;
}

void FournisseurManager::writeFormData(const FournisseurData &d)
{
    m_ui->lineEditCIN_2->setText(d.code);
    m_ui->lineEditNom_2->setText(d.raisonSociale);
    m_ui->lineEditPrenom_2->setText(d.fiabilite);
    m_ui->lineEdit->setText(d.emailAchats);
    m_ui->lineEditEmail_2->setText(d.zone);
    m_ui->lineEdit_2->setText(QString::number(d.slaJours));
    if (m_spinCommandes) m_spinCommandes->setValue(d.commandes);
}

bool FournisseurManager::ensureSchema(QString *errorMessage) const
{
    if (!QSqlDatabase::database().isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de donnees fermee.";
        return false;
    }
    QSqlQuery q;
    const QString sql =
        "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE FOURNISSEURS ("
        "ID NUMBER PRIMARY KEY, "
        "NOM VARCHAR2(120) NOT NULL, "
        "PRENOM VARCHAR2(30), "
        "EMAIL_ACHATS VARCHAR2(150) NOT NULL, "
        "PAYS VARCHAR2(80) NOT NULL, "
        "SLA_JOURS NUMBER(3) NOT NULL, "
        "NB_COMMANDES NUMBER DEFAULT 0 NOT NULL"
        ")'; "
        "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;";
    if (!q.exec(sql)) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }

    // Compatibilite avec anciens schemas: certains projets utilisent COMMANDES / SLA.
    if (!columnExists("FOURNISSEURS", "NB_COMMANDES") && !columnExists("FOURNISSEURS", "COMMANDES")) {
        QSqlQuery alter;
        if (!alter.exec("ALTER TABLE FOURNISSEURS ADD (NB_COMMANDES NUMBER DEFAULT 0 NOT NULL)")) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    if (!columnExists("FOURNISSEURS", "SLA_JOURS") && !columnExists("FOURNISSEURS", "SLA")) {
        QSqlQuery alter;
        if (!alter.exec("ALTER TABLE FOURNISSEURS ADD (SLA_JOURS NUMBER(3) DEFAULT 1 NOT NULL)")) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    if (!columnExists("FOURNISSEURS", "ZONE_GEO") &&
        !columnExists("FOURNISSEURS", "ZONE") &&
        !columnExists("FOURNISSEURS", "PAYS")) {
        QSqlQuery alter;
        if (!alter.exec("ALTER TABLE FOURNISSEURS ADD (ZONE_GEO VARCHAR2(80) DEFAULT 'N/A' NOT NULL)")) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    if (!columnExists("FOURNISSEURS", "EMAIL_ACHATS") &&
        !columnExists("FOURNISSEURS", "EMAIL") &&
        !columnExists("FOURNISSEURS", "MAIL")) {
        QSqlQuery alter;
        if (!alter.exec("ALTER TABLE FOURNISSEURS ADD (EMAIL_ACHATS VARCHAR2(150) DEFAULT 'na@na.com' NOT NULL)")) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    return true;
}

bool FournisseurManager::codeExistsDb(const QString &code, const QString &excludeCode, QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    QSqlQuery q;
    const QString codeCol = codeColumnName();
    if (excludeCode.isEmpty()) {
        q.prepare(QString("SELECT COUNT(*) FROM FOURNISSEURS WHERE TO_CHAR(%1)=:c").arg(codeCol));
        q.bindValue(":c", code.trimmed());
    } else {
        q.prepare(QString("SELECT COUNT(*) FROM FOURNISSEURS WHERE TO_CHAR(%1)=:c AND TO_CHAR(%1)<>:e").arg(codeCol));
        q.bindValue(":c", code.trimmed());
        q.bindValue(":e", excludeCode.trimmed());
    }
    if (!q.exec() || !q.next()) {
        if (errorMessage) *errorMessage = "Verification code impossible: " + q.lastError().text();
        return false;
    }
    return q.value(0).toInt() > 0;
}

bool FournisseurManager::ajouterDb(const FournisseurData &d, QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    const QString cmdCol = commandesColumnName();
    const QString slaCol = slaColumnName();
    const QString zoneCol = zoneColumnName();
    const QString mailCol = emailColumnName();
    const QString nomCol = nomColumnName();
    const QString fiaCol = fiabiliteColumnName();
    const QString codeCol = codeColumnName();
    QSqlQuery q;
    q.prepare(QString("INSERT INTO FOURNISSEURS (%1, %2, %3, %4, %5, %6, %7) "
                      "VALUES (:c,:r,:f,:e,:z,:s,:n)").arg(codeCol, nomCol, fiaCol, mailCol, zoneCol, slaCol, cmdCol));
    q.bindValue(":c", d.code);
    q.bindValue(":r", d.raisonSociale);
    q.bindValue(":f", d.fiabilite);
    q.bindValue(":e", d.emailAchats.toLower());
    q.bindValue(":z", d.zone);
    q.bindValue(":s", d.slaJours);
    q.bindValue(":n", d.commandes);
    if (q.exec()) return true;
    if (errorMessage) *errorMessage = "Ajout impossible: " + q.lastError().text();
    return false;
}

bool FournisseurManager::modifierDb(const QString &selectedCode, const FournisseurData &d, QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    const QString cmdCol = commandesColumnName();
    const QString slaCol = slaColumnName();
    const QString zoneCol = zoneColumnName();
    const QString mailCol = emailColumnName();
    const QString nomCol = nomColumnName();
    const QString fiaCol = fiabiliteColumnName();
    const QString codeCol = codeColumnName();
    QSqlQuery q;
    q.prepare(QString("UPDATE FOURNISSEURS SET %1=:newc, %2=:r, %3=:f, %4=:e, "
                      "%5=:z, %6=:s, %7=:n WHERE TO_CHAR(%1)=:oldc")
                  .arg(codeCol, nomCol, fiaCol, mailCol, zoneCol, slaCol, cmdCol));
    q.bindValue(":newc", d.code);
    q.bindValue(":r", d.raisonSociale);
    q.bindValue(":f", d.fiabilite);
    q.bindValue(":e", d.emailAchats.toLower());
    q.bindValue(":z", d.zone);
    q.bindValue(":s", d.slaJours);
    q.bindValue(":n", d.commandes);
    q.bindValue(":oldc", selectedCode);
    if (!q.exec()) {
        if (errorMessage) *errorMessage = "Modification impossible: " + q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (errorMessage) *errorMessage = "Aucun fournisseur modifie.";
        return false;
    }
    return true;
}

bool FournisseurManager::supprimerDb(const QString &code, QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    QSqlQuery q;
    q.prepare(QString("DELETE FROM FOURNISSEURS WHERE TO_CHAR(%1)=:c").arg(codeColumnName()));
    q.bindValue(":c", code.trimmed());
    if (!q.exec()) {
        if (errorMessage) *errorMessage = "Suppression impossible: " + q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (errorMessage) *errorMessage = "Fournisseur introuvable.";
        return false;
    }
    return true;
}

bool FournisseurManager::chargerDb(QList<FournisseurData> &rows, const QString &keyword, QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    const QString cmdCol = commandesColumnName();
    const QString slaCol = slaColumnName();
    const QString zoneCol = zoneColumnName();
    const QString mailCol = emailColumnName();
    const QString nomCol = nomColumnName();
    const QString fiaCol = fiabiliteColumnName();
    const QString codeCol = codeColumnName();
    const QString codeExpr = textExpr(codeCol);
    const QString nomExpr = textExpr(nomCol);
    const QString fiaExpr = textExpr(fiaCol);
    const QString mailExpr = textExpr(mailCol);
    const QString zoneExpr = textExpr(zoneCol);
    QSqlQuery q;
    const QString mot = keyword.trimmed().toLower();
    if (mot.isEmpty()) {
        q.prepare(QString("SELECT %1, %2, %3, %4, %5, %6, %7 "
                          "FROM FOURNISSEURS ORDER BY %1").arg(codeCol, nomCol, fiaCol, mailCol, zoneCol, slaCol, cmdCol));
    } else {
        q.prepare(QString("SELECT %1, %2, %3, %4, %5, %6, %7 "
                          "FROM FOURNISSEURS "
                          "WHERE %8 LIKE :k OR %9 LIKE :k OR %10 LIKE :k OR "
                          "%11 LIKE :k OR %12 LIKE :k "
                          "ORDER BY %1")
                      .arg(codeCol, nomCol, fiaCol, mailCol, zoneCol, slaCol, cmdCol,
                           codeExpr, nomExpr, fiaExpr, mailExpr, zoneExpr));
        q.bindValue(":k", "%" + mot + "%");
    }
    if (!q.exec()) {
        if (errorMessage) *errorMessage = "Chargement impossible: " + q.lastError().text();
        return false;
    }
    rows.clear();
    while (q.next()) {
        FournisseurData d;
        d.code = q.value(0).toString();
        d.raisonSociale = q.value(1).toString();
        d.fiabilite = q.value(2).toString();
        d.emailAchats = q.value(3).toString();
        d.zone = q.value(4).toString();
        d.slaJours = q.value(5).toInt();
        if (d.slaJours <= 0) d.slaJours = 1;
        d.commandes = q.value(6).toInt();
        rows.append(d);
    }
    return true;
}
