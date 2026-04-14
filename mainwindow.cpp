#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QDebug>
#include <QRegularExpression>
#include <QToolTip>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QStringList>
#include <QProcessEnvironment>
#include <QMetaObject>
#include <QProcess>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

#include "apiclient.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    apiClient = new ApiClient(this);

    if (ui->lineEdit_18) {
        ui->lineEdit_18->setPlaceholderText(
            QStringLiteral("Nom, type de cuir ou etat..."));
    }
    if (ui->textEdit_2) {
        ui->textEdit_2->setPlaceholderText(
            QStringLiteral("Dictez ou tapez le nom ou l ID du produit (Windows: Win+H pour dicter), puis cliquez Voix."));
    }
    if (ui->lineEdit_19) {
        ui->lineEdit_19->setPlaceholderText(
            QStringLiteral("Posez une question sur un produit (nom ou ID)..."));
        QAction *historyAction = ui->lineEdit_19->addAction(QStringLiteral("Questions anciennes"), QLineEdit::TrailingPosition);
        connect(historyAction, &QAction::triggered, this, [this]() {
            if (questionAnswerHistory.isEmpty()) {
                QMessageBox::information(this, QStringLiteral("Historique"),
                                         QStringLiteral("Aucune question ancienne pour le moment."));
                return;
            }

            QStringList items;
            for (int i = questionAnswerHistory.size() - 1; i >= 0; --i) {
                items << questionAnswerHistory.at(i).first;
            }
            bool ok = false;
            const QString chosen = QInputDialog::getItem(
                this,
                QStringLiteral("Questions anciennes"),
                QStringLiteral("Choisissez une question :"),
                items,
                0,
                false,
                &ok);
            if (!ok || chosen.isEmpty())
                return;

            for (int i = questionAnswerHistory.size() - 1; i >= 0; --i) {
                if (questionAnswerHistory.at(i).first == chosen) {
                    ui->lineEdit_19->setText(chosen);
                    ui->textEdit_3->setPlainText(
                        QStringLiteral("Question ancienne:\n%1\n\nReponse memoire:\n%2")
                            .arg(questionAnswerHistory.at(i).first,
                                 questionAnswerHistory.at(i).second));
                    break;
                }
            }
        });
    }

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

    connect(ui->lineEdit_18, &QLineEdit::textChanged, this, [this]() {
        afficherProduits(ui->lineEdit_18->text());
    });
    connect(ui->lineEdit_18, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_4_clicked);
    connect(ui->comboBox_6, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        afficherProduits(ui->lineEdit_18->text());
    });
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

void MainWindow::afficherProduits(const QString &texteFiltre)
{
    // Garde l'ordre retourné par la requête (pas de tri auto UI)
    ui->employeeTable_4->setSortingEnabled(false);

    const QString f = texteFiltre.trimmed();
    QString sql =
        "SELECT ID, NOM_PRODUIT, TYPE_CUIR, QUALITE, ETAT_PRODUIT, "
        "TYPE_DESIGN, DATE_FABRICATION, CATEGORIE, QUANTITE_STOCK, STYLE "
        "FROM PRODUITS ";
    if (!f.isEmpty()) {
        int crit = ui->comboBox_6->currentIndex();
        if (crit < 0 || crit > 2)
            crit = 0;
        switch (crit) {
        case 0:
            sql += "WHERE NOM_PRODUIT LIKE ? ESCAPE '!' ";
            break;
        case 1:
            sql += "WHERE TYPE_CUIR LIKE ? ESCAPE '!' ";
            break;
        case 2:
            sql += "WHERE ETAT_PRODUIT LIKE ? ESCAPE '!' ";
            break;
        }
    }
    sql += "ORDER BY ROWID ASC";

    QSqlQuery query;
    query.prepare(sql);
    if (!f.isEmpty()) {
        QString esc = f;
        const QChar excl(0x21);
        const QChar pct(0x25);
        const QChar und(0x5F);
        esc.replace(excl, QStringLiteral("!!"));
        esc.replace(pct, QStringLiteral("!%"));
        esc.replace(und, QStringLiteral("!_"));
        query.addBindValue(QStringLiteral("%") + esc + QStringLiteral("%"));
    }
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur SQL", query.lastError().text());
        return;
    }

    remplirTableProduitsDepuisRequete(query);
}

void MainWindow::remplirTableProduitsDepuisRequete(QSqlQuery &query)
{
    ui->employeeTable_4->setRowCount(0);
    int row = 0;
    while (query.next()) {
        ui->employeeTable_4->insertRow(row);
        for (int c = 0; c <= 9; ++c) {
            if (c == 6) {
                const QString d = query.value(6).toDate().toString(QStringLiteral("yyyy-MM-dd"));
                ui->employeeTable_4->setItem(row, c, new QTableWidgetItem(d));
            } else {
                ui->employeeTable_4->setItem(row, c, new QTableWidgetItem(query.value(c).toString()));
            }
        }
        ui->employeeTable_4->setItem(row, 10, new QTableWidgetItem(QString()));
        ++row;
    }
}

