#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "commercestore.h"
#include "recommendationservice.h"
#include "chatbotservice.h"
#include "fournisseur.h"
#include "produit.h"
#include <QAbstractItemView>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QSqlDatabase>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QSizePolicy>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QStringConverter>
#include <QSqlQueryModel>
#include <memory>

namespace {
static bool mwColumnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME=UPPER(:t) AND COLUMN_NAME=UPPER(:c)");
    q.bindValue(":t", table);
    q.bindValue(":c", column);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}
}

// ------------------- Constructeur -------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_networkAccessManager = new QNetworkAccessManager(this);

    // Certains placeholders du Designer peuvent recouvrir la page fournisseurs
    // et bloquer les clics (champs/boutons "inaccessibles").
    if (ui->verticalLayoutWidget_3) {
        ui->verticalLayoutWidget_3->hide();
        ui->verticalLayoutWidget_3->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
    if (ui->horizontalLayoutWidget_4) {
        ui->horizontalLayoutWidget_4->hide();
        ui->horizontalLayoutWidget_4->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
    if (ui->horizontalLayoutWidget_3) {
        ui->horizontalLayoutWidget_3->hide();
        ui->horizontalLayoutWidget_3->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

    // Hide default menubar / statusbar
    menuBar()->hide();
    statusBar()->hide();

    // Stretch table columns to fill
    ui->employeeTable->horizontalHeader()->setStretchLastSection(true);
    ui->employeeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Ajustement de la table client
    ui->clientTable->horizontalHeader()->setStretchLastSection(true);
    ui->clientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->clientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->clientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->clientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->clientTable->setColumnCount(14);
    ui->clientTable->setHorizontalHeaderLabels({
        "ID", "Nom", "Prenom", "Email", "Telephone", "Adresse", "Statut",
        "Categorie", "Remise %", "Total achats", "Frequence", "Retards",
        "Limite credit", "Solde credit"
    });

    connectSidebar();
    setupClientValidators();
    setupClientUiEnhancements();
    connect(ui->clientTable, &QTableWidget::cellClicked, this, &MainWindow::on_clientTable_cellClicked);

    // Start on Accueil (page 0)
    ui->contentStack->setCurrentIndex(0);
    setActiveButton(ui->btnAccueil);

    // Connexion à la base de données
    if (connectToDatabase())
        qDebug() << "Connexion BD réussie";
    else
        qDebug() << "Connexion BD échouée";

    QString err;
    if (!Client::ensureSchema(&err)) {
        QMessageBox::critical(this, "Base de données", "Erreur de préparation du schéma CLIENT:\n" + err);
    }

    if (db.isOpen()) {
        QString cerr;
        if (!CommerceStore::ensureSchema(&cerr)) {
            QMessageBox::critical(this, "Base de données", "Erreur schéma commerce (produits/commandes):\n" + cerr);
        } else {
            CommerceStore::seedDemoCatalogIfEmpty(&cerr);
        }
    }

    if (db.isOpen()) {
        gestionFournisseur = new FournisseurManager(ui, this);
        setupFournisseurDashboardBlock();
        connect(gestionFournisseur, &FournisseurManager::fournisseursChanged, this, &MainWindow::refreshFournisseurDashboard);
        refreshFournisseurDashboard();
    } else {
        QMessageBox::warning(this, "Fournisseurs", "Module fournisseurs desactive: connexion base de donnees fermee.");
    }

    loadClients();
    refreshClientRecommendations();

    if (db.isOpen()) {
        setupProduitPage();
    }
}

void MainWindow::setupFournisseurDashboardBlock()
{
    if (!ui->chatGroupBox_2 || !ui->chatLayout_2) {
        return;
    }

    // Retouche 1: labels metier fournisseurs (affichage proche du projet ami).
    if (ui->labelCIN_2) ui->labelCIN_2->setText("Code partenaire");
    if (ui->labelNom_2) ui->labelNom_2->setText("Raison sociale");
    if (ui->labelPrenom_2) ui->labelPrenom_2->setText("Score fiabilite");
    if (ui->labelSexe_2) ui->labelSexe_2->setText("Email achats");
    if (ui->labelSalaire_2) ui->labelSalaire_2->setText("Zone logistique");
    if (ui->labelDateEmbauche_2) ui->labelDateEmbauche_2->setText("SLA livraison (jours)");
    if (ui->lineEditCIN_2) ui->lineEditCIN_2->setPlaceholderText("ex: FRN-001");
    if (ui->lineEditNom_2) ui->lineEditNom_2->setPlaceholderText("ex: Leather Supply Pro");
    if (ui->lineEditPrenom_2) ui->lineEditPrenom_2->setPlaceholderText("0 a 100");
    if (ui->lineEdit) ui->lineEdit->setPlaceholderText("contact@fournisseur.com");
    if (ui->lineEditEmail_2) ui->lineEditEmail_2->setPlaceholderText("ex: Maghreb / Europe");
    if (ui->lineEdit_2) ui->lineEdit_2->setPlaceholderText("ex: 5");

    if (ui->employesTitle_2) ui->employesTitle_2->setText("Gestion des fournisseurs");
    if (ui->employeeFormBox_2) ui->employeeFormBox_2->setTitle("Fiche Fournisseurs");
    if (ui->btnRechercher_6) ui->btnRechercher_6->setText("Rechercher");
    if (ui->lineEditSearch_2) {
        ui->lineEditSearch_2->setPlaceholderText("Recherche: texte | zone:Europe score>=80 sla<=7");
    }

    // Filtre rapide comme le projet ami
    if (ui->employesHeaderRow_2 && ui->page && !ui->page->findChild<QComboBox *>("comboFiltreFournisseur")) {
        auto *comboFiltre = new QComboBox(ui->page);
        comboFiltre->setObjectName("comboFiltreFournisseur");
        comboFiltre->setMinimumHeight(32);
        comboFiltre->setMinimumWidth(140);
        comboFiltre->addItems({"Tous", "Score >= 80", "SLA <= 7", "Risque eleve", "Risque faible"});
        comboFiltre->setStyleSheet(
            "QComboBox { border: 1px solid rgb(200, 150, 100); border-radius: 4px; padding: 4px 8px; "
            "font-size: 12px; color: rgb(88, 41, 0); background-color: white; }"
            "QComboBox::drop-down { border: none; width: 22px; }");
        auto *filterRow = new QHBoxLayout();
        filterRow->setSpacing(8);
        filterRow->setContentsMargins(0, 0, 0, 0);
        auto *filterLabel = new QLabel("Filtre", ui->page);
        filterLabel->setStyleSheet("font-size: 12px; font-weight: 700; color: rgb(88, 41, 0);");
        filterRow->addWidget(filterLabel, 0);
        filterRow->addWidget(comboFiltre, 0);
        ui->employesHeaderRow_2->insertLayout(2, filterRow);
    }

    if (ui->chatGroupBox_2) {
        ui->chatGroupBox_2->setTitle("Pilotage fournisseur (KPI & Notes)");
    }
    if (ui->chatDisplay_2) {
        ui->chatDisplay_2->setReadOnly(false);
        ui->chatDisplay_2->setPlaceholderText(
            "Saisissez les notes metier: incidents, plans d'action, conditions commerciales...");
        ui->chatDisplay_2->setMinimumHeight(48);
        ui->chatDisplay_2->setMaximumHeight(88);
    }
    if (ui->lineEditChat_2) {
        ui->lineEditChat_2->setPlaceholderText("KPI / Note rapide (ex: OTD 96%, SLA 5j)");
    }
    if (ui->btnSendChat_2) {
        ui->btnSendChat_2->setText("Ajouter note");
        connect(ui->btnSendChat_2, &QPushButton::clicked, this, [this]() {
            const QString quickNote = ui->lineEditChat_2->text().trimmed();
            if (quickNote.isEmpty()) return;
            const QString stamp = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
            ui->chatDisplay_2->append("[" + stamp + "] " + quickNote);
            ui->lineEditChat_2->clear();
        });
    }

    auto *title = new QLabel("Statistiques & Dashboard", ui->chatGroupBox_2);
    title->setStyleSheet("font-size: 12px; font-weight: 700; color: rgb(88, 41, 0);");

    m_fournisseurStatsSummary = new QLabel(ui->chatGroupBox_2);
    m_fournisseurStatsSummary->setWordWrap(true);
    m_fournisseurStatsSummary->setStyleSheet("font-size: 11px; color: #333;");
    m_fournisseurStatsSummary->setText("Chargement des statistiques...");

    m_fournisseurStatsToggle = new QPushButton("Afficher statistiques", ui->chatGroupBox_2);
    m_fournisseurStatsToggle->setCursor(Qt::PointingHandCursor);
    m_fournisseurStatsToggle->setMinimumHeight(30);
    m_fournisseurStatsToggle->setStyleSheet(
        "QPushButton { background-color: rgb(88, 41, 0); color: rgb(248, 241, 227); border: none; "
        "border-radius: 4px; font-size: 11px; font-weight: bold; padding: 6px 10px; }"
        "QPushButton:hover { background-color: rgb(110, 55, 10); }");

    m_fournisseurStatsChartsContainer = new QWidget(ui->chatGroupBox_2);
    auto *chartLay = new QVBoxLayout(m_fournisseurStatsChartsContainer);
    chartLay->setContentsMargins(0, 0, 0, 0);
    chartLay->setSpacing(6);
    m_fournisseurBarChart = new SupplierBarChartWidget(m_fournisseurStatsChartsContainer);
    m_fournisseurPieChart = new SupplierPieChartWidget(m_fournisseurStatsChartsContainer);
    chartLay->addWidget(m_fournisseurBarChart);
    chartLay->addWidget(m_fournisseurPieChart);
    m_fournisseurStatsChartsContainer->hide();

    auto *dashboardBlock = new QWidget(ui->chatGroupBox_2);
    auto *blockLay = new QVBoxLayout(dashboardBlock);
    blockLay->setContentsMargins(0, 0, 0, 0);
    blockLay->setSpacing(6);
    blockLay->addWidget(title);
    blockLay->addWidget(m_fournisseurStatsSummary);
    blockLay->addWidget(m_fournisseurStatsToggle);
    blockLay->addWidget(m_fournisseurStatsChartsContainer);
    ui->chatLayout_2->insertWidget(2, dashboardBlock);

    connect(m_fournisseurStatsToggle, &QPushButton::clicked, this, [this]() {
        const bool show = !m_fournisseurStatsChartsContainer->isVisible();
        m_fournisseurStatsChartsContainer->setVisible(show);
        m_fournisseurStatsToggle->setText(show ? "Masquer statistiques" : "Afficher statistiques");
        if (show) {
            refreshFournisseurDashboard();
        }
    });
}

void MainWindow::refreshFournisseurDashboard()
{
    if (!m_fournisseurStatsSummary) {
        return;
    }
    const QString cmdCol = mwColumnExists("FOURNISSEURS", "NB_COMMANDES") ? "NB_COMMANDES" : "COMMANDES";
    const QString codeCol = mwColumnExists("FOURNISSEURS", "CODE") ? "CODE" : (mwColumnExists("FOURNISSEURS", "ID") ? "ID" : "CIN");
    const QString nomCol = mwColumnExists("FOURNISSEURS", "RAISON_SOCIALE") ? "RAISON_SOCIALE" : "NOM";
    QSqlQuery q;
    if (!q.exec(QString("SELECT NVL(COUNT(*),0), NVL(SUM(NVL(%1,0)),0), "
                        "NVL(MAX(%1),0), NVL(MIN(%1),0) FROM FOURNISSEURS").arg(cmdCol))) {
        m_fournisseurStatsSummary->setText("Statistiques indisponibles.");
        return;
    }
    if (!q.next()) {
        m_fournisseurStatsSummary->setText("Aucune statistique.");
        return;
    }

    const int total = q.value(0).toInt();
    const int commandesTotal = q.value(1).toInt();
    const int maxCmd = q.value(2).toInt();
    const int minCmd = q.value(3).toInt();

    m_fournisseurStatsSummary->setText(
        QString("Fournisseurs: %1\nCommandes totales: %2").arg(total).arg(commandesTotal));

    if (!m_fournisseurStatsChartsContainer || !m_fournisseurStatsChartsContainer->isVisible()) return;

    QList<FournisseurStatsInput> rows;
    QSqlQuery qRows;
    if (!qRows.exec(QString("SELECT TO_CHAR(%1), TO_CHAR(%2), NVL(%3,0) FROM FOURNISSEURS ORDER BY NVL(%3,0) DESC")
                        .arg(codeCol, nomCol, cmdCol))) {
        return;
    }
    while (qRows.next()) {
        FournisseurStatsInput r;
        r.id = qRows.value(0).toString();
        r.nom = qRows.value(1).toString();
        r.commandes = qRows.value(2).toInt();
        rows.append(r);
    }
    const FournisseurStatsSnapshot snap = FournisseurStatsCalculator::compute(rows);

    QVector<QPair<QString, int>> bars;
    int n = 0;
    for (const auto &r : snap.ranked) {
        if (n++ >= 8) break;
        QString shortNom = r.nom;
        if (shortNom.size() > 12) shortNom = shortNom.left(11) + QChar(0x2026);
        bars.append(qMakePair(shortNom, r.commandes));
    }
    if (bars.isEmpty()) bars.append(qMakePair(QString("-"), 0));
    m_fournisseurBarChart->setChartTitle("Commandes par fournisseur");
    m_fournisseurBarChart->setBars(bars);

    QVector<QPair<QString, int>> slices;
    for (const auto &r : snap.ranked) {
        if (r.commandes > 0) slices.append(qMakePair(r.nom, r.commandes));
        if (slices.size() >= 6) break;
    }
    if (slices.isEmpty()) slices.append(qMakePair(QString("-"), 0));
    m_fournisseurPieChart->setChartTitle("Repartition des commandes");
    m_fournisseurPieChart->setSlices(slices);
}

// ------------------- Connexion BD -------------------

bool MainWindow::connectToDatabase()
{
    // Etapes de la capture:
    // 1) Instancier QSqlDatabase via QODBC
    // 2) Renseigner le nom de source de données, l'utilisateur, le mot de passe
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("projet_cuir");
    db.setUserName("dorra");
    db.setPassword("2012");

    if (db.open()) {
        qDebug() << "Connexion Oracle via ODBC réussie";
        return true;
    }

    const QString openError = db.lastError().text();
    const QString availableDrivers = QSqlDatabase::drivers().join(", ");
    qDebug() << "Erreur connexion ODBC :" << openError;
    qDebug() << "Drivers Qt disponibles :" << availableDrivers;

    QMessageBox::critical(
        this,
        "Connexion base de données",
        "Connexion Oracle impossible via ODBC.\n\n"
        "Vérifiez que la source de données ODBC 64 bits existe bien,\n"
        "et que le DSN/utilisateur/mot de passe sont corrects.\n\n"
        "DSN utilise: projet_cuir\n"
        "Erreur: " + openError + "\n"
        "Drivers Qt: " + availableDrivers
    );
    return false;

}
// ------------------- Destructeur -------------------

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------- Sidebar -------------------

void MainWindow::connectSidebar()
{
    connect(ui->btnAccueil, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(0);
        setActiveButton(ui->btnAccueil);
    });

    connect(ui->btnEmployes, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(1);
        setActiveButton(ui->btnEmployes);
    });

    connect(ui->btnclients, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(2);
        setActiveButton(ui->btnclients);
    });

    connect(ui->btnFournisseurs, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(3);
        setActiveButton(ui->btnFournisseurs);
    });

    connect(ui->btnMpremieres, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(5);
        setActiveButton(ui->btnMpremieres);
    });

    connect(ui->btnProduits, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(4);
        setActiveButton(ui->btnProduits);
        if (db.isOpen())
            refreshProduitsTable();
    });
}

