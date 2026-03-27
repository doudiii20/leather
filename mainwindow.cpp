#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QDebug>
#include <QRegularExpression>
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Pas de validators bloquants: l'utilisateur peut saisir librement.
    // Le contrôle est fait au clic Ajouter/Modifier avec message d'erreur.
    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd");
    ui->dateTimeEdit->setMinimumDate(QDate(1900, 1, 1));

    validationLabel = new QLabel(ui->centralwidget);
    validationLabel->setObjectName("validationLabel");
    validationLabel->setStyleSheet(
        "QLabel#validationLabel {"
        "background-color: rgb(255, 235, 235);"
        "color: rgb(170, 0, 0);"
        "border: 1px solid rgb(220, 90, 90);"
        "border-radius: 4px;"
        "padding: 4px 8px;"
        "font-size: 11px;"
        "font-weight: bold;"
        "}"
        );
    validationLabel->setWordWrap(true);
    validationLabel->hide();

    afficherProduits();
    connect(ui->employeeTable_4, &QTableWidget::cellClicked, this, &MainWindow::remplirFormulaire);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::viderFormulaire()
{
    selectedProduitId = -1;
    cacherErreurSaisie();
    ui->lineEdit_7->clear();
    ui->lineEdit_9->clear();
    ui->lineEdit_10->clear();
    ui->lineEdit_11->clear();
    ui->lineEdit_12->clear();
    ui->lineEdit_17->clear();
    ui->comboBox_3->setCurrentIndex(0);
    ui->comboBox_4->setCurrentIndex(0);
    ui->comboBox_5->setCurrentIndex(0);
    ui->dateTimeEdit->setDate(QDate::currentDate());
}

void MainWindow::cacherErreurSaisie()
{
    if (validationLabel) validationLabel->hide();
}

void MainWindow::afficherErreurSaisie(QWidget *champ, const QString &message)
{
    if (!champ) return;
    if (validationLabel)
    {
        validationLabel->setText(message);
        validationLabel->adjustSize();
        QPoint p = champ->mapTo(ui->centralwidget, QPoint(0, 0));
        int y = p.y() - validationLabel->height() - 6;
        if (y < 0) y = p.y() + champ->height() + 6;
        int x = p.x();
        if (x + validationLabel->width() > ui->centralwidget->width())
            x = ui->centralwidget->width() - validationLabel->width() - 8;
        if (x < 0) x = 8;
        validationLabel->move(x, y);
        validationLabel->raise();
        validationLabel->show();
    }
    champ->setFocus();
}

bool MainWindow::validerFormulaire(int &outId, int &outQte, QString &outErrorMsg, QWidget *&outInvalidWidget)
{
    outErrorMsg.clear();
    outInvalidWidget = nullptr;

    bool idOk = false;
    const int id = ui->lineEdit_7->text().trimmed().toInt(&idOk);
    if (!idOk || id <= 0)
    {
        outErrorMsg = "ID produit invalide (nombre > 0).";
        outInvalidWidget = ui->lineEdit_7;
        return false;
    }

    const QRegularExpression rxTextOnly(R"(^[A-Za-zÀ-ÿ][A-Za-zÀ-ÿ '\-]{0,79}$)");
    auto isTextOnly = [&](const QString &s) { return rxTextOnly.match(s).hasMatch(); };

    const QString nom = ui->lineEdit_9->text().trimmed();
    if (nom.isEmpty() || !isTextOnly(nom))
    {
        outErrorMsg = nom.isEmpty() ? "Nom produit obligatoire." : "Nom produit: texte seulement (pas de chiffres).";
        outInvalidWidget = ui->lineEdit_9;
        return false;
    }

    const QString typeCuir = ui->lineEdit_10->text().trimmed();
    if (typeCuir.isEmpty() || !isTextOnly(typeCuir))
    {
        outErrorMsg = typeCuir.isEmpty() ? "Type cuir obligatoire." : "Type cuir: texte seulement (pas de chiffres).";
        outInvalidWidget = ui->lineEdit_10;
        return false;
    }

    const QString categorie = ui->lineEdit_11->text().trimmed();
    if (categorie.isEmpty() || !isTextOnly(categorie))
    {
        outErrorMsg = categorie.isEmpty() ? "Catégories obligatoire." : "Catégories: texte seulement (pas de chiffres).";
        outInvalidWidget = ui->lineEdit_11;
        return false;
    }

    bool qteOk = false;
    const int qte = ui->lineEdit_12->text().trimmed().toInt(&qteOk);
    if (!qteOk || qte < 0)
    {
        outErrorMsg = "Quantité stock invalide (nombre >= 0).";
        outInvalidWidget = ui->lineEdit_12;
        return false;
    }

    const QString styleTxt = ui->lineEdit_17->text().trimmed();
    if (styleTxt.isEmpty() || !isTextOnly(styleTxt))
    {
        outErrorMsg = styleTxt.isEmpty() ? "Style obligatoire (texte)." : "Style: texte seulement (pas de chiffres).";
        outInvalidWidget = ui->lineEdit_17;
        return false;
    }

    const QDate d = QDate::fromString(ui->dateTimeEdit->text().trimmed(), "yyyy-MM-dd");
    if (!d.isValid())
    {
        outErrorMsg = "Date inacceptable. Format attendu: yyyy-MM-dd";
        outInvalidWidget = ui->dateTimeEdit;
        return false;
    }
    if (d > QDate::currentDate())
    {
        outErrorMsg = "Date fabrication ne doit pas dépasser aujourd'hui.";
        outInvalidWidget = ui->dateTimeEdit;
        return false;
    }

    outId = id;
    outQte = qte;
    return true;
}