void MainWindow::rechercherProduitParNomOuId(const QString &texteBrut)
{
    const QString t = texteBrut.trimmed();
    if (t.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("Recherche vocale"),
            QStringLiteral("Saisissez ou dictez le nom ou l ID dans la zone de texte au dessus, puis cliquez Voix.\n\n"
                           "Astuce Windows : cliquez dans la zone, puis Win+H pour la dictee."));
        return;
    }

    ui->employeeTable_4->setSortingEnabled(false);
    QString sql =
        QStringLiteral(
            "SELECT ID, NOM_PRODUIT, TYPE_CUIR, QUALITE, ETAT_PRODUIT, "
            "TYPE_DESIGN, DATE_FABRICATION, CATEGORIE, QUANTITE_STOCK, STYLE "
            "FROM PRODUITS WHERE ");
    QSqlQuery query;
    bool idOk = false;
    const int id = t.toInt(&idOk);
    if (idOk && id > 0) {
        sql += QStringLiteral(
            "ID = ? OR NOM_PRODUIT LIKE ? ESCAPE '!' OR TYPE_CUIR LIKE ? ESCAPE '!' "
            "OR ETAT_PRODUIT LIKE ? ESCAPE '!' ORDER BY ROWID ASC");
        query.prepare(sql);
        query.addBindValue(id);
        QString esc = t;
        const QChar excl(0x21);
        const QChar pct(0x25);
        const QChar und(0x5F);
        esc.replace(excl, QStringLiteral("!!"));
        esc.replace(pct, QStringLiteral("!%"));
        esc.replace(und, QStringLiteral("!_"));
        const QString like = QStringLiteral("%") + esc + QStringLiteral("%");
        query.addBindValue(like);
        query.addBindValue(like);
        query.addBindValue(like);
    } else {
        sql += QStringLiteral(
            "NOM_PRODUIT LIKE ? ESCAPE '!' OR TYPE_CUIR LIKE ? ESCAPE '!' "
            "OR ETAT_PRODUIT LIKE ? ESCAPE '!' ORDER BY ROWID ASC");
        query.prepare(sql);
        QString esc = t;
        const QChar excl(0x21);
        const QChar pct(0x25);
        const QChar und(0x5F);
        esc.replace(excl, QStringLiteral("!!"));
        esc.replace(pct, QStringLiteral("!%"));
        esc.replace(und, QStringLiteral("!_"));
        const QString like = QStringLiteral("%") + esc + QStringLiteral("%");
        query.addBindValue(like);
        query.addBindValue(like);
        query.addBindValue(like);
    }
    if (!query.exec()) {
        QMessageBox::critical(this, QStringLiteral("Erreur SQL"), query.lastError().text());
        return;
    }
    remplirTableProduitsDepuisRequete(query);
    const int n = ui->employeeTable_4->rowCount();
    ui->textEdit_2->append(
        QStringLiteral("\n---\nRecherche intelligente : %1\n%2 produit(s) dans le tableau.")
            .arg(t)
            .arg(n));
}