void MainWindow::setActiveButton(QPushButton *active)
{
    QString defaultStyle = R"(
        background: transparent;
        color: rgb(248, 241, 227);
        text-align: left;
        padding: 12px 16px;
        border: none;
        border-left: 4px solid transparent;
        font-size: 13px;
    )";

    QString activeStyle = R"(
        background-color: rgb(255, 205, 185);
        color: rgb(88, 41, 0);
        text-align: left;
        padding: 12px 16px;
        border: none;
        border-left: 4px solid rgb(255, 205, 185);
        font-size: 13px;
        font-weight: bold;
    )";

    QList<QPushButton*> allBtns = {
        ui->btnAccueil, ui->btnEmployes, ui->btnFournisseurs,
        ui->btnProduits, ui->btnclients, ui->btnMpremieres
    };

    for (QPushButton *btn : allBtns) {
        btn->setStyleSheet(btn == active ? activeStyle : defaultStyle);
    }
}

// ------------------- CRUD Client -------------------

void MainWindow::setupClientValidators()
{
    ui->lineEdit_IDC->setValidator(new QIntValidator(1, 999999999, this));
    ui->lineEdit_remiseC->setValidator(new QDoubleValidator(0.0, 100.0, 2, this));
    ui->lineEdit_telephoneC->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\+?[0-9 ]{0,15}$"), this));
    ui->lineEdit_emailC->setValidator(new QRegularExpressionValidator(QRegularExpression("^[A-Za-z0-9._%+-]*@[A-Za-z0-9.-]*\\.?[A-Za-z]{0,}$"), this));
    ui->lineEdit_totalAchatsSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->lineEdit_freqAchatSeg->setValidator(new QIntValidator(0, 9999, this));
    ui->lineEdit_retardsSeg->setValidator(new QIntValidator(0, 9999, this));
    ui->lineEdit_limiteCreditSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->lineEdit_montantCommandeSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->lineEdit_montantPaiementSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
}

