#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>
#include <QInputDialog>

// ============================================================
//  Constructeur
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    menuBar()->hide();
    statusBar()->hide();

    // Configuration du tableau
    ui->employeeTable->horizontalHeader()->setStretchLastSection(true);
    ui->employeeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->employeeTable->horizontalHeader()->setMinimumSectionSize(80);
    ui->employeeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connectSidebar();
    connecterBD();

    // Charger les donnees au demarrage
    chargerTableau(m_dao.afficherTous());
    rafraichirStats();

    viderFormulaire();

    // Connexions manuelles pour les comboBoxes de tri
    connect(ui->comboBoxTri,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_comboBoxTri_currentIndexChanged);
    // Page Matieres par defaut (index 1)
    ui->contentStack->setCurrentIndex(1);
    setActiveButton(ui->btnMatieres);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================
//  Connexion Oracle via ODBC
// ============================================================
void MainWindow::connecterBD()
{
    bool ok = MatiereDAO::connecterOracle(
        "ranim",
        "107108",
        "localhost",
        1521,
        "XE"
        );

    if (!ok) {
        QMessageBox::warning(this, "Connexion Oracle",
                             "Impossible de se connecter a Oracle.\n\n"
                             "Verifiez :\n"
                             "1. Oracle Database XE est demarree\n"
                             "2. Le DSN ODBC 'OracleXE' est configure (ODBC 64-bit)\n"
                             "3. Login: ranim / Mot de passe: 107108\n"
                             "4. SID: XE, Port: 1521\n\n"
                             "L'application continue en mode hors-connexion.");
    } else {
        statusBar()->showMessage("Oracle XE connecte", 3000);
    }
}

// ============================================================
//  Sidebar
// ============================================================
void MainWindow::connectSidebar()
{
    connect(ui->btnAccueil, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(0);
        setActiveButton(ui->btnAccueil);
    });

    connect(ui->btnMatieres, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(1);
        setActiveButton(ui->btnMatieres);
        chargerTableau(m_dao.afficherTous());
        rafraichirStats();
    });

    ui->btnFournisseurs->setEnabled(false);
    ui->btnProduits->setEnabled(false);
    ui->btnCommandes->setEnabled(false);
    ui->btnParametres->setEnabled(false);
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
        ui->btnMatieres, ui->btnProduits, ui->btnCommandes, ui->btnParametres
    };
    for (QPushButton *btn : allBtns)
        btn->setStyleSheet(btn == active ? activeStyle : defaultStyle);
}

// ============================================================
//  Tableau
// ============================================================
void MainWindow::chargerTableau(const QList<MatierePremiere> &liste)
{
    m_listeCourante = liste;
    ui->employeeTable->setRowCount(0);

    for (const MatierePremiere &m : liste) {
        int row = ui->employeeTable->rowCount();
        ui->employeeTable->insertRow(row);

        auto item = [](const QString &txt, bool critical = false) -> QTableWidgetItem* {
            auto *it = new QTableWidgetItem(txt);
            it->setTextAlignment(Qt::AlignCenter);
            if (critical) {
                it->setForeground(QColor(200, 50, 50));
                it->setFont(QFont("Segoe UI", 9, QFont::Bold));
            }
            return it;
        };

        bool critique = m.isSeuilCritique();

        ui->employeeTable->setItem(row, 0,  item(QString::number(m.getId())));
        ui->employeeTable->setItem(row, 1,  item(m.getReference()));
        ui->employeeTable->setItem(row, 2,  item(m.getNomCuir()));
        ui->employeeTable->setItem(row, 3,  item(m.getTypeCuir()));
        ui->employeeTable->setItem(row, 4,  item(m.getGamme()));
        ui->employeeTable->setItem(row, 5,  item(m.getCouleur()));
        ui->employeeTable->setItem(row, 6,  item(QString::number(m.getEpaisseur(), 'f', 2)));
        ui->employeeTable->setItem(row, 7,  item(m.getOrigine()));
        ui->employeeTable->setItem(row, 8,  item("-"));
        ui->employeeTable->setItem(row, 9,  item(QString::number(m.getReserve()), critique));
        ui->employeeTable->setItem(row, 10, item("-"));
        ui->employeeTable->setItem(row, 11, item("-"));
        ui->employeeTable->setItem(row, 12, item(m.getStatut()));
    }
}

// ============================================================
//  Formulaire
// ============================================================
void MainWindow::viderFormulaire()
{
    ui->lineEditCIN->clear();
    ui->lineEditNom->clear();
    ui->lineEditPrenom->clear();
    ui->comboBoxSexe->setCurrentIndex(0);
    ui->lineEditSalaire->clear();         // Type de cuir
    ui->dateEditEmbauche->setDate(QDate::currentDate());
    ui->lineEditTelephone->clear();       // Couleur
    ui->lineEditPoste->clear();           // Quantite stock
    ui->lineEditAdresse->clear();         // Epaisseur
    ui->lineEditEmail->clear();           // Origine
}