void MainWindow::afficherProduits()
{
    // Garde l'ordre retourné par la requête (pas de tri auto UI)
    ui->employeeTable_4->setSortingEnabled(false);

    QSqlQuery query(
        "SELECT ID, NOM_PRODUIT, TYPE_CUIR, QUALITE, ETAT_PRODUIT, "
        "TYPE_DESIGN, DATE_FABRICATION, CATEGORIE, QUANTITE_STOCK, STYLE "
        "FROM PRODUITS "
        // Affiche dans l'ordre d'insertion: le dernier ajouté en bas.
        "ORDER BY ROWID ASC"
        );
    ui->employeeTable_4->setRowCount(0);
    int row = 0;
    while (query.next())
    {
        ui->employeeTable_4->insertRow(row);
        ui->employeeTable_4->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->employeeTable_4->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->employeeTable_4->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->employeeTable_4->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->employeeTable_4->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        ui->employeeTable_4->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        ui->employeeTable_4->setItem(row, 6, new QTableWidgetItem(query.value(6).toDate().toString("yyyy-MM-dd")));
        ui->employeeTable_4->setItem(row, 7, new QTableWidgetItem(query.value(7).toString()));
        ui->employeeTable_4->setItem(row, 8, new QTableWidgetItem(query.value(8).toString()));
        ui->employeeTable_4->setItem(row, 9, new QTableWidgetItem(query.value(9).toString()));
        ui->employeeTable_4->setItem(row, 10, new QTableWidgetItem(""));
        row++;
    }
}

void MainWindow::remplirFormulaire(int row, int col)
{
    Q_UNUSED(col);
    selectedProduitId = ui->employeeTable_4->item(row, 0)->text().toInt();
    ui->lineEdit_7->setText(ui->employeeTable_4->item(row, 0)->text());
    ui->lineEdit_9->setText(ui->employeeTable_4->item(row, 1)->text());
    ui->lineEdit_10->setText(ui->employeeTable_4->item(row, 2)->text());
    ui->comboBox_3->setCurrentText(ui->employeeTable_4->item(row, 3)->text());
    ui->comboBox_4->setCurrentText(ui->employeeTable_4->item(row, 4)->text());
    ui->comboBox_5->setCurrentText(ui->employeeTable_4->item(row, 5)->text());
    ui->dateTimeEdit->setDate(QDate::fromString(ui->employeeTable_4->item(row, 6)->text(), "yyyy-MM-dd"));
    ui->lineEdit_11->setText(ui->employeeTable_4->item(row, 7)->text());
    ui->lineEdit_12->setText(ui->employeeTable_4->item(row, 8)->text());
    ui->lineEdit_17->setText(ui->employeeTable_4->item(row, 9)->text());
}

void MainWindow::on_btnAjouter_6_clicked()
{
    cacherErreurSaisie();
    int id = 0, qte = 0;
    QString err;
    QWidget *invalidWidget = nullptr;
    if (!validerFormulaire(id, qte, err, invalidWidget))
    {
        afficherErreurSaisie(invalidWidget, err);
        return;
    }

    Produit p(id, ui->lineEdit_9->text().trimmed(), ui->lineEdit_11->text().trimmed(),
              ui->lineEdit_10->text().trimmed(), ui->comboBox_3->currentText(), qte,
              ui->comboBox_4->currentText(), ui->dateTimeEdit->date(),
              ui->comboBox_5->currentText(), ui->lineEdit_17->text().trimmed());

    if (p.ajouter())
    {
        afficherProduits();
        viderFormulaire();
        QMessageBox::information(this, "Succès", "Produit ajouté");
    }
    else
    {
        QMessageBox::critical(this, "Erreur SQL", Produit::lastSqlError);
    }
}

void MainWindow::on_btnModifier_4_clicked()
{
    cacherErreurSaisie();
    const int oldId = selectedProduitId;
    if (oldId <= 0)
    {
        QMessageBox::warning(this, "Validation", "Sélectionner un produit dans le tableau avant de modifier.");
        return;
    }

    int newId = 0, qte = 0;
    QString err;
    QWidget *invalidWidget = nullptr;
    if (!validerFormulaire(newId, qte, err, invalidWidget))
    {
        afficherErreurSaisie(invalidWidget, err);
        return;
    }

    Produit p(newId, ui->lineEdit_9->text().trimmed(), ui->lineEdit_11->text().trimmed(),
              ui->lineEdit_10->text().trimmed(), ui->comboBox_3->currentText(), qte,
              ui->comboBox_4->currentText(), ui->dateTimeEdit->date(),
              ui->comboBox_5->currentText(), ui->lineEdit_17->text().trimmed());

    if (p.modifier(oldId, newId))
    {
        selectedProduitId = newId;
        afficherProduits();
        viderFormulaire();
        QMessageBox::information(this, "Succès", "Produit modifié");
    }
    else
    {
        QMessageBox::critical(this, "Erreur SQL", Produit::lastSqlError);
    }
}

void MainWindow::on_btnSupprimer_4_clicked()
{
    cacherErreurSaisie();
    int id = ui->lineEdit_7->text().toInt();
    Produit p;
    if (p.supprimer(id))
    {
        afficherProduits();
        viderFormulaire();
        QMessageBox::information(this, "Succès", "Produit supprimé");
    }
    else
    {
        QMessageBox::critical(this, "Erreur SQL", Produit::lastSqlError);
    }
}