void MainWindow::setupClientUiEnhancements()
{
    QWidget *clientToolsRow = new QWidget(ui->employeeFormBox_3);
    QHBoxLayout *hTools = new QHBoxLayout(clientToolsRow);
    hTools->setContentsMargins(0, 6, 0, 0);
    hTools->setSpacing(8);

    const QString btnClientTools = QStringLiteral(
        "QPushButton {"
        "  background-color: rgb(88, 41, 0);"
        "  color: rgb(248, 241, 227);"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover { background-color: rgb(110, 55, 10); }");

    const auto prepareToolButton = [&](QPushButton *b) {
        b->setParent(clientToolsRow);
        b->setMinimumHeight(32);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(btnClientTools);
        b->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        hTools->addWidget(b);
    };

    QPushButton *exportButtonClient = new QPushButton(QStringLiteral("Export"), clientToolsRow);
    exportButtonClient->setToolTip(
        QStringLiteral("Exporter tout le tableau des clients vers un fichier Excel (.xls)."));
    prepareToolButton(exportButtonClient);
    connect(exportButtonClient, &QPushButton::clicked, this, &MainWindow::onExporterClientsClicked);

    m_demoOrderButton = new QPushButton(QStringLiteral("Cmd demo"), clientToolsRow);
    m_demoOrderButton->setToolTip(
        QStringLiteral("Creer une commande de demonstration avec le premier produit recommande."));
    prepareToolButton(m_demoOrderButton);
    connect(m_demoOrderButton, &QPushButton::clicked, this, &MainWindow::onDemoTopProductOrderClicked);

    hTools->addStretch(1);
    ui->formOuterLayout_3->addWidget(clientToolsRow);

    connect(ui->pushButton_recalculerSeg, &QPushButton::clicked, this, &MainWindow::onRecalculerSegmentationClicked);
    connect(ui->pushButton_verifierCommandeSeg, &QPushButton::clicked, this, &MainWindow::onVerifierCommandeClicked);
    connect(ui->pushButton_paiementSeg, &QPushButton::clicked, this, &MainWindow::onEnregistrerPaiementClicked);
    connect(ui->pushButton_historiqueSeg, &QPushButton::clicked, this, &MainWindow::onVoirHistoriqueClicked);
    connect(ui->btnRechercher_3, &QPushButton::clicked, this, &MainWindow::on_btnRechercher_3_clicked);
    connect(ui->lineEditSearch_3, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_3_clicked);
    connect(ui->comboBoxFiltreCategorie, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });
    connect(ui->comboBoxFiltreStatut, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });
    connect(ui->comboBoxFiltreRisque, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });

    setupClientIntelligencePanel();
}

void MainWindow::loadClients()
{
    QList<ClientData> clients;
    QString err;
    if (!Client::chargerTous(clients, &err)) {
        QMessageBox::warning(this, "Chargement clients", "Impossible de charger les clients:\n" + err);
        return;
    }
    ui->clientTable->setRowCount(0);
    for (const ClientData &c : clients) {
        const int row = ui->clientTable->rowCount();
        ui->clientTable->insertRow(row);
        ui->clientTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));
        ui->clientTable->setItem(row, 1, new QTableWidgetItem(c.nom));
        ui->clientTable->setItem(row, 2, new QTableWidgetItem(c.prenom));
        ui->clientTable->setItem(row, 3, new QTableWidgetItem(c.email));
        ui->clientTable->setItem(row, 4, new QTableWidgetItem(c.telephone));
        ui->clientTable->setItem(row, 5, new QTableWidgetItem(c.adresse));
        ui->clientTable->setItem(row, 6, new QTableWidgetItem(c.statutClient));
        ui->clientTable->setItem(row, 7, new QTableWidgetItem(c.categorie));
        ui->clientTable->setItem(row, 8, new QTableWidgetItem(QString::number(c.remiseAccordee, 'f', 2)));
        ui->clientTable->setItem(row, 9, new QTableWidgetItem(QString::number(c.totalAchats, 'f', 2)));
        ui->clientTable->setItem(row, 10, new QTableWidgetItem(QString::number(c.frequenceAchat)));
        ui->clientTable->setItem(row, 11, new QTableWidgetItem(QString::number(c.retardsPaiement)));
        ui->clientTable->setItem(row, 12, new QTableWidgetItem(QString::number(c.limiteCredit, 'f', 2)));
        ui->clientTable->setItem(row, 13, new QTableWidgetItem(QString::number(c.soldeCreditUtilise, 'f', 2)));
    }
}

