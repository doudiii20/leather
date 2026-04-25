#include "fournisseur.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSpinBox>
#include <QStyle>
#include <QStringConverter>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QWidget>

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
    m_ui->employeeTable_2->setColumnCount(9);
    m_ui->employeeTable_2->setHorizontalHeaderLabels(
        {"CODE", "RAISON SOCIALE", "FIABILITE", "EMAIL ACHATS", "ZONE", "SLA (JOURS)", "COMMANDES", "SUPPR.", "MODIF."});
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
    if (!m_ui->employeeFormBox_2->property("fournisseurFicheModernized").toBool()) {
        m_ui->employeeFormBox_2->setTitle(QString());
        m_ui->employeeFormBox_2->setStyleSheet(QStringLiteral(
            "QGroupBox {"
            "  background: #ffffff;"
            "  border: 1px solid #dde4ee;"
            "  border-radius: 12px;"
            "}"
            "QLabel { color: #1f2937; font-weight: 600; }"
            "QLineEdit, QComboBox, QSpinBox {"
            "  min-height: 34px;"
            "  border: 1px solid #cfd8e6;"
            "  border-radius: 8px;"
            "  padding: 0 10px;"
            "  background: #ffffff;"
            "}"
            "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #b8c4d3; }"));

        if (m_ui->formRow1_2) {
            m_ui->formOuterLayout_2->removeItem(m_ui->formRow1_2);
            delete m_ui->formRow1_2;
            m_ui->formRow1_2 = nullptr;
        }
        if (m_ui->formRow2_2) {
            m_ui->formOuterLayout_2->removeItem(m_ui->formRow2_2);
            delete m_ui->formRow2_2;
            m_ui->formRow2_2 = nullptr;
        }

        const QList<QWidget *> legacyLabels = {
            m_ui->labelCIN_2, m_ui->labelNom_2, m_ui->labelPrenom_2, m_ui->labelSexe_2, m_ui->labelSalaire_2, m_ui->labelDateEmbauche_2
        };
        for (QWidget *w : legacyLabels) {
            if (w)
                w->hide();
        }

        auto *titleLab = new QLabel(QStringLiteral("Fiche fournisseurs"), m_ui->employeeFormBox_2);
        titleLab->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #000000; padding: 2px 4px;"));
        m_ui->formOuterLayout_2->insertWidget(0, titleLab);

        auto *formWrap = new QWidget(m_ui->employeeFormBox_2);
        formWrap->setStyleSheet(QStringLiteral("background:#ffffff;"));
        auto *formLay = new QFormLayout(formWrap);
        formLay->setContentsMargins(4, 2, 4, 4);
        formLay->setHorizontalSpacing(12);
        formLay->setVerticalSpacing(12);
        formLay->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        formLay->setFormAlignment(Qt::AlignTop);
        formLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        m_ui->lineEditCIN_2->setPlaceholderText(QStringLiteral("Entrez le code partenaire"));
        m_ui->lineEditNom_2->setPlaceholderText(QStringLiteral("Entrez la raison sociale"));
        m_ui->lineEditPrenom_2->setPlaceholderText(QStringLiteral("0 a 100"));
        m_ui->lineEdit->setPlaceholderText(QStringLiteral("Entrez l'email achats"));
        m_ui->lineEditEmail_2->setPlaceholderText(QStringLiteral("Entrez la zone logistique"));
        m_ui->lineEdit_2->setPlaceholderText(QStringLiteral("SLA livraison (jours)"));

        m_spinCommandes = new QSpinBox(m_ui->employeeFormBox_2);
        m_spinCommandes->setRange(0, 1000000);
        m_spinCommandes->setValue(0);
        m_spinCommandes->setMinimumHeight(34);
        m_spinCommandes->setMaximumWidth(220);
        m_spinCommandes->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        formLay->addRow(new QLabel(QStringLiteral("Code"), formWrap), m_ui->lineEditCIN_2);
        formLay->addRow(new QLabel(QStringLiteral("Raison sociale"), formWrap), m_ui->lineEditNom_2);
        formLay->addRow(new QLabel(QStringLiteral("Fiabilite"), formWrap), m_ui->lineEditPrenom_2);
        formLay->addRow(new QLabel(QStringLiteral("E-mail achats"), formWrap), m_ui->lineEdit);
        formLay->addRow(new QLabel(QStringLiteral("Zone"), formWrap), m_ui->lineEditEmail_2);
        formLay->addRow(new QLabel(QStringLiteral("SLA (jours)"), formWrap), m_ui->lineEdit_2);
        formLay->addRow(new QLabel(QStringLiteral("Nb commandes"), formWrap), m_spinCommandes);
        const QList<QWidget *> fournisseurFields = {
            static_cast<QWidget *>(m_ui->lineEditCIN_2),
            static_cast<QWidget *>(m_ui->lineEditNom_2),
            static_cast<QWidget *>(m_ui->lineEditPrenom_2),
            static_cast<QWidget *>(m_ui->lineEdit),
            static_cast<QWidget *>(m_ui->lineEditEmail_2),
            static_cast<QWidget *>(m_ui->lineEdit_2),
            static_cast<QWidget *>(m_spinCommandes),
        };
        for (QWidget *w : fournisseurFields) {
            if (!w)
                continue;
            w->setMaximumWidth(220);
            w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }
        m_ui->formOuterLayout_2->insertWidget(1, formWrap);

        if (m_ui->formBtnLayout_2) {
            m_ui->formBtnLayout_2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            m_ui->formBtnLayout_2->setSpacing(10);
            for (int i = m_ui->formBtnLayout_2->count() - 1; i >= 0; --i) {
                QLayoutItem *it = m_ui->formBtnLayout_2->itemAt(i);
                if (it && it->spacerItem()) {
                    QLayoutItem *removed = m_ui->formBtnLayout_2->takeAt(i);
                    delete removed;
                }
            }
        }
        if (m_ui->btnAjouter_5) {
            m_ui->btnAjouter_5->setMinimumSize(180, 38);
            m_ui->btnAjouter_5->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: #f3f5f8;"
                "  border: 1px solid #c7d1de;"
                "  border-radius: 10px;"
                "  color: #000000;"
                "  font-weight: 600;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; }"
                "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; }"));
            m_ui->btnAjouter_5->setIcon(m_ui->btnAjouter_5->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
        }
        if (!m_cancelEditButton && m_ui->formBtnLayout_2) {
            m_cancelEditButton = new QPushButton(QStringLiteral("Annuler"), m_ui->employeeFormBox_2);
            m_cancelEditButton->setMinimumSize(110, 32);
            m_cancelEditButton->setCursor(Qt::PointingHandCursor);
            if (m_ui->btnAjouter_5)
                m_cancelEditButton->setStyleSheet(m_ui->btnAjouter_5->styleSheet());
            m_cancelEditButton->hide();
            m_ui->formBtnLayout_2->addWidget(m_cancelEditButton);
            connect(m_cancelEditButton, &QPushButton::clicked, this, [this]() { exitEditMode(true); });
        }
        if (m_ui->btnModifier_2) {
            m_ui->btnModifier_2->hide();
            if (m_ui->formBtnLayout_2)
                m_ui->formBtnLayout_2->removeWidget(m_ui->btnModifier_2);
        }
        if (m_ui->btnSupprimer_2) {
            m_ui->btnSupprimer_2->hide();
            if (m_ui->formBtnLayout_2)
                m_ui->formBtnLayout_2->removeWidget(m_ui->btnSupprimer_2);
        }
        if (m_ui->btnAjouter_2) {
            m_ui->btnAjouter_2->hide();
            if (m_ui->formBtnLayout_2)
                m_ui->formBtnLayout_2->removeWidget(m_ui->btnAjouter_2);
        }
        m_ui->employeeFormBox_2->setProperty("fournisseurFicheModernized", true);
    }
}