QPair<bool, QString> MainWindow::transcrireVoixWindows()
{
#ifdef Q_OS_WIN
    QStringList nomsArray;
    QSqlQuery q;
    q.exec("SELECT DISTINCT NOM_PRODUIT FROM PRODUITS");
    while (q.next()) {
        QString nom = q.value(0).toString().trimmed();
        if (!nom.isEmpty()) {
            nom.replace("'", "''"); // echapper pour powershell
            nomsArray << QStringLiteral("'%1'").arg(nom);
        }
    }
    q.exec("SELECT ID FROM PRODUITS");
    while (q.next()) {
        QString id = q.value(0).toString().trimmed();
        if (!id.isEmpty()) {
            nomsArray << QStringLiteral("'%1'").arg(id);
        }
    }

    QString script;
    if (nomsArray.isEmpty()) {
        script = QStringLiteral(
            "$ErrorActionPreference='SilentlyContinue';"
            "Add-Type -AssemblyName System.Speech;"
            "$cult='fr-FR';"
            "try {"
            "  $ci=New-Object System.Globalization.CultureInfo($cult);"
            "  $rec=New-Object System.Speech.Recognition.SpeechRecognitionEngine($ci);"
            "} catch {"
            "  $rec=New-Object System.Speech.Recognition.SpeechRecognitionEngine;"
            "}"
            "$rec.LoadGrammar((New-Object System.Speech.Recognition.DictationGrammar));"
            "$rec.SetInputToDefaultAudioDevice();"
            "$res=$rec.Recognize([TimeSpan]::FromSeconds(8));"
            "if($res -and $res.Text){ [Console]::OutputEncoding=[Text.UTF8Encoding]::UTF8; Write-Output $res.Text }");
    } else {
        script = QStringLiteral(
            "$ErrorActionPreference='SilentlyContinue';"
            "Add-Type -AssemblyName System.Speech;"
            "$cult='fr-FR';"
            "try {"
            "  $ci=New-Object System.Globalization.CultureInfo($cult);"
            "  $rec=New-Object System.Speech.Recognition.SpeechRecognitionEngine($ci);"
            "} catch {"
            "  $rec=New-Object System.Speech.Recognition.SpeechRecognitionEngine;"
            "}"
            "$choices = New-Object System.Speech.Recognition.Choices;"
            "$choices.Add([string[]]@(%1));"
            "$gb = New-Object System.Speech.Recognition.GrammarBuilder;"
            "$gb.Culture = $rec.RecognizerInfo.Culture;"
            "$gb.Append($choices);"
            "$grammar = New-Object System.Speech.Recognition.Grammar($gb);"
            "$rec.LoadGrammar($grammar);"
            "$rec.SetInputToDefaultAudioDevice();"
            "$res=$rec.Recognize([TimeSpan]::FromSeconds(8));"
            "if($res -and $res.Text){ [Console]::OutputEncoding=[Text.UTF8Encoding]::UTF8; Write-Output $res.Text }"
        ).arg(nomsArray.join(","));
    }

    QProcess p;
    p.start(QStringLiteral("powershell"),
            QStringList() << QStringLiteral("-NoProfile")
                          << QStringLiteral("-ExecutionPolicy") << QStringLiteral("Bypass")
                          << QStringLiteral("-Command") << script);
    if (!p.waitForStarted(3000)) {
        return qMakePair(false, QStringLiteral("Impossible de demarrer l ecoute vocale."));
    }
    if (!p.waitForFinished(12000)) {
        p.kill();
        return qMakePair(false, QStringLiteral("Temps d ecoute depasse, veuillez reessayer."));
    }
    const QString stdErr = QString::fromUtf8(p.readAllStandardError()).trimmed();
    const QString text = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
    if (!text.isEmpty()) {
        return qMakePair(true, text);
    }
    if (!stdErr.isEmpty()) {
        return qMakePair(false, stdErr);
    }
    return qMakePair(false, QStringLiteral("Aucun son reconnu. Rapprochez le micro et reessayez."));
#else
    return qMakePair(false, QStringLiteral("La recherche vocale automatique est supportee ici seulement sous Windows."));
#endif
}

QString MainWindow::construireReponseChatbot(const QString &entree)
{
    const QString q = entree.trimmed();
    if (q.isEmpty()) {
        return QStringLiteral(
            "Indiquez le nom ou l ID du produit. Exemple : sac cuir ou 3.");
    }

    QSqlQuery query;
    bool idOk = false;
    const int id = q.toInt(&idOk);
    if (idOk && id > 0) {
        query.prepare(
            QStringLiteral(
                "SELECT ID, NOM_PRODUIT, TYPE_CUIR, QUALITE, ETAT_PRODUIT, TYPE_DESIGN, "
                "DATE_FABRICATION, CATEGORIE, QUANTITE_STOCK, STYLE FROM PRODUITS WHERE ID = ?"));
        query.addBindValue(id);
    } else {
        query.prepare(
            QStringLiteral(
                "SELECT ID, NOM_PRODUIT, TYPE_CUIR, QUALITE, ETAT_PRODUIT, TYPE_DESIGN, "
                "DATE_FABRICATION, CATEGORIE, QUANTITE_STOCK, STYLE FROM PRODUITS "
                "WHERE NOM_PRODUIT LIKE ? ESCAPE '!' ORDER BY ROWID ASC LIMIT 1"));
        QString esc = q;
        const QChar excl(0x21);
        const QChar pct(0x25);
        const QChar und(0x5F);
        esc.replace(excl, QStringLiteral("!!"));
        esc.replace(pct, QStringLiteral("!%"));
        esc.replace(und, QStringLiteral("!_"));
        query.addBindValue(QStringLiteral("%") + esc + QStringLiteral("%"));
    }
    if (!query.exec())
        return QStringLiteral("Erreur d acces a la base de donnees.");
    if (!query.next()) {
        return QStringLiteral("Aucun produit ne correspond a votre demande.");
    }

    const QString nom = query.value(1).toString();
    const QString stock = query.value(8).toString();
    QString synth = QStringLiteral(
                        "Voici une description du produit \"%1\" :\n\n"
                        "Article en %2, categorie %3, style %4. Qualite : %5. Etat : %6. "
                        "Stock actuel : %7 unites.")
                        .arg(nom,
                             query.value(2).toString(),
                             query.value(7).toString(),
                             query.value(9).toString(),
                             query.value(3).toString(),
                             query.value(4).toString(),
                             stock);

    if (stock == QStringLiteral("0")) {
        synth += QStringLiteral("\n\nAttention : rupture de stock (0 unite).");
    }
    return synth;
}