void MainWindow::fillClientFormFromSelectedRow()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) return;
    ui->lineEdit_IDC->setText(ui->clientTable->item(row, 0)->text());
    ui->lineEdit_nomC->setText(ui->clientTable->item(row, 1)->text());
    ui->lineEdit_PrenomC->setText(ui->clientTable->item(row, 2)->text());
    ui->lineEdit_emailC->setText(ui->clientTable->item(row, 3)->text());
    ui->lineEdit_telephoneC->setText(ui->clientTable->item(row, 4)->text());
    ui->lineEdit_adresseC->setText(ui->clientTable->item(row, 5)->text());
    ui->comboBox_statutC->setCurrentText(ui->clientTable->item(row, 6)->text());
    ui->lineEdit_remiseC->setText(ui->clientTable->item(row, 8)->text());
    ui->lineEdit_totalAchatsSeg->setText(ui->clientTable->item(row, 9)->text());
    ui->lineEdit_freqAchatSeg->setText(ui->clientTable->item(row, 10)->text());
    ui->lineEdit_retardsSeg->setText(ui->clientTable->item(row, 11)->text());
    ui->lineEdit_limiteCreditSeg->setText(ui->clientTable->item(row, 12)->text());
    ClientData c;
    c.id = ui->clientTable->item(row, 0)->text().toInt();
    c.totalAchats = ui->clientTable->item(row, 9)->text().toDouble();
    c.frequenceAchat = ui->clientTable->item(row, 10)->text().toInt();
    c.retardsPaiement = ui->clientTable->item(row, 11)->text().toInt();
    c.limiteCredit = ui->clientTable->item(row, 12)->text().toDouble();
    c.soldeCreditUtilise = ui->clientTable->item(row, 13)->text().toDouble();
    updateAiInsightsPanel(c);
    refreshClientRecommendations();
}

void MainWindow::clearClientForm()
{
    ui->lineEdit_IDC->clear();
    ui->lineEdit_nomC->clear();
    ui->lineEdit_PrenomC->clear();
    ui->lineEdit_emailC->clear();
    ui->lineEdit_telephoneC->clear();
    ui->lineEdit_adresseC->clear();
    ui->lineEdit_remiseC->clear();
    ui->lineEdit_canalC->clear();
    ui->lineEdit_modeC->clear();
}

bool MainWindow::validateClientFormInputs(bool isUpdate)
{
    const QString idText = ui->lineEdit_IDC->text().trimmed();
    const QString nom = ui->lineEdit_nomC->text().trimmed();
    const QString prenom = ui->lineEdit_PrenomC->text().trimmed();
    const QString email = ui->lineEdit_emailC->text().trimmed();
    const QString telephone = ui->lineEdit_telephoneC->text().trimmed();
    const QString adresse = ui->lineEdit_adresseC->text().trimmed();

    if (idText.isEmpty() || idText.toInt() <= 0) {
        QMessageBox::warning(this, "Erreur de saisie", "ID client obligatoire et doit être supérieur à 0.");
        ui->lineEdit_IDC->setFocus();
        return false;
    }
    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Le nom est obligatoire.");
        ui->lineEdit_nomC->setFocus();
        return false;
    }
    if (prenom.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Le prénom est obligatoire.");
        ui->lineEdit_PrenomC->setFocus();
        return false;
    }
    if (email.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "L'email est obligatoire.");
        ui->lineEdit_emailC->setFocus();
        return false;
    }
    const QRegularExpression emailRegex("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$", QRegularExpression::CaseInsensitiveOption);
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Email invalide. Exemple attendu: nom@domaine.com");
        ui->lineEdit_emailC->setFocus();
        return false;
    }
    const QRegularExpression phoneRegex("^\\+?[0-9 ]{8,15}$");
    if (!phoneRegex.match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Téléphone invalide (8 à 15 chiffres, + et espaces autorisés).");
        ui->lineEdit_telephoneC->setFocus();
        return false;
    }
    if (adresse.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "L'adresse est obligatoire.");
        ui->lineEdit_adresseC->setFocus();
        return false;
    }

    if (ui->lineEdit_totalAchatsSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Total achats obligatoire.");
        ui->lineEdit_totalAchatsSeg->setFocus();
        return false;
    }
    if (ui->lineEdit_freqAchatSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Fréquence d'achat obligatoire.");
        ui->lineEdit_freqAchatSeg->setFocus();
        return false;
    }
    if (ui->lineEdit_retardsSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Nombre de retards obligatoire.");
        ui->lineEdit_retardsSeg->setFocus();
        return false;
    }
    if (ui->lineEdit_limiteCreditSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Limite de crédit obligatoire.");
        ui->lineEdit_limiteCreditSeg->setFocus();
        return false;
    }

    if (isUpdate && ui->clientTable->currentRow() < 0) {
        QMessageBox::warning(this, "Erreur de saisie", "Sélectionnez un client à modifier.");
        return false;
    }
    return true;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    if (!validateClientFormInputs(false)) {
        return;
    }

    ClientData c;
    c.id = ui->lineEdit_IDC->text().toInt();
    c.nom = ui->lineEdit_nomC->text();
    c.prenom = ui->lineEdit_PrenomC->text();
    c.email = ui->lineEdit_emailC->text();
    c.telephone = ui->lineEdit_telephoneC->text();
    c.adresse = ui->lineEdit_adresseC->text();
    c.statutClient = ui->comboBox_statutC->currentText();
    c.canalAcquisition = ui->lineEdit_canalC->text();
    c.modePaiementPrefere = ui->lineEdit_modeC->text();
    c.totalAchats = ui->lineEdit_totalAchatsSeg->text().toDouble();
    c.frequenceAchat = ui->lineEdit_freqAchatSeg->text().toInt();
    c.retardsPaiement = ui->lineEdit_retardsSeg->text().toInt();
    c.limiteCredit = ui->lineEdit_limiteCreditSeg->text().toDouble();

    QString err;
    if (!Client::ajouter(c, &err)) {
        QMessageBox::critical(this, "Ajout client", "Echec d'ajout:\n" + err);
        return;
    }
    loadClients();
    clearClientForm();
    refreshClientRecommendations();
    QMessageBox::information(this, "OK", "Client ajouté avec succès.");
}