void MainWindow::remplirFormulaire(const MatierePremiere &m)
{
    ui->lineEditCIN->setText(QString::number(m.getId()));
    ui->lineEditNom->setText(m.getReference());
    ui->lineEditPrenom->setText(m.getNomCuir());

    // Gamme -> comboBox
    int idx = ui->comboBoxSexe->findText(m.getGamme(), Qt::MatchFixedString);
    ui->comboBoxSexe->setCurrentIndex(idx >= 0 ? idx : 0);

    ui->lineEditSalaire->setText(m.getTypeCuir());          // Type de cuir
    ui->lineEditTelephone->setText(m.getCouleur());         // Couleur
    ui->lineEditPoste->setText(QString::number(m.getReserve()));
    ui->lineEditAdresse->setText(QString::number(m.getEpaisseur(), 'f', 2));
    ui->lineEditEmail->setText(m.getOrigine());             // Origine
}

MatierePremiere MainWindow::lireFormulaire()
{
    MatierePremiere m;
    m.setId(ui->lineEditCIN->text().toInt());
    m.setReference(ui->lineEditNom->text().trimmed());
    m.setNomCuir(ui->lineEditPrenom->text().trimmed());
    m.setGamme(ui->comboBoxSexe->currentText());
    m.setTypeCuir(ui->lineEditSalaire->text().trimmed());   // Type de cuir
    m.setCouleur(ui->lineEditTelephone->text().trimmed());  // Couleur
    m.setReserve(ui->lineEditPoste->text().toInt());
    m.setEpaisseur(ui->lineEditAdresse->text().toDouble());
    m.setOrigine(ui->lineEditEmail->text().trimmed());      // Origine
    m.setStatut("Disponible");
    return m;
}

bool MainWindow::validerFormulaire()
{
    if (ui->lineEditPrenom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", "Le champ 'Nom du cuir' est obligatoire.");
        ui->lineEditPrenom->setFocus();
        return false;
    }
    if (ui->lineEditNom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", "La reference interne est obligatoire.");
        ui->lineEditNom->setFocus();
        return false;
    }
    if (ui->lineEditSalaire->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", "Le type de cuir est obligatoire.");
        ui->lineEditSalaire->setFocus();
        return false;
    }
    bool okReserve;
    int reserve = ui->lineEditPoste->text().toInt(&okReserve);
    if (!okReserve || reserve < 0) {
        QMessageBox::warning(this, "Saisie invalide",
                             "La quantite de stock doit etre un entier positif ou nul.");
        ui->lineEditPoste->setFocus();
        return false;
    }
    bool okEpaisseur;
    double ep = ui->lineEditAdresse->text().toDouble(&okEpaisseur);
    if (!okEpaisseur || ep < 0) {
        QMessageBox::warning(this, "Saisie invalide",
                             "L'epaisseur doit etre un nombre decimal positif (ex: 1.5).");
        ui->lineEditAdresse->setFocus();
        return false;
    }
    Q_UNUSED(reserve);
    Q_UNUSED(ep);
    return true;
}

void MainWindow::rafraichirStats()
{
    QList<MatierePremiere> tous = m_dao.afficherTous();
    int total    = tous.size();
    int critique = 0;
    int totalReserve = 0;
    QSet<QString> types;

    for (const MatierePremiere &m : tous) {
        if (m.isSeuilCritique()) critique++;
        types.insert(m.getTypeCuir());
        totalReserve += m.getReserve();
    }

    // statCard1 -> Quantite totale en stock
    ui->statTitle1->setText(QString("Quantite en stock : %1 unites").arg(totalReserve));
    // statCard2 -> Types differents
    ui->statTitle2->setText(QString("Types de cuir : %1 types").arg(types.size()));
    // statCard3 -> Seuil critique
    ui->statTitle3->setText(QString("Seuil critique (<=10): %1 articles").arg(critique));
}

// ============================================================
//  Slots CRUD
// ============================================================
void MainWindow::on_btnAjouter_clicked()
{
    if (!validerFormulaire()) return;

    MatierePremiere m = lireFormulaire();

    if (m.getId() == 0)
        m.setId(m_dao.prochainId());

    if (m_dao.ajouter(m)) {
        QMessageBox::information(this, "Succes", "Matiere premiere ajoutee avec succes.");
        chargerTableau(m_dao.afficherTous());
        viderFormulaire();
        rafraichirStats();
    } else {
        QMessageBox::critical(this, "Erreur",
                              "Impossible d'ajouter la matiere premiere.\n"
                              "Verifiez la connexion Oracle et que l'ID n'est pas deja utilise.");
    }
}

void MainWindow::on_btnModifier_clicked()
{
    if (ui->employeeTable->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Modification",
                             "Veuillez selectionner une matiere dans le tableau.");
        return;
    }
    if (!validerFormulaire()) return;

    MatierePremiere m = lireFormulaire();

    if (m_dao.modifier(m)) {
        QMessageBox::information(this, "Succes", "Matiere premiere modifiee avec succes.");
        chargerTableau(m_dao.afficherTous());
        viderFormulaire();
        rafraichirStats();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de modifier la matiere premiere.");
    }
}