QString MainWindow::construireContexteCataloguePourApi()
{
    QSqlQuery q;
    const bool ok = q.exec(QStringLiteral(
        "SELECT ID, NOM_PRODUIT, TYPE_CUIR, CATEGORIE, QUALITE, ETAT_PRODUIT, "
        "QUANTITE_STOCK, STYLE, DATE_FABRICATION, TYPE_DESIGN "
        "FROM PRODUITS ORDER BY ROWID ASC"));
    if (!ok)
        return QStringLiteral("(Impossible de lire le catalogue.)");

    QStringList lines;
    lines << QStringLiteral("Echantillon du catalogue :");
    int n = 0;
    while (q.next()) {
        ++n;
        lines << QStringLiteral(
                     "ID=%1 | nom=%2 | cuir=%3 | cat=%4 | qual=%5 | etat=%6 | stock=%7 | style=%8 | date=%9 | design=%10")
                     .arg(q.value(0).toString())
                     .arg(q.value(1).toString())
                     .arg(q.value(2).toString())
                     .arg(q.value(3).toString())
                     .arg(q.value(4).toString())
                     .arg(q.value(5).toString())
                     .arg(q.value(6).toString())
                     .arg(q.value(7).toString())
                     .arg(q.value(8).toDate().toString(QStringLiteral("yyyy-MM-dd")))
                     .arg(q.value(9).toString());
    }
    if (n == 0)
        lines << QStringLiteral("(Aucun produit en base.)");
    return lines.join(QChar('\n'));
}

QString MainWindow::construireReponseLocaleAvancee(const QString &question)
{
    const QString q = question.trimmed().toLower();
    if (q.isEmpty())
        return QStringLiteral("Posez une question sur les produits, le stock, les types de cuir, l etat, la qualite ou les categories.");

    QSqlQuery totalQ;
    int total = 0;
    if (totalQ.exec(QStringLiteral("SELECT COUNT(*) FROM PRODUITS")) && totalQ.next())
        total = totalQ.value(0).toInt();

    if (q.contains(QStringLiteral("combien")) || q.contains(QStringLiteral("total"))) {
        return QStringLiteral("Il y a %1 produit(s) dans le catalogue.").arg(total);
    }

    if (q.contains(QStringLiteral("rupture")) || q.contains(QStringLiteral("stock 0")) || q.contains(QStringLiteral("sans stock"))) {
        QSqlQuery r;
        if (!r.exec(QStringLiteral("SELECT ID, NOM_PRODUIT FROM PRODUITS WHERE COALESCE(QUANTITE_STOCK,0)=0 ORDER BY ID")))
            return QStringLiteral("Impossible de lire les ruptures de stock.");
        QStringList lines;
        int n = 0;
        while (r.next()) {
            ++n;
            lines << QStringLiteral("- ID %1 : %2").arg(r.value(0).toString(), r.value(1).toString());
        }
        if (n == 0)
            return QStringLiteral("Aucun produit en rupture de stock.");
        return QStringLiteral("Produits en rupture (%1):\n%2").arg(n).arg(lines.join(QChar('\n')));
    }

    QSqlQuery filt;
    filt.prepare(QStringLiteral(
        "SELECT ID, NOM_PRODUIT, TYPE_CUIR, ETAT_PRODUIT, QUANTITE_STOCK, CATEGORIE, STYLE "
        "FROM PRODUITS WHERE LOWER(NOM_PRODUIT) LIKE ? OR LOWER(TYPE_CUIR) LIKE ? OR "
        "LOWER(ETAT_PRODUIT) LIKE ? OR LOWER(CATEGORIE) LIKE ? OR LOWER(STYLE) LIKE ? "
        "ORDER BY ROWID ASC"));
    const QString like = QStringLiteral("%") + q + QStringLiteral("%");
    filt.addBindValue(like);
    filt.addBindValue(like);
    filt.addBindValue(like);
    filt.addBindValue(like);
    filt.addBindValue(like);
    if (!filt.exec())
        return QStringLiteral("Je n arrive pas a interroger la base de donnees maintenant.");

    QStringList found;
    int count = 0;
    while (filt.next()) {
        ++count;
        if (count <= 8) {
            found << QStringLiteral("- ID %1 | %2 | cuir=%3 | etat=%4 | stock=%5 | cat=%6 | style=%7")
                         .arg(filt.value(0).toString(),
                              filt.value(1).toString(),
                              filt.value(2).toString(),
                              filt.value(3).toString(),
                              filt.value(4).toString(),
                              filt.value(5).toString(),
                              filt.value(6).toString());
        }
    }

    if (count > 0) {
        QString rep = QStringLiteral("J ai trouve %1 produit(s) correspondant a \"%2\".\n%3")
                          .arg(count)
                          .arg(question)
                          .arg(found.join(QChar('\n')));
        if (count > 8)
            rep += QStringLiteral("\n... (affichage limite a 8 lignes)");
        return rep;
    }

    QSqlQuery typesQ;
    QStringList types;
    if (typesQ.exec(QStringLiteral("SELECT DISTINCT TYPE_CUIR FROM PRODUITS ORDER BY TYPE_CUIR"))) {
        while (typesQ.next()) {
            const QString t = typesQ.value(0).toString().trimmed();
            if (!t.isEmpty())
                types << t;
        }
    }
    return QStringLiteral(
               "Je ne trouve pas de correspondance exacte pour \"%1\".\n"
               "Vous pouvez demander par nom, type de cuir, etat, categorie, style, stock, rupture.\n"
               "Types de cuir disponibles: %2")
        .arg(question, types.isEmpty() ? QStringLiteral("(aucun)") : types.join(QStringLiteral(", ")));
}