void MainWindow::on_pushButton_supprimer_clicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez une ligne à supprimer.");
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    QString err;
    if (!Client::supprimer(id, &err)) {
        QMessageBox::critical(this, "Suppression client", "Echec suppression:\n" + err);
        return;
    }
    loadClients();
    clearClientForm();
    refreshClientRecommendations();
    QMessageBox::information(this, "OK", "Client supprimé.");
}

void MainWindow::on_pushButton_modifier_clicked()
{
    if (!validateClientFormInputs(true)) {
        return;
    }

    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez une ligne à modifier.");
        return;
    }

    ClientData c;
    c.id = ui->lineEdit_IDC->text().toInt();
    c.nom = ui->lineEdit_nomC->text();
    c.prenom = ui->lineEdit_PrenomC->text();
    c.email = ui->lineEdit_emailC->text();
    c.telephone = ui->lineEdit_telephoneC->text();
    c.adresse = ui->lineEdit_adresseC->text();
    c.statutClient = ui->comboBox_statutC->currentText();
    c.canalAcquisition = ui->lineEdit_canalC->text();
    c.modePaiementPrefere = ui->lineEdit_modeC->text();
    c.totalAchats = ui->lineEdit_totalAchatsSeg->text().toDouble();
    c.frequenceAchat = ui->lineEdit_freqAchatSeg->text().toInt();
    c.retardsPaiement = ui->lineEdit_retardsSeg->text().toInt();
    c.limiteCredit = ui->lineEdit_limiteCreditSeg->text().toDouble();
    c.soldeCreditUtilise = ui->clientTable->item(row, 13)->text().toDouble();

    QString err;
    if (!Client::modifier(c, &err)) {
        QMessageBox::critical(this, "Modification client", "Echec modification:\n" + err);
        return;
    }
    loadClients();
    refreshClientRecommendations();
    QMessageBox::information(this, "OK", "Client modifié.");
}

void MainWindow::on_btnRechercher_3_clicked()
{
    const QString motCle = ui->lineEditSearch_3->text().trimmed();
    const QString categorie = ui->comboBoxFiltreCategorie->currentText();
    const QString statut = ui->comboBoxFiltreStatut->currentText();
    const QString risque = ui->comboBoxFiltreRisque->currentText();

    QList<ClientData> clients;
    QString err;
    if (!Client::rechercherEtFiltrer(motCle, categorie, statut, risque, clients, &err)) {
        QMessageBox::warning(this, "Recherche", err);
        return;
    }
    ui->clientTable->setRowCount(0);
    for (const ClientData &c : clients) {
        const int row = ui->clientTable->rowCount();
        ui->clientTable->insertRow(row);
        ui->clientTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));
        ui->clientTable->setItem(row, 1, new QTableWidgetItem(c.nom));
        ui->clientTable->setItem(row, 2, new QTableWidgetItem(c.prenom));
        ui->clientTable->setItem(row, 3, new QTableWidgetItem(c.email));
        ui->clientTable->setItem(row, 4, new QTableWidgetItem(c.telephone));
        ui->clientTable->setItem(row, 5, new QTableWidgetItem(c.adresse));
        ui->clientTable->setItem(row, 6, new QTableWidgetItem(c.statutClient));
        ui->clientTable->setItem(row, 7, new QTableWidgetItem(c.categorie));
        ui->clientTable->setItem(row, 8, new QTableWidgetItem(QString::number(c.remiseAccordee, 'f', 2)));
        ui->clientTable->setItem(row, 9, new QTableWidgetItem(QString::number(c.totalAchats, 'f', 2)));
        ui->clientTable->setItem(row, 10, new QTableWidgetItem(QString::number(c.frequenceAchat)));
        ui->clientTable->setItem(row, 11, new QTableWidgetItem(QString::number(c.retardsPaiement)));
        ui->clientTable->setItem(row, 12, new QTableWidgetItem(QString::number(c.limiteCredit, 'f', 2)));
        ui->clientTable->setItem(row, 13, new QTableWidgetItem(QString::number(c.soldeCreditUtilise, 'f', 2)));
    }
    refreshClientRecommendations();
}

void MainWindow::on_pushButton_resetFiltres_clicked()
{
    ui->lineEditSearch_3->clear();
    ui->comboBoxFiltreCategorie->setCurrentText("Toutes");
    ui->comboBoxFiltreStatut->setCurrentText("Tous");
    ui->comboBoxFiltreRisque->setCurrentText("Tous");
    loadClients();
    refreshClientRecommendations();
}

void MainWindow::on_clientTable_cellClicked(int, int)
{
    fillClientFormFromSelectedRow();
}

void MainWindow::onRecalculerSegmentationClicked()
{
    if (ui->lineEdit_totalAchatsSeg->text().trimmed().isEmpty() ||
        ui->lineEdit_freqAchatSeg->text().trimmed().isEmpty() ||
        ui->lineEdit_retardsSeg->text().trimmed().isEmpty() ||
        ui->lineEdit_limiteCreditSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Renseignez Total achats, Fréquence, Retards et Limite crédit avant recalcul.");
        return;
    }

    ClientData c;
    c.totalAchats = ui->lineEdit_totalAchatsSeg->text().toDouble();
    c.frequenceAchat = ui->lineEdit_freqAchatSeg->text().toInt();
    c.retardsPaiement = ui->lineEdit_retardsSeg->text().toInt();
    c.limiteCredit = ui->lineEdit_limiteCreditSeg->text().toDouble();
    const int row = ui->clientTable->currentRow();
    if (row >= 0) c.soldeCreditUtilise = ui->clientTable->item(row, 13)->text().toDouble();
    Client::recalculerScoresEtCategorie(c);
    ui->labelScoreSeg->setText("Score IA: " + QString::number(c.scoreClient));
    ui->labelCategorieSeg->setText("Categorie IA: " + c.categorie + " (remise " + QString::number(c.remiseAccordee, 'f', 1) + "%)");
    ui->labelRisqueSeg->setText("Score risque IA: " + QString::number(c.scoreRisque));
    updateAiInsightsPanel(c);
    refreshClientRecommendations();
}

void MainWindow::onVerifierCommandeClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Credit", "Sélectionnez un client.");
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    if (ui->lineEdit_montantCommandeSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Saisissez le montant de la commande.");
        ui->lineEdit_montantCommandeSeg->setFocus();
        return;
    }
    const double montant = ui->lineEdit_montantCommandeSeg->text().toDouble();
    if (montant <= 0.0) {
        QMessageBox::warning(this, "Erreur de saisie", "Le montant de la commande doit être supérieur à 0.");
        ui->lineEdit_montantCommandeSeg->setFocus();
        return;
    }
    QString err;
    CreditCheckResult r = Client::verifierBlocageCommande(id, montant, &err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Credit", err);
        return;
    }
    ui->labelCreditRestantSeg->setText("Credit restant: " + QString::number(r.restant, 'f', 2));
    if (r.allowed) QMessageBox::information(this, "Commande", r.message + "\nDecision basée sur score IA et crédit réel.");
    else QMessageBox::warning(this, "Commande", r.message + "\nDecision basée sur score IA et crédit réel.");
}