void FournisseurManager::setupConnections()
{
    connect(m_ui->btnAjouter_5, &QPushButton::clicked, this, [this]() {
        if (m_editMode) {
            QString err;
            if (!validateForm(&err)) {
                QMessageBox::warning(nullptr, "Fournisseur", err);
                return;
            }
            const FournisseurData d = readFormData();
            const bool exists = codeExistsDb(d.code, m_editCode, &err);
            if (!err.isEmpty()) {
                QMessageBox::warning(nullptr, "Fournisseur", err);
                return;
            }
            if (exists) {
                QMessageBox::warning(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Code deja pris."));
                return;
            }
            const QString codeAffiche = d.code.trimmed().isEmpty() ? m_editCode : d.code.trimmed();
            const QString confirmMsg = QStringLiteral("Confirmer la modification du fournisseur CODE %1 ?")
                                           .arg(codeAffiche);
            QWidget *dlgParent = qobject_cast<QWidget *>(parent());
            if (QMessageBox::question(dlgParent, QStringLiteral("Fournisseur"), confirmMsg,
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                != QMessageBox::Yes) {
                QMessageBox::information(dlgParent, QStringLiteral("Fournisseur"),
                                         QStringLiteral("Modification annulee."));
                return;
            }
            if (!modifierDb(m_editCode, d, &err)) {
                QMessageBox::critical(nullptr, "Fournisseur", err);
                return;
            }
            refreshTable(m_ui->lineEditSearch_2->text());
            emit fournisseursChanged();
            clearForm();
            exitEditMode(false);
            QMessageBox::information(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Modifie."));
            return;
        }
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
            QMessageBox::warning(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Code deja pris."));
            return;
        }
        if (!ajouterDb(d, &err)) {
            QMessageBox::critical(nullptr, "Fournisseur", err);
            return;
        }
        refreshTable(m_ui->lineEditSearch_2->text());
        emit fournisseursChanged();
        clearForm();
        QMessageBox::information(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Ajoute."));
    });

    connect(m_ui->btnModifier_2, &QPushButton::clicked, this, [this]() {
        const int row = m_ui->employeeTable_2->currentRow();
        if (row < 0) {
            QMessageBox::information(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Selectionnez une ligne."));
            return;
        }
        QTableWidgetItem *codeItem = m_ui->employeeTable_2->item(row, 0);
        if (!codeItem) {
            QMessageBox::warning(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Ligne invalide."));
            return;
        }
        const QString selectedCode = codeItem->text().trimmed();
        enterEditMode(selectedCode);
    });

    connect(m_ui->btnAjouter_2, &QPushButton::clicked, this, [this]() { exportFournisseursTable(); });

    connect(m_ui->btnSupprimer_2, &QPushButton::clicked, this, [this]() {
        QString code;
        const int row = m_ui->employeeTable_2->currentRow();
        if (row >= 0) {
            QTableWidgetItem *codeItem = m_ui->employeeTable_2->item(row, 0);
            if (codeItem)
                code = codeItem->text().trimmed();
        }
        if (code.isEmpty() && m_ui->lineEditCIN_2)
            code = m_ui->lineEditCIN_2->text().trimmed();
        if (code.isEmpty()) {
            QMessageBox::information(nullptr, QStringLiteral("Fournisseur"),
                                     QStringLiteral("Sélectionnez un fournisseur dans le tableau ou saisissez le code partenaire."));
            return;
        }
        QWidget *dlgParent = qobject_cast<QWidget *>(parent());
        if (QMessageBox::question(dlgParent, QStringLiteral("Fournisseur"),
                                  QStringLiteral("Supprimer le fournisseur %1 ?").arg(code),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes) {
            return;
        }
        QString err;
        if (!supprimerDb(code, &err)) {
            QMessageBox::critical(dlgParent, QStringLiteral("Fournisseur"), err);
            return;
        }
        refreshTable(m_ui->lineEditSearch_2->text());
        emit fournisseursChanged();
        clearForm();
        QMessageBox::information(nullptr, QStringLiteral("Fournisseur"), QStringLiteral("Supprime."));
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

void FournisseurManager::exportFournisseursTable()
{
    if (m_ui->employeeTable_2->rowCount() <= 0) {
        QMessageBox::information(qobject_cast<QWidget *>(parent()), "Export", "Aucun fournisseur a exporter.");
        return;
    }

    QWidget *dlgParent = qobject_cast<QWidget *>(parent());
    const QString defaultName =
        "fournisseurs_" + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")) + ".xls";
    const QString filePath = QFileDialog::getSaveFileName(
        dlgParent,
        QStringLiteral("Exporter les fournisseurs"),
        defaultName,
        QStringLiteral("Fichiers Excel (*.xls)"));

    if (filePath.trimmed().isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(dlgParent, "Export", "Erreur fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    QStringList headers;
    headers.reserve(m_ui->employeeTable_2->columnCount());
    for (int col = 0; col < m_ui->employeeTable_2->columnCount(); ++col) {
        QTableWidgetItem *headerItem = m_ui->employeeTable_2->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : QStringLiteral("COL_%1").arg(col));
    }
    out << headers.join('\t') << "\n";

    for (int row = 0; row < m_ui->employeeTable_2->rowCount(); ++row) {
        QStringList values;
        values.reserve(m_ui->employeeTable_2->columnCount());
        for (int col = 0; col < m_ui->employeeTable_2->columnCount(); ++col) {
            QTableWidgetItem *item = m_ui->employeeTable_2->item(row, col);
            QString v = item ? item->text() : QString();
            v.replace('\t', ' ');
            v.replace('\n', ' ');
            values << v;
        }
        out << values.join('\t') << "\n";
    }

    file.close();
    QMessageBox::information(dlgParent, "Export", "Export reussi.");
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

        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), m_ui->employeeTable_2);
        deleteBtn->setToolTip(QStringLiteral("Supprimer ce fournisseur"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        deleteBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #f3f5f8; border: 1px solid #c7d1de; border-radius: 10px; color: #000000; font-weight: 600; padding: 6px 12px; }"
            "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; }"
            "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; }"));
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            m_ui->employeeTable_2->setCurrentCell(row, 0);
            if (m_ui->btnSupprimer_2)
                m_ui->btnSupprimer_2->click();
        });
        m_ui->employeeTable_2->setCellWidget(row, 7, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), m_ui->employeeTable_2);
        updateBtn->setToolTip(QStringLiteral("Modifier ce fournisseur"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        updateBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #f3f5f8; border: 1px solid #c7d1de; border-radius: 10px; color: #000000; font-weight: 600; padding: 6px 12px; }"
            "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; }"
            "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; }"));
        connect(updateBtn, &QPushButton::clicked, this, [this, row]() {
            m_ui->employeeTable_2->setCurrentCell(row, 0);
            loadRowToForm(row);
            const QString code = m_ui->employeeTable_2->item(row, 0) ? m_ui->employeeTable_2->item(row, 0)->text().trimmed() : QString();
            if (!code.isEmpty())
                enterEditMode(code);
        });
        m_ui->employeeTable_2->setCellWidget(row, 8, updateBtn);
    }
}

void FournisseurManager::enterEditMode(const QString &code)
{
    if (code.trimmed().isEmpty())
        return;
    m_editMode = true;
    m_editCode = code.trimmed();
    m_editSnapshot = readFormData();
    if (m_ui->btnAjouter_5) {
        m_ui->btnAjouter_5->setText(QStringLiteral("Enregistrer"));
        if (m_cancelEditButton)
            m_ui->btnAjouter_5->setMinimumSize(m_cancelEditButton->minimumSize());
    }
    if (m_cancelEditButton)
        m_cancelEditButton->show();
    if (m_ui->employeeTable_2) {
        m_ui->employeeTable_2->setSelectionMode(QAbstractItemView::NoSelection);
        for (int r = 0; r < m_ui->employeeTable_2->rowCount(); ++r) {
            if (QWidget *w = m_ui->employeeTable_2->cellWidget(r, 7)) w->setEnabled(false);
            if (QWidget *w = m_ui->employeeTable_2->cellWidget(r, 8)) w->setEnabled(false);
        }
    }
}

void FournisseurManager::exitEditMode(bool restoreSnapshot)
{
    if (restoreSnapshot && m_editMode)
        writeFormData(m_editSnapshot);
    m_editMode = false;
    m_editCode.clear();
    m_editSnapshot = FournisseurData{};
    if (m_ui->btnAjouter_5)
        m_ui->btnAjouter_5->setText(QStringLiteral("Ajouter"));
    if (m_cancelEditButton)
        m_cancelEditButton->hide();
    if (m_ui->employeeTable_2) {
        m_ui->employeeTable_2->setSelectionMode(QAbstractItemView::SingleSelection);
        for (int r = 0; r < m_ui->employeeTable_2->rowCount(); ++r) {
            if (QWidget *w = m_ui->employeeTable_2->cellWidget(r, 7)) w->setEnabled(true);
            if (QWidget *w = m_ui->employeeTable_2->cellWidget(r, 8)) w->setEnabled(true);
        }
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
    QStringList errors;

    if (code.isEmpty())
        errors << QStringLiteral("- Code requis.");
    const QString codeCol = codeColumnName();
    if (codeCol == "ID") {
        if (!QRegularExpression("^[0-9]{1,10}$").match(code).hasMatch()) {
            errors << QStringLiteral("- ID : chiffres uniquement.");
        }
    } else if (!QRegularExpression("^[A-Z0-9_-]{2,20}$").match(code).hasMatch()) {
        errors << QStringLiteral("- Code 2-20 (A-Z, 0-9, _, -).");
    }
    if (nom.isEmpty())
        errors << QStringLiteral("- Raison sociale requise.");
    if (fiabilite.isEmpty())
        errors << QStringLiteral("- Fiabilite requise.");
    if (!QRegularExpression("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$",
                            QRegularExpression::CaseInsensitiveOption).match(email).hasMatch()) {
        errors << QStringLiteral("- Email invalide.");
    }
    if (zone.isEmpty())
        errors << QStringLiteral("- Zone requise.");
    if (!QRegularExpression("^[0-9]{1,3}$").match(sla).hasMatch() || sla.toInt() <= 0) {
        errors << QStringLiteral("- SLA : jours > 0.");
    }
    if (!errors.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Veuillez corriger :\n%1").arg(errors.join(QLatin1Char('\n')));
        return false;
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
        if (errorMessage) *errorMessage = QStringLiteral("Base fermee.");
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
    if (!columnExists("FOURNISSEURS", "LATITUDE")) {
        QSqlQuery alter;
        if (!alter.exec(QStringLiteral("ALTER TABLE FOURNISSEURS ADD (LATITUDE NUMBER)"))) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    if (!columnExists("FOURNISSEURS", "LONGITUDE")) {
        QSqlQuery alter;
        if (!alter.exec(QStringLiteral("ALTER TABLE FOURNISSEURS ADD (LONGITUDE NUMBER)"))) {
            if (errorMessage) *errorMessage = alter.lastError().text();
            return false;
        }
    }
    return true;
}

bool FournisseurManager::enregistrerCoordonneesGeo(const QString &codePartenaire,
                                                   double latitude,
                                                   double longitude,
                                                   QString *errorMessage) const
{
    QString err;
    if (!ensureSchema(&err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }
    const QString codeCol = codeColumnName();
    QSqlQuery q;
    q.prepare(QStringLiteral("UPDATE FOURNISSEURS SET LATITUDE=:la, LONGITUDE=:lo WHERE TO_CHAR(%1)=:c").arg(codeCol));
    q.bindValue(QStringLiteral(":la"), latitude);
    q.bindValue(QStringLiteral(":lo"), longitude);
    q.bindValue(QStringLiteral(":c"), codePartenaire.trimmed());
    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
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