bool MainWindow::envoyerEmailAlerteStockSiConfigure(const QString &corpsTexte,
                                                    int nombreProduits,
                                                    QString &outInfo)
{
    outInfo.clear();

    QString web3formsKey = QStringLiteral("903fcaae-a5c4-4e6e-bdd8-ec8e593a3cd8");

    if (web3formsKey.trimmed().isEmpty()) {
        outInfo = QStringLiteral("Email non envoyé : clé Web3Forms manquante.");
        return false;
    }

    const QString subject = QStringLiteral("[Alerte stock] %1 produit(s) concernes").arg(nombreProduits);
    const QString corps = QStringLiteral("Bonjour,\n\n%1\n\n--\nNotification automatique Web3Forms.")
                              .arg(corpsTexte);

    QMetaObject::Connection *conn = new QMetaObject::Connection();
    *conn = connect(apiClient, &ApiClient::emailCompleted, this,
                    [this, conn](bool ok, const QString &err) {
                        QObject::disconnect(*conn);
                        delete conn;
                        if (!ok) {
                            qWarning() << "Alerte email:" << err;
                            QMessageBox::critical(this, QStringLiteral("Erreur Email Web3Forms"), 
                                QStringLiteral("L'email n'a pas pu être envoyé !\nVérifiez votre connexion internet ou que votre Access Key Web3Forms est correcte.\n\nDétails de l'erreur : ") + err);
                        } else {
                            QMessageBox::information(this, QStringLiteral("Email Envoyé avec Succès"), 
                                QStringLiteral("L'alerte a été correctement envoyée sans problème d'IP ! Veuillez vérifier votre boîte de réception."));
                        }
                    });

    apiClient->postWeb3FormsMail(web3formsKey, subject, corps);
    outInfo = QStringLiteral("Demande d envoi email lancee via Web3Forms.");
    return true;
}

void MainWindow::on_pushButton_6_clicked()
{
    if (ui->textEdit)
        ui->textEdit->clear();

    QSqlQuery q;
    q.prepare(QStringLiteral(
        "SELECT ID, NOM_PRODUIT, QUANTITE_STOCK FROM PRODUITS "
        "WHERE COALESCE(QUANTITE_STOCK, 0) = 0 ORDER BY ID"));
    if (!q.exec()) {
        QMessageBox::critical(this, QStringLiteral("Erreur SQL"), q.lastError().text());
        return;
    }

    QStringList lines;
    lines << QStringLiteral("Alerte : produits sans stock (quantite = 0).\n");
    int n = 0;
    while (q.next()) {
        ++n;
        lines << QStringLiteral("- ID %1 : %2 (stock = %3)")
                     .arg(q.value(0).toString())
                     .arg(q.value(1).toString())
                     .arg(q.value(2).toString());
    }

    if (n == 0) {
        QMessageBox::information(this,
                                 QStringLiteral("Stock"),
                                 QStringLiteral("Aucune alerte : aucun produit avec stock a 0."));
        return;
    }

    const QString corps = lines.join(QStringLiteral("\n"));
    QString emailInfo;
    const bool emailStarted = envoyerEmailAlerteStockSiConfigure(corps, n, emailInfo);
    QMessageBox::warning(
        this,
        QStringLiteral("Alerte production"),
        emailStarted
            ? QStringLiteral("%1 produit(s) n'ont plus de stock (0).\nLa requête d'email d'alerte est lancée. Veuillez patienter quelques secondes pour la confirmation finale (ou une erreur réseau)...")
                  .arg(n)
            : QStringLiteral("%1 produit(s) n'ont plus de stock (0). Email non configuré.")
                  .arg(n));
}