void MainWindow::onEnregistrerPaiementClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Paiement", "Sélectionnez un client.");
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    if (ui->lineEdit_montantPaiementSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Saisissez le montant du paiement.");
        ui->lineEdit_montantPaiementSeg->setFocus();
        return;
    }
    const double montant = ui->lineEdit_montantPaiementSeg->text().toDouble();
    if (montant <= 0.0) {
        QMessageBox::warning(this, "Erreur de saisie", "Le montant du paiement doit être supérieur à 0.");
        ui->lineEdit_montantPaiementSeg->setFocus();
        return;
    }
    QString err;
    if (!Client::enregistrerPaiement(id, montant, ui->lineEdit_notePaiementSeg->text(), &err)) {
        QMessageBox::critical(this, "Paiement", err);
        return;
    }
    loadClients();
    refreshClientRecommendations();
    QMessageBox::information(this, "Paiement", "Paiement enregistré.");
}

void MainWindow::onVoirHistoriqueClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Historique", "Sélectionnez un client.");
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    QList<QPair<QString, QString>> rows;
    QString err;
    if (!Client::historiquePaiements(id, rows, &err)) {
        QMessageBox::warning(this, "Historique", err);
        return;
    }
    QString txt;
    for (const auto &r : rows) {
        txt += r.first + " | " + r.second + "\n";
    }
    if (txt.isEmpty()) txt = "Aucun paiement enregistré.";
    QMessageBox::information(this, "Historique paiements", txt);
}

void MainWindow::onExporterClientsClicked()
{
    if (ui->clientTable->rowCount() <= 0) {
        QMessageBox::information(this, "Export", "Aucun client.");
        return;
    }

    const QString defaultName = "clients_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".xls";
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter les clients",
        defaultName,
        "Fichiers Excel (*.xls)"
    );

    if (filePath.trimmed().isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export", "Erreur fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Entetes Excel (TSV compatible .xls)
    QStringList headers;
    headers.reserve(ui->clientTable->columnCount());
    for (int col = 0; col < ui->clientTable->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->clientTable->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : QString("COL_%1").arg(col));
    }
    out << headers.join('\t') << "\n";

    // Donnees Excel (TSV compatible .xls)
    for (int row = 0; row < ui->clientTable->rowCount(); ++row) {
        QStringList values;
        values.reserve(ui->clientTable->columnCount());
        for (int col = 0; col < ui->clientTable->columnCount(); ++col) {
            QTableWidgetItem *item = ui->clientTable->item(row, col);
            QString v = item ? item->text() : QString();
            v.replace('\t', ' ');
            v.replace('\n', ' ');
            values << v;
        }
        out << values.join('\t') << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export", "Export reussi.");
}

void MainWindow::updateAiInsightsPanel(const ClientData &client)
{
    ui->textEditInsightsSeg->setPlainText(Client::genererExplicationIA(client));
}

void MainWindow::setupClientIntelligencePanel()
{
    if (!ui->chatLayout_3 || !ui->textEditInsightsSeg || !ui->chatGroupBox_3) {
        return;
    }

    int idxInsights = -1;
    for (int i = 0; i < ui->chatLayout_3->count(); ++i) {
        QLayoutItem *lit = ui->chatLayout_3->itemAt(i);
        if (lit && lit->widget() == ui->textEditInsightsSeg) {
            idxInsights = i;
            break;
        }
    }
    if (idxInsights < 0) {
        return;
    }

    auto *lblReco = new QLabel("Produits recommandes (IA)", ui->chatGroupBox_3);
    lblReco->setStyleSheet("font-weight: bold; color: rgb(88, 41, 0);");
    m_clientRecoDisplay = new QTextEdit(ui->chatGroupBox_3);
    m_clientRecoDisplay->setReadOnly(true);
    m_clientRecoDisplay->setMinimumHeight(90);
    m_clientRecoDisplay->setMaximumHeight(200);
    m_clientRecoDisplay->setPlaceholderText("Selectionnez un client pour voir les suggestions.");
    ui->chatLayout_3->insertWidget(idxInsights, lblReco);
    ui->chatLayout_3->insertWidget(idxInsights + 1, m_clientRecoDisplay);

    idxInsights = -1;
    for (int i = 0; i < ui->chatLayout_3->count(); ++i) {
        QLayoutItem *lit = ui->chatLayout_3->itemAt(i);
        if (lit && lit->widget() == ui->textEditInsightsSeg) {
            idxInsights = i;
            break;
        }
    }
    if (idxInsights < 0) {
        return;
    }

    auto *lblChat = new QLabel("Assistant IA (chatbot)", ui->chatGroupBox_3);
    lblChat->setStyleSheet("font-weight: bold; color: rgb(88, 41, 0);");
    m_clientChatLog = new QTextEdit(ui->chatGroupBox_3);
    m_clientChatLog->setReadOnly(true);
    m_clientChatLog->setMinimumHeight(110);
    m_clientChatLog->setPlaceholderText("Conversation...");
    m_clientChatInput = new QLineEdit(ui->chatGroupBox_3);
    m_clientChatInput->setPlaceholderText("Prix, stock, suivi commande...");
    m_clientChatSendBtn = new QPushButton("Envoyer", ui->chatGroupBox_3);
    m_clientChatSendBtn->setMinimumHeight(32);
    m_clientChatSendBtn->setCursor(Qt::PointingHandCursor);
    m_clientChatSendBtn->setStyleSheet(
        "QPushButton { background-color: rgb(88, 41, 0); color: rgb(248, 241, 227); border: none; "
        "border-radius: 4px; font-size: 12px; font-weight: bold; padding: 6px 12px; }"
        "QPushButton:hover { background-color: rgb(110, 55, 10); }");

    ui->chatLayout_3->insertWidget(idxInsights + 1, lblChat);
    ui->chatLayout_3->insertWidget(idxInsights + 2, m_clientChatLog);
    auto *chatRow = new QHBoxLayout();
    chatRow->setSpacing(8);
    chatRow->addWidget(m_clientChatInput, 1);
    chatRow->addWidget(m_clientChatSendBtn, 0);
    ui->chatLayout_3->insertLayout(idxInsights + 3, chatRow);

    m_chatbotService = new ChatbotService(m_networkAccessManager, this);
    connect(m_chatbotService, &ChatbotService::replyReady, this, [this](const QString &t) {
        if (m_clientChatLog) {
            m_clientChatLog->append("Assistant: " + t);
        }
    });

    connect(m_clientChatSendBtn, &QPushButton::clicked, this, &MainWindow::onClientChatSendRequested);
    connect(m_clientChatInput, &QLineEdit::returnPressed, this, &MainWindow::onClientChatSendRequested);
}

