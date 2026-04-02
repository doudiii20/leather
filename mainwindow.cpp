#include "mainwindow.h"
#include "employe.h"
#include "smtp.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QDate>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QTextStream>
#include <QStringConverter>
#include <QTextDocument>
#include <QMessageBox>

namespace {
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
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Validateurs simples (empêchent la mauvaise saisie dès le clavier)
    ui->cin->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,8}$"), ui->cin));
    ui->telephone->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,8}$"), ui->telephone));

    ui->table->setColumnCount(10);
    ui->table->setHorizontalHeaderLabels(QStringList()
        << "CIN"
        << "Nom"
        << "Prénom"
        << "Sexe"
        << "Salaire (DT)"
        << "Date d'embauche"
        << "Téléphone"
        << "Poste"
        << "Adresse"
        << "Email");
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table->setSortingEnabled(true);

    // Au démarrage, désactiver Modifier / Supprimer (aucune sélection)
    ui->btnModifier->setEnabled(false);
    ui->btnSupprimer->setEnabled(false);

    connect(ui->table, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(ui->table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onHeaderClicked);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [this](const QString &text){
        applySearchFilter(text);
        updateStats();
    });

    // Connexion pour l'assistant IA : touche Entrée
    connect(ui->lineEditChat, &QLineEdit::returnPressed, ui->btnSendChat, &QPushButton::click);

    // Bouton Export Excel (CSV) ajoute a cote de Export PDF
    auto *btnExportExcel = new QPushButton("Export Excel", this);
    btnExportExcel->setMinimumSize(QSize(110, 32));
    btnExportExcel->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    btnExportExcel->setStyleSheet(ui->btnExportPDF->styleSheet());
    ui->formBtnLayout->addWidget(btnExportExcel);
    connect(btnExportExcel, &QPushButton::clicked, this, &MainWindow::on_btnExportExcel_clicked);

    refreshTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onHeaderClicked(int logicalIndex)
{
    if (logicalIndex == lastSortColumn) {
        sortAsc = !sortAsc;
    } else {
        lastSortColumn = logicalIndex;
        sortAsc = true;
    }
    ui->table->sortItems(logicalIndex, sortAsc ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void MainWindow::applySearchFilter(const QString &text)
{
    const QString t = text.trimmed();
    for (int r = 0; r < ui->table->rowCount(); ++r) {
        bool match = t.isEmpty();
        if (!match) {
            for (int c = 0; c < ui->table->columnCount(); ++c) {
                const auto *it = ui->table->item(r, c);
                if (it && it->text().contains(t, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        ui->table->setRowHidden(r, !match);
    }
}

void MainWindow::updateStats()
{
    int total = 0;
    double sumSalaire = 0.0;
    int countSalaire = 0;
    int nouveauxMois = 0;
    const QDate now = QDate::currentDate();

    int countH = 0;
    int countF = 0;
    QMap<QString, int> posteCounts;

    for (int r = 0; r < ui->table->rowCount(); ++r) {
        if (ui->table->isRowHidden(r))
            continue;
        ++total;

        bool ok = false;
        const double sal = ui->table->item(r, 4) ? ui->table->item(r, 4)->text().replace(',', '.').toDouble(&ok) : 0.0;
        if (ok) {
            sumSalaire += sal;
            ++countSalaire;
        }

        const QString dateTxt = ui->table->item(r, 5) ? ui->table->item(r, 5)->text() : QString();
        const QDate d = QDate::fromString(dateTxt, "dd/MM/yyyy");
        if (d.isValid() && d.year() == now.year() && d.month() == now.month())
            ++nouveauxMois;

        const QString sexe = ui->table->item(r, 3) ? ui->table->item(r, 3)->text().trimmed() : QString();
        if (sexe.startsWith("H", Qt::CaseInsensitive)) {
            ++countH;
        } else if (sexe.startsWith("F", Qt::CaseInsensitive)) {
            ++countF;
        }

        const QString poste = ui->table->item(r, 7) ? ui->table->item(r, 7)->text().trimmed() : QString();
        if (!poste.isEmpty()) {
            posteCounts[poste]++;
        }
    }
    ui->statValue1->setText(QString::number(total));
    ui->statValue2->setText(countSalaire > 0 ? QString::number(sumSalaire / countSalaire, 'f', 2) : "0");
    ui->statValue3->setText(QString::number(nouveauxMois));

    ui->statValue4->setText(QString("H: %1 | F: %2").arg(countH).arg(countF));

    QString frequentPoste = "--";
    int maxCount = 0;
    for (auto it = posteCounts.begin(); it != posteCounts.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            frequentPoste = it.key();
        }
    }
    ui->statValue5->setText(frequentPoste);
}

void MainWindow::on_btnRechercher_clicked()
{
    applySearchFilter(ui->lineEditSearch->text());
    updateStats();
}

void MainWindow::on_btnExportPDF_clicked()
{
    const QString path = QFileDialog::getSaveFileName(this, "Exporter PDF", "", "PDF (*.pdf)");
    if (path.isEmpty())
        return;

    QPdfWriter writer(path);
    writer.setResolution(96); // pour une taille de texte normale à l'écran
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Portrait);
    writer.setTitle("Liste des employes");

    const QString exportDate = QDate::currentDate().toString("dd/MM/yyyy");

    QString html;
    html += "<div style='margin:40px auto; width:95%;'>";
    html += "<div style='display:flex; justify-content:space-between; align-items:center;'>";
    html += "<h1 style='font-family:Arial; font-weight:bold; font-size:20pt; margin:0;'>Liste des employes</h1>";
    html += "<div style='font-family:Arial; font-size:10pt; text-align:right;'>";
    html += "Date d'export : " + exportDate.toHtmlEscaped();
    html += "</div></div>";
    html += "<hr style='margin:12pt 0;'>";
    html += "<table border='1' cellspacing='0' cellpadding='6' "
            "style='width:100%; font-size:10pt; font-family:Arial; border-collapse:collapse; table-layout:fixed;'>";
    html += "<tr>";
    for (int c = 0; c < ui->table->columnCount(); ++c) {
        html += "<th style='background-color:#eeeeee; text-align:left; padding:4px; white-space:nowrap;'>"
                + ui->table->horizontalHeaderItem(c)->text().toHtmlEscaped() + "</th>";
    }
    html += "</tr>";

    bool alternate = false;
    for (int r = 0; r < ui->table->rowCount(); ++r) {
        if (ui->table->isRowHidden(r))
            continue;
        const QString bg = alternate ? "#f9f9f9" : "#ffffff";
        alternate = !alternate;
        html += "<tr style='background-color:" + bg + ";'>";
        for (int c = 0; c < ui->table->columnCount(); ++c) {
            const QString cell = ui->table->item(r, c) ? ui->table->item(r, c)->text() : "";
            html += "<td style='padding:4px; white-space:nowrap;'>" + cell.toHtmlEscaped() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table></div>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(QSizeF(writer.width(), writer.height()));

    QPainter painter(&writer);
    doc.drawContents(&painter);

    // Pied de page simple (adapté à un tableau sur une page)
    const QString footer = "Page 1 / 1";
    QFont footerFont("Arial", 9);
    painter.setFont(footerFont);
    const int margin = 40;
    painter.drawText(margin,
                     writer.height() - margin,
                     footer);
}

void MainWindow::on_btnExportExcel_clicked()
{
    const QString path = QFileDialog::getSaveFileName(this, "Exporter Excel (CSV)", "", "CSV (*.csv)");
    if (path.isEmpty())
        return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Erreur", "Impossible de creer le fichier CSV.");
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true); // Ajoute le BOM UTF-8 pour Excel

    // Header
    for (int c = 0; c < ui->table->columnCount(); ++c) {
        if (c) out << ';';
        out << '"' << ui->table->horizontalHeaderItem(c)->text().replace('"', "\"\"") << '"';
    }
    out << "\n";

    for (int r = 0; r < ui->table->rowCount(); ++r) {
        if (ui->table->isRowHidden(r))
            continue;
        for (int c = 0; c < ui->table->columnCount(); ++c) {
            if (c) out << ';';
            const QString cell = ui->table->item(r, c) ? ui->table->item(r, c)->text() : "";
            QString escaped = cell;
            escaped.replace('"', "\"\"");
            out << '"' << escaped << '"';
        }
        out << "\n";
    }
}

void MainWindow::refreshTable()
{
    ui->table->setRowCount(0);
    QSqlQuery query("SELECT CIN,NOM,PRENOM,SEXE,SALAIRE,TO_CHAR(DATE_EMBAUCHE,'DD/MM/YYYY'),TELEPHONE,POSTE,ADRESSE,EMAIL FROM EMPLOYE");

    int row = 0;
    while(query.next())
    {
        ui->table->insertRow(row);
        for(int col = 0; col < 10; col++)
        {
            QString val = query.value(col).toString();
            if (col == 5) {
                const QDate d = QDate::fromString(val, "dd/MM/yyyy");
                ui->table->setItem(row, col, new DateTableItem(val, d));
            } else {
                ui->table->setItem(row, col, new QTableWidgetItem(val));
            }
        }
        row++;
    }

    applySearchFilter(ui->lineEditSearch->text().trimmed());
    updateStats();
}

void MainWindow::clearChamps()
{
    ui->cin->clear();
    ui->nom->clear();
    ui->prenom->clear();
    ui->salaire->clear();
    ui->telephone->clear();
    ui->poste->clear();
    ui->adresse->clear();
    ui->email->clear();
    ui->sexe->setCurrentIndex(0);
    ui->dateEmbauche->setDate(QDate::currentDate());
}

bool MainWindow::champsValides()
{
    const QString cin = ui->cin->text().trimmed();
    const QString nom = ui->nom->text().trimmed();
    const QString prenom = ui->prenom->text().trimmed();
    QString salaireTxt = ui->salaire->text().trimmed();
    const QString tel = ui->telephone->text().trimmed();
    const QString poste = ui->poste->text().trimmed();
    const QString adresse = ui->adresse->text().trimmed();
    const QString email = ui->email->text().trimmed();

    if (cin.isEmpty() || nom.isEmpty() || prenom.isEmpty() || salaireTxt.isEmpty() ||
        tel.isEmpty() || poste.isEmpty() || adresse.isEmpty() || email.isEmpty())
    {
        QMessageBox::warning(this, "Champs vides", "Veuillez remplir tous les champs.");
        return false;
    }

    if (!QRegularExpression("^\\d{8}$").match(cin).hasMatch())
    {
        QMessageBox::warning(this, "CIN invalide", "Le CIN doit contenir exactement 8 chiffres.");
        ui->cin->setFocus();
        return false;
    }

    if (!QRegularExpression("^\\d{8}$").match(tel).hasMatch())
    {
        QMessageBox::warning(this, "Telephone invalide", "Le numero de telephone doit contenir exactement 8 chiffres.");
        ui->telephone->setFocus();
        return false;
    }

    // Email: format simple type exemple@domaine.com
    const QRegularExpression rxEmail("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!rxEmail.match(email).hasMatch())
    {
        QMessageBox::warning(this, "Email invalide",
                             "Veuillez saisir une adresse email valide (exemple@domaine.com).");
        ui->email->setFocus();
        return false;
    }

    // Nom/Prenom : lettres + espaces + tirets (simple)
    // \\p{L} = toute lettre Unicode (evite les problemes d'encodage dans le code source)
    const QRegularExpression rxNom("^[\\p{L}\\s'-]+$");
    if (!rxNom.match(nom).hasMatch() || !rxNom.match(prenom).hasMatch())
    {
        QMessageBox::warning(this, "Nom/Prenom invalide", "Nom et prenom doivent contenir seulement des lettres (avec espace ou tiret).");
        return false;
    }

    // Date d'embauche: ne doit pas être dans le futur
    const QDate dateEmb = ui->dateEmbauche->date();
    if (dateEmb > QDate::currentDate())
    {
        QMessageBox::warning(this, "Date invalide",
                             "La date d'embauche ne peut pas être postérieure à la date du jour.");
        ui->dateEmbauche->setFocus();
        return false;
    }

    bool okSalaire = false;
    salaireTxt.replace(',', '.');
    const double salaire = salaireTxt.toDouble(&okSalaire);
    if (!okSalaire || salaire <= 0)
    {
        QMessageBox::warning(this, "Salaire invalide", "Le salaire doit être un nombre positif.");
        ui->salaire->setFocus();
        return false;
    }

    return true;
}

void MainWindow::on_btnAjouter_clicked()
{
    if (!champsValides())
        return;

    // Vérifier unicité du CIN
    const QString cin = ui->cin->text().trimmed();
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE CIN = :cin");
    checkQuery.bindValue(":cin", cin);
    if (!checkQuery.exec()) {
        QMessageBox::warning(this, "Erreur",
                             "Impossible de vérifier l'unicité du CIN.\nVeuillez réessayer.");
        qDebug() << checkQuery.lastError();
        return;
    }
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "CIN déjà utilisé",
                             "Un employé avec ce CIN existe déjà. Veuillez saisir un autre CIN.");
        ui->cin->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYE VALUES (:cin,:nom,:prenom,:sexe,:salaire,TO_DATE(:date,'DD/MM/YYYY'),:tel,:poste,:adresse,:email)");

    query.bindValue(":cin", ui->cin->text().trimmed());
    query.bindValue(":nom", ui->nom->text().trimmed());
    query.bindValue(":prenom", ui->prenom->text().trimmed());
    query.bindValue(":sexe", ui->sexe->currentText());
    QString salaireNorm = ui->salaire->text();
    salaireNorm.replace(',', '.');
    double salaireVal = salaireNorm.toDouble();
    QString dateStr = ui->dateEmbauche->date().toString("dd/MM/yyyy");

    Employe E(ui->cin->text().trimmed(),
              ui->nom->text().trimmed(),
              ui->prenom->text().trimmed(),
              ui->sexe->currentText(),
              salaireVal,
              dateStr,
              ui->telephone->text().trimmed(),
              ui->poste->text().trimmed(),
              ui->adresse->text().trimmed(),
              ui->email->text().trimmed());

    if(E.ajouter()) {
        // Envoi de l'email de bienvenue
        Smtp* smtp = new Smtp("bonour39@gmail.com", "ewqyqyzuqxoosvux", "smtp.gmail.com", 465);
        QString objet = "Bienvenue chez LEATHER HOUSE !";
        QString corps = "Bonjour " + ui->prenom->text().trimmed() + ",\n\n"
                        "Nous sommes ravis de vous compter parmi nos employés en tant que " + ui->poste->text().trimmed() + ".\n\n"
                        "Cordialement,\nL'équipe LEATHER HOUSE";
        smtp->sendMail("votre_email@gmail.com", ui->email->text().trimmed(), objet, corps);

        refreshTable();
        clearChamps();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de l'ajout de l'employé.");
    }
}

void MainWindow::on_btnModifier_clicked()
{
    if(ui->cin->text().trimmed().isEmpty())
    {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un employé avant de le modifier.");
        return;
    }

    if (!champsValides())
        return;
    QString salaireNorm = ui->salaire->text();
    salaireNorm.replace(',', '.');
    double salaireVal = salaireNorm.toDouble();
    QString dateStr = ui->dateEmbauche->date().toString("dd/MM/yyyy");

    // Récupérer l'ancien salaire pour vérifier s'il y a une augmentation
    double ancienSalaire = 0.0;
    QSqlQuery queryCheck;
    queryCheck.prepare("SELECT SALAIRE FROM EMPLOYE WHERE CIN=:cin");
    queryCheck.bindValue(":cin", ui->cin->text().trimmed());
    if(queryCheck.exec() && queryCheck.next()) {
        ancienSalaire = queryCheck.value(0).toDouble();
    }

    Employe E(ui->cin->text().trimmed(),
              ui->nom->text().trimmed(),
              ui->prenom->text().trimmed(),
              ui->sexe->currentText(),
              salaireVal,
              dateStr,
              ui->telephone->text().trimmed(),
              ui->poste->text().trimmed(),
              ui->adresse->text().trimmed(),
              ui->email->text().trimmed());

    if(E.modifier()) {
        if (salaireVal > ancienSalaire) {
            Smtp* smtp = new Smtp("bonour39@gmail.com", "ewqyqyzuqxoosvux", "smtp.gmail.com", 465);
            QString objet = "Félicitations pour votre augmentation !";
            QString corps = "Bonjour " + ui->prenom->text().trimmed() + ",\n\n"
                            "Nous avons le plaisir de vous informer que votre salaire a été revalorisé.\n"
                            "Nouveau salaire : " + QString::number(salaireVal, 'f', 2) + " DT.\n\n"
                            "Merci pour votre excellent travail chez LEATHER HOUSE.\n\n"
                            "Cordialement,\nLa Direction";
            smtp->sendMail("bonour39@gmail.com", ui->email->text().trimmed(), objet, corps);
        } else if (salaireVal < ancienSalaire) {
            Smtp* smtp = new Smtp("bonour39@gmail.com", "ewqyqyzuqxoosvux", "smtp.gmail.com", 465);
            QString objet = "Notification de modification de salaire";
            QString corps = "Bonjour " + ui->prenom->text().trimmed() + ",\n\n"
                            "Nous vous informons que votre salaire a été ajusté.\n"
                            "Nouveau salaire : " + QString::number(salaireVal, 'f', 2) + " DT.\n\n"
                            "Pour plus d'informations (retard, absence, etc.), veuillez contacter les ressources humaines de LEATHER HOUSE.\n\n"
                            "Cordialement,\nLa Direction";
            smtp->sendMail("bonour39@gmail.com", ui->email->text().trimmed(), objet, corps);
        }
        refreshTable();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de la modification de l'employé.");
    }
}

void MainWindow::onTableSelectionChanged()
{
    int row = ui->table->currentRow();
    if(row < 0 || !ui->table->item(row, 0)) {
        ui->btnModifier->setEnabled(false);
        ui->btnSupprimer->setEnabled(false);
        return;
    }

    ui->btnModifier->setEnabled(true);
    ui->btnSupprimer->setEnabled(true);

    ui->cin->setText(ui->table->item(row, 0)->text());
    ui->nom->setText(ui->table->item(row, 1)->text());
    ui->prenom->setText(ui->table->item(row, 2)->text());
    ui->sexe->setCurrentText(ui->table->item(row, 3)->text());
    ui->salaire->setText(ui->table->item(row, 4)->text());
    ui->dateEmbauche->setDate(QDate::fromString(ui->table->item(row, 5)->text(), "dd/MM/yyyy"));
    ui->telephone->setText(ui->table->item(row, 6)->text());
    ui->poste->setText(ui->table->item(row, 7)->text());
    ui->adresse->setText(ui->table->item(row, 8)->text());
    ui->email->setText(ui->table->item(row, 9)->text());
}

void MainWindow::on_btnSupprimer_clicked()
{
    const QString cin = ui->cin->text().trimmed();
    const QString nom = ui->nom->text().trimmed();
    const QString prenom = ui->prenom->text().trimmed();

    if(cin.isEmpty())
    {
        QMessageBox::information(this, "Aucune sélection",
                                 "Veuillez sélectionner un employé avant de le supprimer.");
        return;
    }

    const QString question = QString("Voulez-vous vraiment supprimer l'employé suivant :\n\n"
                                     "CIN : %1\nNom : %2\nPrénom : %3 ?")
                                     .arg(cin, nom, prenom);

    auto reply = QMessageBox::question(this,
                                       "Confirmation de suppression",
                                       question,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    Employe E;
    if(E.supprimer(cin)) {
        refreshTable();
        clearChamps();
        ui->btnModifier->setEnabled(false);
        ui->btnSupprimer->setEnabled(false);
        QMessageBox::information(this, "Suppression réussie",
                                 "L'employé a été supprimé avec succès.");
    } else {
        QMessageBox::warning(this, "Erreur",
                             "Une erreur est survenue lors de la suppression de l'employé.");
    }
}

void MainWindow::on_btnAccueil_clicked()
{
    ui->contentStack->setCurrentWidget(ui->pageAccueil);
}

void MainWindow::on_btnEmployes_clicked()
{
    ui->contentStack->setCurrentWidget(ui->pageEmployes);
}

void MainWindow::on_btnSendChat_clicked()
{
    QString message = ui->lineEditChat->text().trimmed();
    if (message.isEmpty())
        return;

    // Affiche le message de l'utilisateur
    ui->chatDisplay->append("<b>Vous:</b> " + message);
    ui->lineEditChat->clear();

    // Simulation d'une réponse de l'IA
    QString reponse;
    if (message.contains("bonjour", Qt::CaseInsensitive)) {
        reponse = "Bonjour ! Je suis votre assistant virtuel pour LEATHER HOUSE. Comment puis-je vous aider ?";
    } else if (message.contains("stat", Qt::CaseInsensitive) || message.contains("employe", Qt::CaseInsensitive)) {
        reponse = "Pour voir les statistiques détaillées, vous pouvez consulter le panneau de droite dans l'onglet 'Employés'.";
    } else if (message.contains("email", Qt::CaseInsensitive) || message.contains("notification", Qt::CaseInsensitive) || message.contains("communication", Qt::CaseInsensitive) || message.contains("innovant", Qt::CaseInsensitive)) {
        reponse = "<b>Employé en notification et communication interne par email :</b><br>Ce métier innovant utilise l'application pour envoyer automatiquement des e-mails aux employés selon certaines actions (comme un email de bienvenue à l'embauche ou une alerte lors d'une augmentation de salaire).";
    } else {
        reponse = "C'est noté. Si vous avez besoin d'aide avec la gestion de vos employés, je suis là.";
    }

    ui->chatDisplay->append("<i><b>IA:</b> " + reponse + "</i><br>");
}


void MainWindow::on_comboBoxSort_currentIndexChanged(int index)
{
    ui->table->setSortingEnabled(true);

    switch(index) {
        case 0: // Pertinence
            ui->table->sortByColumn(0, Qt::AscendingOrder);
            break;
        case 1: // Alphabetique (Nom)
            ui->table->sortByColumn(1, Qt::AscendingOrder);
            break;
        case 2: // Date d'embauche
            ui->table->sortByColumn(5, Qt::AscendingOrder);
            break;
        case 3: // Salaire croissant
            ui->table->sortByColumn(4, Qt::AscendingOrder);
            break;
        case 4: // Salaire decroissant
            ui->table->sortByColumn(4, Qt::DescendingOrder);
            break;
    }
}