void MainWindow::on_pushButton_7_clicked()
{
    ui->textEdit_2->setFocus();
    ui->textEdit_2->clear();
    ui->textEdit_2->append(QStringLiteral("Ecoute en cours (8 secondes)..."));

    const QPair<bool, QString> r = transcrireVoixWindows();

    if (r.first) {
        QString texteTranscrit = r.second.trimmed();
        ui->textEdit_2->setPlainText(texteTranscrit);

        // Chercher le produit le plus proche phonétiquement
        QString meilleurMatch = trouverMeilleurMatch(texteTranscrit);

        if (!meilleurMatch.isEmpty()) {
            ui->textEdit_2->append(
                QStringLiteral("Correspondance trouvee : ") + meilleurMatch);
            rechercherProduitParNomOuId(meilleurMatch);
        } else {
            rechercherProduitParNomOuId(texteTranscrit);
        }
        return;
    }

    const QString fallback = ui->textEdit_2->toPlainText().trimmed();
    if (!fallback.isEmpty()) {
        ui->textEdit_2->append(QStringLiteral("Echec micro, recherche avec texte saisi."));
        rechercherProduitParNomOuId(fallback);
        return;
    }

    QMessageBox::information(this, QStringLiteral("Recherche vocale"), r.second);
}
// ── Levenshtein (distance d'edition entre deux chaines) ──────────────────
static int levenshtein(const QString &s1, const QString &s2)
{
    int n = s1.size(), m = s2.size();
    QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1, 0));
    for (int i = 0; i <= n; i++) dp[i][0] = i;
    for (int j = 0; j <= m; j++) dp[0][j] = j;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            dp[i][j] = (s1[i-1].toLower() == s2[j-1].toLower())
                           ? dp[i-1][j-1]
                           : 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
    return dp[n][m];
}

// ── Trouve le nom de produit le plus proche de la transcription ──────────
QString MainWindow::trouverMeilleurMatch(const QString &transcrit)
{
    QSqlQuery q("SELECT NOM_PRODUIT FROM PRODUITS");

    QString meilleur;
    int meilleurScore = INT_MAX;
    QString transcritLower = transcrit.toLower().simplified();

    QStringList mots = transcritLower.split(' ', Qt::SkipEmptyParts);

    while (q.next()) {
        QString nom = q.value(0).toString();
        QString nomLower = nom.toLower();

        // Ignorer les noms trop courts (moins de 3 caractères)
        if (nomLower.length() < 3) continue;

        int dist = INT_MAX;

        // Tester chaque mot de la transcription séparément
        for (const QString &mot : mots) {
            // Ignorer les mots parasites courts (the, a, was, is...)
            if (mot.length() < 3) continue;

            int d = levenshtein(mot, nomLower);
            if (d < dist) dist = d;
        }

        // Si tous les mots étaient trop courts, tester la phrase entière
        if (dist == INT_MAX)
            dist = levenshtein(transcritLower, nomLower);

        // Seuil STRICT : distance <= 1 pour noms courts, <= 2 pour noms longs
        int seuil = (nomLower.length() <= 5) ? 1 : 2;

        if (dist < meilleurScore && dist <= seuil) {
            meilleurScore = dist;
            meilleur = nom;
        }
    }

    return meilleur;
}