void MainWindow::refreshClientRecommendations()
{
    if (!m_clientRecoDisplay) {
        return;
    }
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        m_lastRecommendations.clear();
        m_clientRecoDisplay->setPlainText("Selectionnez un client dans le tableau.");
        return;
    }

    ClientData c;
    c.id = ui->clientTable->item(row, 0)->text().toInt();
    c.nom = ui->clientTable->item(row, 1)->text();
    c.prenom = ui->clientTable->item(row, 2)->text();
    c.email = ui->clientTable->item(row, 3)->text();
    c.telephone = ui->clientTable->item(row, 4)->text();
    c.adresse = ui->clientTable->item(row, 5)->text();
    c.statutClient = ui->clientTable->item(row, 6)->text();
    c.categorie = ui->clientTable->item(row, 7)->text();
    c.remiseAccordee = ui->clientTable->item(row, 8)->text().toDouble();
    c.totalAchats = ui->clientTable->item(row, 9)->text().toDouble();
    c.frequenceAchat = ui->clientTable->item(row, 10)->text().toInt();
    c.retardsPaiement = ui->clientTable->item(row, 11)->text().toInt();
    c.limiteCredit = ui->clientTable->item(row, 12)->text().toDouble();
    c.soldeCreditUtilise = ui->clientTable->item(row, 13)->text().toDouble();

    ClientData dbClient;
    if (Client::chargerParId(c.id, dbClient, nullptr)) {
        c.scoreClient = dbClient.scoreClient;
        c.scoreRisque = dbClient.scoreRisque;
        c.canalAcquisition = dbClient.canalAcquisition;
        c.modePaiementPrefere = dbClient.modePaiementPrefere;
    }

    QString err;
    if (!RecommendationService::recommendForClient(c, 5, m_lastRecommendations, &err)) {
        m_clientRecoDisplay->setPlainText("Recommandations indisponibles:\n" + err);
        m_lastRecommendations.clear();
        return;
    }
    m_clientRecoDisplay->setPlainText(RecommendationService::formatAsText(m_lastRecommendations));
}

void MainWindow::onClientChatSendRequested()
{
    if (!m_chatbotService || !m_clientChatInput || !m_clientChatLog) {
        return;
    }
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Chatbot", "Selectionnez un client.");
        return;
    }
    const QString msg = m_clientChatInput->text().trimmed();
    if (msg.isEmpty()) {
        return;
    }
    m_clientChatInput->clear();
    m_clientChatLog->append("Vous: " + msg);

    const int cid = ui->clientTable->item(row, 0)->text().toInt();
    const QString nom = ui->clientTable->item(row, 1)->text() + " " + ui->clientTable->item(row, 2)->text();
    const QString email = ui->clientTable->item(row, 3)->text();
    QString cerr;
    const QString ctx = CommerceStore::buildChatbotContext(cid, nom.trimmed(), email, &cerr);

    m_chatbotService->ask(msg, ctx);
}

void MainWindow::onDemoTopProductOrderClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Commande", "Selectionnez un client.");
        return;
    }
    if (m_lastRecommendations.isEmpty()) {
        refreshClientRecommendations();
    }
    if (m_lastRecommendations.isEmpty()) {
        QMessageBox::information(this, "Commande", "Aucun produit recommande (stock ou donnees).");
        return;
    }
    const int clientId = ui->clientTable->item(row, 0)->text().toInt();
    const int pid = m_lastRecommendations.first().productId;

    QString err;
    int orderId = 0;
    if (!CommerceStore::createSampleOrder(clientId, pid, 1, &orderId, &err)) {
        QMessageBox::critical(this, "Commande", err);
        return;
    }

    loadClients();
    refreshClientRecommendations();
    QMessageBox::information(this, "Commande", "Commande demo creee #" + QString::number(orderId));
}

// ------------------- Page Produits -------------------

void MainWindow::setupProduitPage()
{
    if (!ui->employeeTable_4)
        return;

    ui->employeeTable_4->setColumnCount(10);
    ui->employeeTable_4->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Nom produit"),
        QStringLiteral("Categorie"),
        QStringLiteral("Type cuir"),
        QStringLiteral("Qualite"),
        QStringLiteral("Qt stock"),
        QStringLiteral("Etat"),
        QStringLiteral("Date fab."),
        QStringLiteral("Type design"),
        QStringLiteral("Style (interne)"),
    });
    ui->employeeTable_4->horizontalHeader()->setStretchLastSection(true);
    ui->employeeTable_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->employeeTable_4->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->employeeTable_4->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->employeeTable_4->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (ui->comboBox_4 && ui->comboBox_4->findText(QStringLiteral("Inactif")) < 0)
        ui->comboBox_4->addItem(QStringLiteral("Inactif"));

    if (ui->lineEditAdresse_2)
        ui->lineEditAdresse_2->setValidator(new QIntValidator(0, 999999999, this));

    connect(ui->employeeTable_4, &QTableWidget::cellClicked, this, &MainWindow::on_produitTable_cellClicked);
    connect(ui->btnAjouter_6, &QPushButton::clicked, this, &MainWindow::on_btnAjouter_6_clicked);
    connect(ui->btnModifier_4, &QPushButton::clicked, this, &MainWindow::on_btnModifier_4_clicked);
    connect(ui->btnSupprimer_4, &QPushButton::clicked, this, &MainWindow::on_btnSupprimer_4_clicked);

    if (ui->pushButton_8 && ui->textEdit_3 && ui->lineEditSearch_4) {
        connect(ui->pushButton_8, &QPushButton::clicked, this, [this]() {
            const QString q = ui->lineEditSearch_4->text().trimmed();
            if (q.isEmpty()) {
                QMessageBox::information(this, QStringLiteral("Produits"),
                                         QStringLiteral("Saisissez un texte dans le champ au-dessus du bouton Envoyer."));
                return;
            }
            ui->textEdit_3->append(
                QStringLiteral("[%1] %2\n(Assistant API non integre.)")
                    .arg(QDateTime::currentDateTime().toString(QStringLiteral("dd/MM HH:mm")), q));
            ui->lineEditSearch_4->clear();
        });
    }

    refreshProduitsTable();
}

void MainWindow::refreshProduitsTable()
{
    if (!ui->employeeTable_4 || !db.isOpen())
        return;

    std::unique_ptr<QSqlQueryModel> model(Produit::afficher());
    if (!model)
        return;
    if (model->lastError().isValid()) {
        QMessageBox::warning(this, QStringLiteral("Produits"),
                             QStringLiteral("Impossible de charger les produits:\n%1").arg(model->lastError().text()));
        return;
    }

    ui->employeeTable_4->setSortingEnabled(false);
    ui->employeeTable_4->setRowCount(0);

    const int n = model->rowCount();
    for (int r = 0; r < n; ++r) {
        ui->employeeTable_4->insertRow(r);
        for (int c = 0; c < 10; ++c) {
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
            ui->employeeTable_4->setItem(r, c, new QTableWidgetItem(txt));
        }
    }
    ui->employeeTable_4->setSortingEnabled(true);
}

void MainWindow::clearProduitForm()
{
    m_produitSelectedId = -1;
    if (ui->lineEditCIN_4)
        ui->lineEditCIN_4->clear();
    if (ui->lineEditNom_4)
        ui->lineEditNom_4->clear();
    if (ui->lineEditPrenom_4)
        ui->lineEditPrenom_4->clear();
    if (ui->lineEditAdresse_2)
        ui->lineEditAdresse_2->clear();
    if (ui->lineEditPrenom_5)
        ui->lineEditPrenom_5->clear();
    if (ui->comboBox_3)
        ui->comboBox_3->setCurrentIndex(0);
    if (ui->comboBox_4)
        ui->comboBox_4->setCurrentIndex(0);
    if (ui->comboBox_5)
        ui->comboBox_5->setCurrentIndex(0);
    if (ui->dateTimeEdit)
        ui->dateTimeEdit->setDate(QDate::currentDate());
}