void MainWindow::on_btnSupprimer_clicked()
{
    if (ui->employeeTable->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Suppression",
                             "Veuillez selectionner une matiere dans le tableau.");
        return;
    }

    int row = ui->employeeTable->currentRow();
    int id  = ui->employeeTable->item(row, 0)->text().toInt();
    QString nom = ui->employeeTable->item(row, 2)->text();

    int rep = QMessageBox::question(this, "Confirmation",
                                    QString("Supprimer la matiere '%1' (ID: %2) ?").arg(nom).arg(id),
                                    QMessageBox::Yes | QMessageBox::No);

    if (rep == QMessageBox::Yes) {
        if (m_dao.supprimer(id)) {
            QMessageBox::information(this, "Succes", "Matiere premiere supprimee.");
            chargerTableau(m_dao.afficherTous());
            viderFormulaire();
            rafraichirStats();
        } else {
            QMessageBox::critical(this, "Erreur",
                                  "Impossible de supprimer.\n"
                                  "La matiere est peut-etre liee a d'autres tables (FOURNIR/FABRIQUER).");
        }
    }
}

void MainWindow::on_btnRechercher_clicked()
{
    QString motCle = ui->lineEditSearch->text().trimmed();
    if (motCle.isEmpty()) {
        chargerTableau(m_dao.afficherTous());
        return;
    }
    QList<MatierePremiere> res = m_dao.rechercher(motCle);
    chargerTableau(res);
    if (res.isEmpty())
        QMessageBox::information(this, "Recherche",
                                 QString("Aucun resultat pour '%1'.").arg(motCle));
}

// ============================================================
//  Slots Tri
// ============================================================
void MainWindow::on_comboBoxTri_currentIndexChanged(int index)
{
    if (index <= 0) {
        chargerTableau(m_dao.afficherTous());
        return;
    }

    // Correspondance index -> colonne SQL
    QStringList champs = {"", "NOM_CUIR", "GAME", "EPAISSEUR", "RESERVE", "TYPE_CUIR"};
    if (index < champs.size() && !champs[index].isEmpty()) {
        QString ordre = (ui->comboBoxOrdre->currentIndex() == 1) ? "DESC" : "ASC";
        chargerTableau(m_dao.trierPar(champs[index], ordre));
    }
}

void MainWindow::on_comboBoxOrdre_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    // Reapplique le tri courant avec le nouvel ordre
    int triIdx = ui->comboBoxTri->currentIndex();
    if (triIdx > 0)
        on_comboBoxTri_currentIndexChanged(triIdx);
}

// ============================================================
//  Slots Filtres
// ============================================================
void MainWindow::on_btnFiltreDisponible_clicked()
{
    chargerTableau(m_dao.filtrerDisponibles());
    ui->lineEditSearch->clear();
}

void MainWindow::on_btnFiltreSeuilCritique_clicked()
{
    bool ok;
    int seuil = QInputDialog::getInt(this, "Seuil critique",
                                     "Afficher les matieres avec reserve <=",
                                     10, 0, 100000, 1, &ok);
    if (ok)
        chargerTableau(m_dao.filtrerSeuilCritique(seuil));
}

void MainWindow::on_btnAfficherTous_clicked()
{
    ui->lineEditSearch->clear();
    ui->comboBoxTri->setCurrentIndex(0);
    chargerTableau(m_dao.afficherTous());
}

// ============================================================
//  Slots Export
// ============================================================
void MainWindow::on_btnExportPDF_clicked()
{
    if (m_listeCourante.isEmpty()) {
        QMessageBox::information(this, "Export PDF", "Aucune donnee a exporter.");
        return;
    }
    QString fichier = QFileDialog::getSaveFileName(
        this, "Exporter en PDF", "stock_matieres.pdf",
        "Fichiers PDF (*.pdf)");
    if (fichier.isEmpty()) return;

    if (m_dao.exporterPDF(m_listeCourante, fichier))
        QMessageBox::information(this, "Export PDF",
                                 "Export PDF reussi !\nFichier: " + fichier);
    else
        QMessageBox::critical(this, "Export PDF", "Echec de l'export PDF.");
}

void MainWindow::on_btnExportExcel_clicked()
{
    if (m_listeCourante.isEmpty()) {
        QMessageBox::information(this, "Export Excel", "Aucune donnee a exporter.");
        return;
    }
    QString fichier = QFileDialog::getSaveFileName(
        this, "Exporter en Excel (CSV)", "stock_matieres.csv",
        "Fichiers CSV (*.csv)");
    if (fichier.isEmpty()) return;

    if (m_dao.exporterExcel(m_listeCourante, fichier))
        QMessageBox::information(this, "Export Excel",
                                 "Export CSV reussi !\nOuvrez avec Excel.\nFichier: " + fichier);
    else
        QMessageBox::critical(this, "Export Excel", "Echec de l'export.");
}

// ============================================================
//  Selection dans le tableau -> remplir formulaire
// ============================================================
void MainWindow::on_employeeTable_itemSelectionChanged()
{
    int row = ui->employeeTable->currentRow();
    if (row < 0 || row >= m_listeCourante.size()) return;
    remplirFormulaire(m_listeCourante[row]);
}