void MainWindow::on_pushButton_8_clicked()
{
    const QString question = ui->lineEdit_19->text().trimmed();
    ui->lineEdit_19->clear();

    const QString qLower = question.toLower();
    const bool forceImageByPrefix = qLower.startsWith(QStringLiteral("img "));
    const bool imageKeywordIntent =
        qLower.contains(QStringLiteral("image")) ||
        qLower.contains(QStringLiteral("photo")) ||
        qLower.contains(QStringLiteral("dessine")) ||
        qLower.contains(QStringLiteral("dessin")) ||
        qLower.contains(QStringLiteral("render")) ||
        qLower.contains(QStringLiteral("modele")) ||
        qLower.contains(QStringLiteral("modèle")) ||
        qLower.contains(QStringLiteral("creer")) ||
        qLower.contains(QStringLiteral("créer")) ||
        qLower.contains(QStringLiteral("genere")) ||
        qLower.contains(QStringLiteral("génère"));
    const bool wantsImage = qLower.contains(QStringLiteral("genere une image")) ||
                            qLower.contains(QStringLiteral("génère une image")) ||
                            qLower.contains(QStringLiteral("creer une image")) ||
                            qLower.contains(QStringLiteral("créer une image")) ||
                            qLower.startsWith(QStringLiteral("/image")) ||
                            qLower.startsWith(QStringLiteral("image:")) ||
                            forceImageByPrefix ||
                            imageKeywordIntent;
    if (wantsImage) {
        QString prompt = question;
        if (forceImageByPrefix)
            prompt = question.mid(4);
        prompt.replace(QStringLiteral("/image"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("image:"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("genere une image"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("génère une image"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("creer une image"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("créer une image"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("genere"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("génère"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("creer"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("créer"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("dessine"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("dessin"), QString(), Qt::CaseInsensitive);
        prompt.replace(QStringLiteral("render"), QString(), Qt::CaseInsensitive);
        prompt.replace(QRegularExpression(QStringLiteral("\\bmodele\\b"), QRegularExpression::CaseInsensitiveOption), QString());
        prompt.replace(QRegularExpression(QStringLiteral("\\bmodèle\\b"), QRegularExpression::CaseInsensitiveOption), QString());
        prompt.replace(QRegularExpression(QStringLiteral("\\bimage\\b"), QRegularExpression::CaseInsensitiveOption), QString());
        prompt.replace(QRegularExpression(QStringLiteral("\\bphoto\\b"), QRegularExpression::CaseInsensitiveOption), QString());
        prompt = prompt.trimmed();
        if (prompt.isEmpty()) {
            ui->textEdit_3->setPlainText(
                QStringLiteral("Assistant : Donnez un prompt. Exemple: genere une image d un sac en cuir noir luxe."));
            return;
        }

        ui->textEdit_3->setPlainText(QStringLiteral("Assistant : Generation d image en cours..."));
        const QString originalPrompt = prompt;
        QMetaObject::Connection *imgConn = new QMetaObject::Connection();
        *imgConn = connect(apiClient, &ApiClient::imageGenerated, this,
                           [this, imgConn, originalPrompt](const QByteArray &bytes, const QString &err) {
                               QObject::disconnect(*imgConn);
                               delete imgConn;
                               if (!err.isEmpty()) {
                                   ui->textEdit_3->setPlainText(
                                       QStringLiteral("Assistant : Echec generation image : %1").arg(err));
                                   return;
                               }

                               QPixmap pix;
                               if (!pix.loadFromData(bytes)) {
                                   ui->textEdit_3->setPlainText(
                                       QStringLiteral("Assistant : Image recue mais format non reconnu."));
                                   return;
                               }

                               const QString dir = QDir::homePath() + QStringLiteral("/Pictures/LeatherAI");
                               QDir().mkpath(dir);
                               const QString path = dir + QStringLiteral("/image_%1.png")
                                                            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));
                               pix.save(path, "PNG");

                               QDialog *dlg = new QDialog(this);
                               dlg->setWindowTitle(QStringLiteral("Image IA - %1").arg(originalPrompt));
                               dlg->resize(900, 900);
                               QVBoxLayout *lay = new QVBoxLayout(dlg);
                               QLabel *imgLabel = new QLabel(dlg);
                               imgLabel->setAlignment(Qt::AlignCenter);
                               imgLabel->setPixmap(pix.scaled(860, 820, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                               lay->addWidget(imgLabel);
                               dlg->setLayout(lay);
                               dlg->show();

                               ui->textEdit_3->setPlainText(
                                   QStringLiteral("Assistant : Image generee avec succes.\nSauvegardee ici:\n%1").arg(path));
                           });
        const QString strictPrompt = QStringLiteral(
                                         "Generate exactly the requested image, no approximation, no unrelated objects. "
                                         "Follow the user request literally. Keep subject, color, material, style, and composition faithful. "
                                         "High detail, realistic lighting, clean background, sharp focus, professional quality. "
                                         "Negative constraints: no text, no watermark, no logo, no extra objects, no distortion, no blur. "
                                         "User request: %1")
                                         .arg(prompt);
        apiClient->generateImageFromPrompt(strictPrompt);
        return;
    }

    if (question.isEmpty()) {
        const QString rep = construireReponseLocaleAvancee(question);
        ui->textEdit_3->setPlainText(QStringLiteral("Assistant : %1").arg(rep));
        chatHistoryTurns << QStringLiteral("Assistant: %1").arg(rep);
        while (chatHistoryTurns.size() > maxChatHistoryTurns)
            chatHistoryTurns.removeFirst();
        questionAnswerHistory.append(qMakePair(QStringLiteral("(vide)"), rep));
        while (questionAnswerHistory.size() > 50)
            questionAnswerHistory.removeFirst();
        return;
    }

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString groqKey = env.value(QStringLiteral("GROQ_API_KEY"));
    if (groqKey.isEmpty()) {
        chatHistoryTurns << QStringLiteral("Utilisateur: %1").arg(question);
        while (chatHistoryTurns.size() > maxChatHistoryTurns)
            chatHistoryTurns.removeFirst();
        const QString rep = construireReponseLocaleAvancee(question);
        ui->textEdit_3->setPlainText(QStringLiteral("Assistant : %1").arg(rep));
        chatHistoryTurns << QStringLiteral("Assistant: %1").arg(rep);
        while (chatHistoryTurns.size() > maxChatHistoryTurns)
            chatHistoryTurns.removeFirst();
        questionAnswerHistory.append(qMakePair(question, rep));
        while (questionAnswerHistory.size() > 50)
            questionAnswerHistory.removeFirst();
        ui->textEdit_3->append(
            QStringLiteral("(IA gratuite : creer une cle sur console.groq.com puis definir GROQ_API_KEY.)\n"));
        return;
    }

    const QString groqUrl = QStringLiteral("https://api.groq.com/openai/v1/chat/completions");
    const QString model = env.value(QStringLiteral("GROQ_MODEL"), QStringLiteral("llama-3.1-8b-instant"));
    const QString systemPrompt = QStringLiteral(
        "Tu es un assistant IA generaliste, utile et pedagogique. "
        "Tu peux repondre a n importe quelle question (etudes, programmation, redaction, culture generale, etc.). "
        "Reponds en francais clair, structure et concis. "
        "Quand la question porte sur les produits/cuir/stock/catalogue de l application, appuie-toi en priorite "
        "sur le catalogue fourni. "
        "Si la question n a pas de lien avec le catalogue, reponds normalement comme une IA generale.");

    QString conversationMemo;
    if (!chatHistoryTurns.isEmpty()) {
        conversationMemo = QStringLiteral("Historique recent de conversation:\n%1\n\n")
                               .arg(chatHistoryTurns.join(QStringLiteral("\n")));
    }
    const QString userMsg = QStringLiteral(
        "%1"
        "Nouvelle question utilisateur:\n%2\n\n"
        "Catalogue interne (a utiliser seulement si la question concerne l application de gestion):\n%3")
                                .arg(conversationMemo, question, construireContexteCataloguePourApi());

    chatHistoryTurns << QStringLiteral("Utilisateur: %1").arg(question);
    while (chatHistoryTurns.size() > maxChatHistoryTurns)
        chatHistoryTurns.removeFirst();

    const QString qCopy = question;
    QMetaObject::Connection *conn = new QMetaObject::Connection();
    *conn = connect(apiClient, &ApiClient::chatCompleted, this,
                    [this, conn, qCopy](const QString &text, const QString &err) {
                        QObject::disconnect(*conn);
                        delete conn;
                        if (!err.isEmpty()) {
                            ui->textEdit_3->setPlainText(
                                QStringLiteral("Assistant (erreur API) : %1").arg(err));
                            chatHistoryTurns << QStringLiteral("Assistant: [erreur API] %1").arg(err);
                            while (chatHistoryTurns.size() > maxChatHistoryTurns)
                                chatHistoryTurns.removeFirst();
                            const QString localFallback = construireReponseLocaleAvancee(qCopy);
                            ui->textEdit_3->append(
                                QStringLiteral("Repli local : %1\n").arg(localFallback));
                            chatHistoryTurns << QStringLiteral("Assistant: %1").arg(localFallback);
                            while (chatHistoryTurns.size() > maxChatHistoryTurns)
                                chatHistoryTurns.removeFirst();
                            questionAnswerHistory.append(qMakePair(qCopy, localFallback));
                            while (questionAnswerHistory.size() > 50)
                                questionAnswerHistory.removeFirst();
                            return;
                        }
                        ui->textEdit_3->setPlainText(QStringLiteral("Assistant : %1").arg(text));
                        chatHistoryTurns << QStringLiteral("Assistant: %1").arg(text);
                        while (chatHistoryTurns.size() > maxChatHistoryTurns)
                            chatHistoryTurns.removeFirst();
                        questionAnswerHistory.append(qMakePair(qCopy, text));
                        while (questionAnswerHistory.size() > 50)
                            questionAnswerHistory.removeFirst();
                    });

    apiClient->postChatCompletion(groqKey, groqUrl, model, systemPrompt, userMsg);
}

void MainWindow::on_btnRechercher_4_clicked()
{
    afficherProduits(ui->lineEdit_18->text());
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
        if (qte == 0) {
            const QString corps = QStringLiteral("Ajout d un produit avec stock a 0.\nID %1 : %2")
            .arg(QString::number(id), ui->lineEdit_9->text().trimmed());
            QString emailInfo;
            envoyerEmailAlerteStockSiConfigure(corps, 1, emailInfo);
        }
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
        if (qte == 0) {
            const QString corps = QStringLiteral(
                                      "Modification : produit passe a stock 0.\nID %1 : %2")
                                      .arg(QString::number(newId), ui->lineEdit_9->text().trimmed());
            QString emailInfo;
            envoyerEmailAlerteStockSiConfigure(corps, 1, emailInfo);
        }
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