void MainWindow::fillProduitFormFromTableRow(int row)
{
    if (row < 0 || !ui->employeeTable_4 || !ui->employeeTable_4->item(row, 0))
        return;

    auto cell = [this, row](int col) -> QString {
        QTableWidgetItem *it = ui->employeeTable_4->item(row, col);
        return it ? it->text() : QString();
    };

    m_produitSelectedId = cell(0).toInt();
    if (ui->lineEditCIN_4)
        ui->lineEditCIN_4->setText(cell(1));
    if (ui->lineEditNom_4)
        ui->lineEditNom_4->setText(cell(2));
    if (ui->lineEditPrenom_4)
        ui->lineEditPrenom_4->setText(cell(3));

    const QString packedStyle = cell(9);
    QString pq, ptd, ps;
    QDate pdf;
    Produit::unpackPackedStyle(packedStyle, &pq, nullptr, &pdf, &ptd, &ps);

    const QString qualiteAff = cell(4).isEmpty() ? pq : cell(4);
    if (ui->comboBox_3 && !qualiteAff.isEmpty())
        ui->comboBox_3->setCurrentText(qualiteAff);

    bool qtyOk = false;
    const int qty = cell(5).toInt(&qtyOk);
    if (ui->lineEditAdresse_2)
        ui->lineEditAdresse_2->setText(qtyOk ? QString::number(qty) : cell(5));

    const QString etatAff = cell(6);
    if (ui->comboBox_4 && !etatAff.isEmpty()) {
        const int ix = ui->comboBox_4->findText(etatAff);
        if (ix >= 0)
            ui->comboBox_4->setCurrentIndex(ix);
        else
            ui->comboBox_4->setCurrentText(etatAff);
    }

    const QString tdAff = cell(8).isEmpty() ? ptd : cell(8);
    if (ui->comboBox_5 && !tdAff.isEmpty())
        ui->comboBox_5->setCurrentText(tdAff);

    QDate dateFab;
    if (!cell(7).isEmpty()) {
        dateFab = QDate::fromString(cell(7), Qt::ISODate);
        if (!dateFab.isValid())
            dateFab = QDate::fromString(cell(7), QStringLiteral("dd/MM/yyyy"));
    }
    if (!dateFab.isValid())
        dateFab = pdf;
    if (ui->dateTimeEdit) {
        if (dateFab.isValid())
            ui->dateTimeEdit->setDate(dateFab);
        else
            ui->dateTimeEdit->setDate(QDate::currentDate());
    }

    if (ui->lineEditPrenom_5)
        ui->lineEditPrenom_5->setText(ps.isEmpty() ? packedStyle : ps);
}

void MainWindow::on_produitTable_cellClicked(int row, int)
{
    fillProduitFormFromTableRow(row);
}

void MainWindow::on_btnAjouter_6_clicked()
{
    if (!db.isOpen()) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Base de donnees non connectee."));
        return;
    }

    bool qtyOk = false;
    const int qte = ui->lineEditAdresse_2 ? ui->lineEditAdresse_2->text().trimmed().toInt(&qtyOk) : 0;
    if (!qtyOk || qte < 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Quantite stock invalide (entier >= 0)."));
        return;
    }

    const int nid = Produit::nextAvailableId();
    const QString nom = ui->lineEditCIN_4 ? ui->lineEditCIN_4->text().trimmed() : QString();
    if (nom.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Nom produit obligatoire."));
        return;
    }

    Produit p(nid,
              nom,
              ui->lineEditNom_4 ? ui->lineEditNom_4->text().trimmed() : QString(),
              ui->lineEditPrenom_4 ? ui->lineEditPrenom_4->text().trimmed() : QString(),
              ui->comboBox_3 ? ui->comboBox_3->currentText() : QString(),
              qte,
              ui->comboBox_4 ? ui->comboBox_4->currentText() : QString(),
              ui->dateTimeEdit ? ui->dateTimeEdit->date() : QDate(),
              ui->comboBox_5 ? ui->comboBox_5->currentText() : QString(),
              ui->lineEditPrenom_5 ? ui->lineEditPrenom_5->text().trimmed() : QString());

    if (!p.ajouter()) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec ajout:\n%1").arg(Produit::lastSqlError));
        return;
    }
    refreshProduitsTable();
    clearProduitForm();
    QMessageBox::information(this, QStringLiteral("Produits"), QStringLiteral("Produit ajoute (ID %1).").arg(nid));
}

void MainWindow::on_btnModifier_4_clicked()
{
    if (!db.isOpen()) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Base de donnees non connectee."));
        return;
    }
    if (m_produitSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Selectionnez une ligne dans le tableau."));
        return;
    }

    bool qtyOk = false;
    const int qte = ui->lineEditAdresse_2 ? ui->lineEditAdresse_2->text().trimmed().toInt(&qtyOk) : 0;
    if (!qtyOk || qte < 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Quantite stock invalide (entier >= 0)."));
        return;
    }

    const QString nom = ui->lineEditCIN_4 ? ui->lineEditCIN_4->text().trimmed() : QString();
    if (nom.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Nom produit obligatoire."));
        return;
    }

    Produit p(m_produitSelectedId,
              nom,
              ui->lineEditNom_4 ? ui->lineEditNom_4->text().trimmed() : QString(),
              ui->lineEditPrenom_4 ? ui->lineEditPrenom_4->text().trimmed() : QString(),
              ui->comboBox_3 ? ui->comboBox_3->currentText() : QString(),
              qte,
              ui->comboBox_4 ? ui->comboBox_4->currentText() : QString(),
              ui->dateTimeEdit ? ui->dateTimeEdit->date() : QDate(),
              ui->comboBox_5 ? ui->comboBox_5->currentText() : QString(),
              ui->lineEditPrenom_5 ? ui->lineEditPrenom_5->text().trimmed() : QString());

    if (!p.modifier(m_produitSelectedId, m_produitSelectedId)) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec modification:\n%1").arg(Produit::lastSqlError));
        return;
    }
    refreshProduitsTable();
    QMessageBox::information(this, QStringLiteral("Produits"), QStringLiteral("Produit modifie."));
}

void MainWindow::on_btnSupprimer_4_clicked()
{
    if (!db.isOpen()) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Base de donnees non connectee."));
        return;
    }
    if (m_produitSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Selectionnez une ligne dans le tableau."));
        return;
    }

    const auto r = QMessageBox::question(this,
                                         QStringLiteral("Produits"),
                                         QStringLiteral("Supprimer le produit ID %1 ?").arg(m_produitSelectedId),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
    if (r != QMessageBox::Yes)
        return;

    if (!Produit().supprimer(m_produitSelectedId)) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec suppression:\n%1").arg(Produit::lastSqlError));
        return;
    }
    refreshProduitsTable();
    clearProduitForm();
    QMessageBox::information(this, QStringLiteral("Produits"), QStringLiteral("Produit supprime."));
}
