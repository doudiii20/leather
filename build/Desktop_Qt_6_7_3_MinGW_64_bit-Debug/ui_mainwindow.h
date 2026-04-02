/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainVerticalLayout;
    QWidget *appHeader;
    QHBoxLayout *headerLayout;
    QLabel *headerTitle;
    QSpacerItem *headerStretch;
    QLabel *headerUserLabel;
    QWidget *bodyWidget;
    QHBoxLayout *bodyLayout;
    QWidget *sidebar;
    QVBoxLayout *sidebarLayout;
    QWidget *sidebarLogoArea;
    QHBoxLayout *sidebarLogoLayout;
    QLabel *sidebarBrandLabel;
    QFrame *sidebarSep1;
    QVBoxLayout *sidebarBtnLayout;
    QPushButton *btnAccueil;
    QPushButton *btnEmployes;
    QPushButton *btnFournisseurs;
    QPushButton *btnProduits;
    QPushButton *btnCommandes;
    QPushButton *btnParametres;
    QSpacerItem *sidebarStretch;
    QStackedWidget *contentStack;
    QWidget *pageAccueil;
    QVBoxLayout *accueilLayout;
    QLabel *accueilTitle;
    QLabel *accueilSub;
    QSpacerItem *accueilSpacer;
    QWidget *pageEmployes;
    QVBoxLayout *employesMainLayout;
    QHBoxLayout *employesHeaderRow;
    QLabel *employesTitle;
    QSpacerItem *headerSearchSpacer;
    QLabel *lblSort;
    QComboBox *comboBoxSort;
    QLineEdit *lineEditSearch;
    QPushButton *btnRechercher;
    QHBoxLayout *employesContentLayout;
    QVBoxLayout *employesLeftColumn;
    QTableWidget *table;
    QGroupBox *employeeFormBox;
    QVBoxLayout *formOuterLayout;
    QHBoxLayout *formRow1;
    QVBoxLayout *cinLayout;
    QLabel *labelCIN;
    QLineEdit *cin;
    QVBoxLayout *nomLayout;
    QLabel *labelNom;
    QLineEdit *nom;
    QVBoxLayout *prenomLayout;
    QLabel *labelPrenom;
    QLineEdit *prenom;
    QVBoxLayout *sexeLayout;
    QLabel *labelSexe;
    QComboBox *sexe;
    QVBoxLayout *salaireLayout;
    QLabel *labelSalaire;
    QLineEdit *salaire;
    QHBoxLayout *formRow2;
    QVBoxLayout *dateEmbaucheLayout;
    QLabel *labelDateEmbauche;
    QDateEdit *dateEmbauche;
    QVBoxLayout *telephoneLayout;
    QLabel *labelTelephone;
    QLineEdit *telephone;
    QVBoxLayout *posteLayout;
    QLabel *labelPoste;
    QLineEdit *poste;
    QVBoxLayout *adresseLayout;
    QLabel *labelAdresse;
    QLineEdit *adresse;
    QVBoxLayout *emailLayout;
    QLabel *labelEmail;
    QLineEdit *email;
    QHBoxLayout *formBtnLayout;
    QSpacerItem *formBtnSpacer;
    QPushButton *btnAjouter;
    QPushButton *btnModifier;
    QPushButton *btnSupprimer;
    QPushButton *btnExportPDF;
    QWidget *employesRightPanel;
    QVBoxLayout *rightColumnLayout;
    QGroupBox *statsGroupBox;
    QGridLayout *statsLayout;
    QFrame *statCard1;
    QVBoxLayout *statCard1Layout;
    QLabel *statTitle1;
    QLabel *statValue1;
    QFrame *statCard2;
    QVBoxLayout *statCard2Layout;
    QLabel *statTitle2;
    QLabel *statValue2;
    QFrame *statCard3;
    QVBoxLayout *statCard3Layout;
    QLabel *statTitle3;
    QLabel *statValue3;
    QFrame *statCard4;
    QVBoxLayout *statCard4Layout;
    QLabel *statTitle4;
    QLabel *statValue4;
    QFrame *statCard5;
    QVBoxLayout *statCard5Layout;
    QLabel *statTitle5;
    QLabel *statValue5;
    QGroupBox *chatGroupBox;
    QVBoxLayout *chatLayout;
    QTextEdit *chatDisplay;
    QHBoxLayout *chatInputLayout;
    QLineEdit *lineEditChat;
    QPushButton *btnSendChat;
    QWidget *appFooter;
    QHBoxLayout *footerLayout;
    QLabel *footerLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1280, 720);
        MainWindow->setMinimumSize(QSize(1280, 720));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainVerticalLayout = new QVBoxLayout(centralwidget);
        mainVerticalLayout->setSpacing(0);
        mainVerticalLayout->setObjectName("mainVerticalLayout");
        mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
        appHeader = new QWidget(centralwidget);
        appHeader->setObjectName("appHeader");
        appHeader->setMinimumSize(QSize(0, 56));
        appHeader->setMaximumSize(QSize(16777215, 56));
        appHeader->setStyleSheet(QString::fromUtf8("#appHeader {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border-bottom: 1px solid rgb(230, 220, 200);\n"
"}"));
        headerLayout = new QHBoxLayout(appHeader);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(16, 0, 20, 0);
        headerTitle = new QLabel(appHeader);
        headerTitle->setObjectName("headerTitle");
        headerTitle->setStyleSheet(QString::fromUtf8("font-size: 17px; font-weight: bold; color: rgb(88, 41, 0);"));

        headerLayout->addWidget(headerTitle);

        headerStretch = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        headerLayout->addItem(headerStretch);

        headerUserLabel = new QLabel(appHeader);
        headerUserLabel->setObjectName("headerUserLabel");
        headerUserLabel->setStyleSheet(QString::fromUtf8("color: rgb(88, 41, 0); font-size: 12px;"));

        headerLayout->addWidget(headerUserLabel);


        mainVerticalLayout->addWidget(appHeader);

        bodyWidget = new QWidget(centralwidget);
        bodyWidget->setObjectName("bodyWidget");
        bodyLayout = new QHBoxLayout(bodyWidget);
        bodyLayout->setSpacing(0);
        bodyLayout->setObjectName("bodyLayout");
        bodyLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QWidget(bodyWidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(220, 0));
        sidebar->setMaximumSize(QSize(220, 16777215));
        sidebar->setStyleSheet(QString::fromUtf8("#sidebar {\n"
"  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 rgb(88, 41, 0), stop:1 rgb(110, 55, 10));\n"
"}\n"
"#sidebar QPushButton {\n"
"  background: transparent;\n"
"  color: rgb(248, 241, 227);\n"
"  text-align: left;\n"
"  padding: 12px 16px;\n"
"  border: none;\n"
"  border-left: 4px solid transparent;\n"
"  font-size: 13px;\n"
"}\n"
"#sidebar QPushButton:hover {\n"
"  background-color: rgba(255, 205, 185, 40);\n"
"}"));
        sidebarLayout = new QVBoxLayout(sidebar);
        sidebarLayout->setSpacing(0);
        sidebarLayout->setObjectName("sidebarLayout");
        sidebarLayout->setContentsMargins(0, 0, 0, 0);
        sidebarLogoArea = new QWidget(sidebar);
        sidebarLogoArea->setObjectName("sidebarLogoArea");
        sidebarLogoArea->setMinimumSize(QSize(0, 70));
        sidebarLogoArea->setMaximumSize(QSize(16777215, 70));
        sidebarLogoLayout = new QHBoxLayout(sidebarLogoArea);
        sidebarLogoLayout->setObjectName("sidebarLogoLayout");
        sidebarBrandLabel = new QLabel(sidebarLogoArea);
        sidebarBrandLabel->setObjectName("sidebarBrandLabel");
        sidebarBrandLabel->setStyleSheet(QString::fromUtf8("color: rgb(248, 241, 227); font-size: 15px; font-weight: bold;"));

        sidebarLogoLayout->addWidget(sidebarBrandLabel);


        sidebarLayout->addWidget(sidebarLogoArea);

        sidebarSep1 = new QFrame(sidebar);
        sidebarSep1->setObjectName("sidebarSep1");
        sidebarSep1->setMaximumSize(QSize(16777215, 1));
        sidebarSep1->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 205, 185, 80);"));
        sidebarSep1->setFrameShape(QFrame::Shape::HLine);

        sidebarLayout->addWidget(sidebarSep1);

        sidebarBtnLayout = new QVBoxLayout();
        sidebarBtnLayout->setSpacing(2);
        sidebarBtnLayout->setObjectName("sidebarBtnLayout");
        sidebarBtnLayout->setContentsMargins(6, 10, 6, 6);
        btnAccueil = new QPushButton(sidebar);
        btnAccueil->setObjectName("btnAccueil");
        btnAccueil->setMinimumSize(QSize(0, 42));
        btnAccueil->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        sidebarBtnLayout->addWidget(btnAccueil);

        btnEmployes = new QPushButton(sidebar);
        btnEmployes->setObjectName("btnEmployes");
        btnEmployes->setMinimumSize(QSize(0, 42));
        btnEmployes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        sidebarBtnLayout->addWidget(btnEmployes);

        btnFournisseurs = new QPushButton(sidebar);
        btnFournisseurs->setObjectName("btnFournisseurs");
        btnFournisseurs->setEnabled(false);
        btnFournisseurs->setMinimumSize(QSize(0, 42));

        sidebarBtnLayout->addWidget(btnFournisseurs);

        btnProduits = new QPushButton(sidebar);
        btnProduits->setObjectName("btnProduits");
        btnProduits->setEnabled(false);
        btnProduits->setMinimumSize(QSize(0, 42));

        sidebarBtnLayout->addWidget(btnProduits);

        btnCommandes = new QPushButton(sidebar);
        btnCommandes->setObjectName("btnCommandes");
        btnCommandes->setEnabled(false);
        btnCommandes->setMinimumSize(QSize(0, 42));

        sidebarBtnLayout->addWidget(btnCommandes);

        btnParametres = new QPushButton(sidebar);
        btnParametres->setObjectName("btnParametres");
        btnParametres->setEnabled(false);
        btnParametres->setMinimumSize(QSize(0, 42));

        sidebarBtnLayout->addWidget(btnParametres);


        sidebarLayout->addLayout(sidebarBtnLayout);

        sidebarStretch = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        sidebarLayout->addItem(sidebarStretch);


        bodyLayout->addWidget(sidebar);

        contentStack = new QStackedWidget(bodyWidget);
        contentStack->setObjectName("contentStack");
        contentStack->setStyleSheet(QString::fromUtf8("#contentStack {\n"
"  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 rgb(248, 241, 227), stop:1 rgb(243, 233, 215));\n"
"}"));
        pageAccueil = new QWidget();
        pageAccueil->setObjectName("pageAccueil");
        accueilLayout = new QVBoxLayout(pageAccueil);
        accueilLayout->setObjectName("accueilLayout");
        accueilLayout->setContentsMargins(40, 40, 40, 40);
        accueilTitle = new QLabel(pageAccueil);
        accueilTitle->setObjectName("accueilTitle");
        accueilTitle->setStyleSheet(QString::fromUtf8("font-size: 24px; font-weight: bold; color: rgb(88, 41, 0);"));

        accueilLayout->addWidget(accueilTitle);

        accueilSub = new QLabel(pageAccueil);
        accueilSub->setObjectName("accueilSub");
        accueilSub->setStyleSheet(QString::fromUtf8("font-size: 13px; color: rgb(140, 100, 60);"));

        accueilLayout->addWidget(accueilSub);

        accueilSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        accueilLayout->addItem(accueilSpacer);

        contentStack->addWidget(pageAccueil);
        pageEmployes = new QWidget();
        pageEmployes->setObjectName("pageEmployes");
        employesMainLayout = new QVBoxLayout(pageEmployes);
        employesMainLayout->setSpacing(10);
        employesMainLayout->setObjectName("employesMainLayout");
        employesMainLayout->setContentsMargins(20, 16, 20, 12);
        employesHeaderRow = new QHBoxLayout();
        employesHeaderRow->setSpacing(16);
        employesHeaderRow->setObjectName("employesHeaderRow");
        employesTitle = new QLabel(pageEmployes);
        employesTitle->setObjectName("employesTitle");
        employesTitle->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: rgb(88, 41, 0);"));

        employesHeaderRow->addWidget(employesTitle);

        headerSearchSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        employesHeaderRow->addItem(headerSearchSpacer);

        lblSort = new QLabel(pageEmployes);
        lblSort->setObjectName("lblSort");
        lblSort->setStyleSheet(QString::fromUtf8("font-size: 12px; color: rgb(88, 41, 0); font-weight: bold;"));

        employesHeaderRow->addWidget(lblSort);

        comboBoxSort = new QComboBox(pageEmployes);
        comboBoxSort->addItem(QString());
        comboBoxSort->addItem(QString());
        comboBoxSort->addItem(QString());
        comboBoxSort->addItem(QString());
        comboBoxSort->addItem(QString());
        comboBoxSort->setObjectName("comboBoxSort");
        comboBoxSort->setMinimumSize(QSize(130, 32));
        comboBoxSort->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 4px 10px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: white;\n"
"}\n"
"QComboBox::drop-down {\n"
"  border-left: 1px solid rgb(200, 150, 100);\n"
"}\n"
""));

        employesHeaderRow->addWidget(comboBoxSort);

        lineEditSearch = new QLineEdit(pageEmployes);
        lineEditSearch->setObjectName("lineEditSearch");
        lineEditSearch->setMinimumSize(QSize(250, 32));
        lineEditSearch->setMaximumSize(QSize(350, 32));
        lineEditSearch->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 4px 10px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: white;\n"
"}"));

        employesHeaderRow->addWidget(lineEditSearch);

        btnRechercher = new QPushButton(pageEmployes);
        btnRechercher->setObjectName("btnRechercher");
        btnRechercher->setMinimumSize(QSize(90, 32));
        btnRechercher->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnRechercher->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 4px 14px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        employesHeaderRow->addWidget(btnRechercher);


        employesMainLayout->addLayout(employesHeaderRow);

        employesContentLayout = new QHBoxLayout();
        employesContentLayout->setSpacing(14);
        employesContentLayout->setObjectName("employesContentLayout");
        employesLeftColumn = new QVBoxLayout();
        employesLeftColumn->setSpacing(10);
        employesLeftColumn->setObjectName("employesLeftColumn");
        table = new QTableWidget(pageEmployes);
        if (table->columnCount() < 10)
            table->setColumnCount(10);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        table->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        table->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        table->setObjectName("table");
        table->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"  background-color: white;\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 6px;\n"
"  gridline-color: rgb(230, 220, 200);\n"
"  font-size: 11px;\n"
"  color: rgb(88, 41, 0);\n"
"}\n"
"QTableWidget::item {\n"
"  padding: 4px 6px;\n"
"}\n"
"QTableWidget::item:selected {\n"
"  background-color: rgb(255, 205, 185);\n"
"  color: rgb(88, 41, 0);\n"
"}\n"
"QHeaderView::section {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  padding: 6px 6px;\n"
"  border: none;\n"
"  font-weight: bold;\n"
"  font-size: 11px;\n"
"}"));
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        table->setColumnCount(10);

        employesLeftColumn->addWidget(table);

        employeeFormBox = new QGroupBox(pageEmployes);
        employeeFormBox->setObjectName("employeeFormBox");
        employeeFormBox->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"  background-color: white;\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 6px;\n"
"  margin-top: 8px;\n"
"  padding-top: 16px;\n"
"  font-weight: bold;\n"
"  font-size: 13px;\n"
"  color: rgb(88, 41, 0);\n"
"}\n"
"QGroupBox::title {\n"
"  subcontrol-origin: margin;\n"
"  left: 14px;\n"
"  padding: 0 6px;\n"
"}\n"
"QLineEdit, QComboBox, QDateEdit {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 5px 8px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: rgb(255, 255, 255);\n"
"  font-weight: normal;\n"
"}\n"
"QLineEdit:focus, QComboBox:focus, QDateEdit:focus {\n"
"  border: 1px solid rgb(88, 41, 0);\n"
"}\n"
"QLabel {\n"
"  font-size: 12px;\n"
"  font-weight: normal;\n"
"  color: rgb(88, 41, 0);\n"
"}"));
        formOuterLayout = new QVBoxLayout(employeeFormBox);
        formOuterLayout->setSpacing(8);
        formOuterLayout->setObjectName("formOuterLayout");
        formOuterLayout->setContentsMargins(12, 10, 12, 10);
        formRow1 = new QHBoxLayout();
        formRow1->setSpacing(12);
        formRow1->setObjectName("formRow1");
        cinLayout = new QVBoxLayout();
        cinLayout->setSpacing(2);
        cinLayout->setObjectName("cinLayout");
        labelCIN = new QLabel(employeeFormBox);
        labelCIN->setObjectName("labelCIN");

        cinLayout->addWidget(labelCIN);

        cin = new QLineEdit(employeeFormBox);
        cin->setObjectName("cin");

        cinLayout->addWidget(cin);


        formRow1->addLayout(cinLayout);

        nomLayout = new QVBoxLayout();
        nomLayout->setSpacing(2);
        nomLayout->setObjectName("nomLayout");
        labelNom = new QLabel(employeeFormBox);
        labelNom->setObjectName("labelNom");

        nomLayout->addWidget(labelNom);

        nom = new QLineEdit(employeeFormBox);
        nom->setObjectName("nom");

        nomLayout->addWidget(nom);


        formRow1->addLayout(nomLayout);

        prenomLayout = new QVBoxLayout();
        prenomLayout->setSpacing(2);
        prenomLayout->setObjectName("prenomLayout");
        labelPrenom = new QLabel(employeeFormBox);
        labelPrenom->setObjectName("labelPrenom");

        prenomLayout->addWidget(labelPrenom);

        prenom = new QLineEdit(employeeFormBox);
        prenom->setObjectName("prenom");

        prenomLayout->addWidget(prenom);


        formRow1->addLayout(prenomLayout);

        sexeLayout = new QVBoxLayout();
        sexeLayout->setSpacing(2);
        sexeLayout->setObjectName("sexeLayout");
        labelSexe = new QLabel(employeeFormBox);
        labelSexe->setObjectName("labelSexe");

        sexeLayout->addWidget(labelSexe);

        sexe = new QComboBox(employeeFormBox);
        sexe->addItem(QString());
        sexe->addItem(QString());
        sexe->setObjectName("sexe");

        sexeLayout->addWidget(sexe);


        formRow1->addLayout(sexeLayout);

        salaireLayout = new QVBoxLayout();
        salaireLayout->setSpacing(2);
        salaireLayout->setObjectName("salaireLayout");
        labelSalaire = new QLabel(employeeFormBox);
        labelSalaire->setObjectName("labelSalaire");

        salaireLayout->addWidget(labelSalaire);

        salaire = new QLineEdit(employeeFormBox);
        salaire->setObjectName("salaire");

        salaireLayout->addWidget(salaire);


        formRow1->addLayout(salaireLayout);


        formOuterLayout->addLayout(formRow1);

        formRow2 = new QHBoxLayout();
        formRow2->setSpacing(12);
        formRow2->setObjectName("formRow2");
        dateEmbaucheLayout = new QVBoxLayout();
        dateEmbaucheLayout->setSpacing(2);
        dateEmbaucheLayout->setObjectName("dateEmbaucheLayout");
        labelDateEmbauche = new QLabel(employeeFormBox);
        labelDateEmbauche->setObjectName("labelDateEmbauche");

        dateEmbaucheLayout->addWidget(labelDateEmbauche);

        dateEmbauche = new QDateEdit(employeeFormBox);
        dateEmbauche->setObjectName("dateEmbauche");
        dateEmbauche->setCalendarPopup(true);

        dateEmbaucheLayout->addWidget(dateEmbauche);


        formRow2->addLayout(dateEmbaucheLayout);

        telephoneLayout = new QVBoxLayout();
        telephoneLayout->setSpacing(2);
        telephoneLayout->setObjectName("telephoneLayout");
        labelTelephone = new QLabel(employeeFormBox);
        labelTelephone->setObjectName("labelTelephone");

        telephoneLayout->addWidget(labelTelephone);

        telephone = new QLineEdit(employeeFormBox);
        telephone->setObjectName("telephone");

        telephoneLayout->addWidget(telephone);


        formRow2->addLayout(telephoneLayout);

        posteLayout = new QVBoxLayout();
        posteLayout->setSpacing(2);
        posteLayout->setObjectName("posteLayout");
        labelPoste = new QLabel(employeeFormBox);
        labelPoste->setObjectName("labelPoste");

        posteLayout->addWidget(labelPoste);

        poste = new QLineEdit(employeeFormBox);
        poste->setObjectName("poste");

        posteLayout->addWidget(poste);


        formRow2->addLayout(posteLayout);

        adresseLayout = new QVBoxLayout();
        adresseLayout->setSpacing(2);
        adresseLayout->setObjectName("adresseLayout");
        labelAdresse = new QLabel(employeeFormBox);
        labelAdresse->setObjectName("labelAdresse");

        adresseLayout->addWidget(labelAdresse);

        adresse = new QLineEdit(employeeFormBox);
        adresse->setObjectName("adresse");

        adresseLayout->addWidget(adresse);


        formRow2->addLayout(adresseLayout);

        emailLayout = new QVBoxLayout();
        emailLayout->setSpacing(2);
        emailLayout->setObjectName("emailLayout");
        labelEmail = new QLabel(employeeFormBox);
        labelEmail->setObjectName("labelEmail");

        emailLayout->addWidget(labelEmail);

        email = new QLineEdit(employeeFormBox);
        email->setObjectName("email");

        emailLayout->addWidget(email);


        formRow2->addLayout(emailLayout);


        formOuterLayout->addLayout(formRow2);

        formBtnLayout = new QHBoxLayout();
        formBtnLayout->setSpacing(8);
        formBtnLayout->setObjectName("formBtnLayout");
        formBtnSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        formBtnLayout->addItem(formBtnSpacer);

        btnAjouter = new QPushButton(employeeFormBox);
        btnAjouter->setObjectName("btnAjouter");
        btnAjouter->setMinimumSize(QSize(90, 32));
        btnAjouter->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAjouter->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 6px 14px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        formBtnLayout->addWidget(btnAjouter);

        btnModifier = new QPushButton(employeeFormBox);
        btnModifier->setObjectName("btnModifier");
        btnModifier->setMinimumSize(QSize(90, 32));
        btnModifier->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnModifier->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(255, 205, 185);\n"
"  color: rgb(88, 41, 0);\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 6px 14px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(230, 180, 150);\n"
"}"));

        formBtnLayout->addWidget(btnModifier);

        btnSupprimer = new QPushButton(employeeFormBox);
        btnSupprimer->setObjectName("btnSupprimer");
        btnSupprimer->setMinimumSize(QSize(90, 32));
        btnSupprimer->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnSupprimer->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(200, 50, 50);\n"
"  color: white;\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 6px 14px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(180, 40, 40);\n"
"}"));

        formBtnLayout->addWidget(btnSupprimer);

        btnExportPDF = new QPushButton(employeeFormBox);
        btnExportPDF->setObjectName("btnExportPDF");
        btnExportPDF->setMinimumSize(QSize(100, 32));
        btnExportPDF->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportPDF->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(50, 120, 50);\n"
"  color: white;\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 6px 14px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(40, 100, 40);\n"
"}"));

        formBtnLayout->addWidget(btnExportPDF);


        formOuterLayout->addLayout(formBtnLayout);


        employesLeftColumn->addWidget(employeeFormBox);


        employesContentLayout->addLayout(employesLeftColumn);

        employesRightPanel = new QWidget(pageEmployes);
        employesRightPanel->setObjectName("employesRightPanel");
        employesRightPanel->setMinimumSize(QSize(280, 0));
        employesRightPanel->setMaximumSize(QSize(320, 16777215));
        rightColumnLayout = new QVBoxLayout(employesRightPanel);
        rightColumnLayout->setSpacing(10);
        rightColumnLayout->setObjectName("rightColumnLayout");
        rightColumnLayout->setContentsMargins(0, 0, 0, 0);
        statsGroupBox = new QGroupBox(employesRightPanel);
        statsGroupBox->setObjectName("statsGroupBox");
        statsGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"  background-color: white;\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 6px;\n"
"  margin-top: 8px;\n"
"  padding-top: 14px;\n"
"  font-weight: bold;\n"
"  font-size: 13px;\n"
"  color: rgb(88, 41, 0);\n"
"}\n"
"QGroupBox::title {\n"
"  subcontrol-origin: margin;\n"
"  left: 14px;\n"
"  padding: 0 6px;\n"
"}\n"
"QLabel {\n"
"  font-weight: normal;\n"
"}"));
        statsLayout = new QGridLayout(statsGroupBox);
        statsLayout->setSpacing(6);
        statsLayout->setObjectName("statsLayout");
        statsLayout->setContentsMargins(10, 8, 10, 8);
        statCard1 = new QFrame(statsGroupBox);
        statCard1->setObjectName("statCard1");
        statCard1->setStyleSheet(QString::fromUtf8("QFrame {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  padding: 8px;\n"
"}"));
        statCard1Layout = new QVBoxLayout(statCard1);
        statCard1Layout->setSpacing(2);
        statCard1Layout->setObjectName("statCard1Layout");
        statTitle1 = new QLabel(statCard1);
        statTitle1->setObjectName("statTitle1");
        statTitle1->setStyleSheet(QString::fromUtf8("font-size: 11px; color: rgb(140, 100, 60); border: none; font-weight: bold;"));

        statCard1Layout->addWidget(statTitle1);

        statValue1 = new QLabel(statCard1);
        statValue1->setObjectName("statValue1");
        statValue1->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: rgb(88, 41, 0); border: none;"));

        statCard1Layout->addWidget(statValue1);


        statsLayout->addWidget(statCard1, 0, 0, 1, 1);

        statCard2 = new QFrame(statsGroupBox);
        statCard2->setObjectName("statCard2");
        statCard2->setStyleSheet(QString::fromUtf8("QFrame {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  padding: 8px;\n"
"}"));
        statCard2Layout = new QVBoxLayout(statCard2);
        statCard2Layout->setSpacing(2);
        statCard2Layout->setObjectName("statCard2Layout");
        statTitle2 = new QLabel(statCard2);
        statTitle2->setObjectName("statTitle2");
        statTitle2->setStyleSheet(QString::fromUtf8("font-size: 11px; color: rgb(140, 100, 60); border: none; font-weight: bold;"));

        statCard2Layout->addWidget(statTitle2);

        statValue2 = new QLabel(statCard2);
        statValue2->setObjectName("statValue2");
        statValue2->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: rgb(88, 41, 0); border: none;"));

        statCard2Layout->addWidget(statValue2);


        statsLayout->addWidget(statCard2, 0, 1, 1, 1);

        statCard3 = new QFrame(statsGroupBox);
        statCard3->setObjectName("statCard3");
        statCard3->setStyleSheet(QString::fromUtf8("QFrame {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  padding: 8px;\n"
"}"));
        statCard3Layout = new QVBoxLayout(statCard3);
        statCard3Layout->setSpacing(2);
        statCard3Layout->setObjectName("statCard3Layout");
        statTitle3 = new QLabel(statCard3);
        statTitle3->setObjectName("statTitle3");
        statTitle3->setStyleSheet(QString::fromUtf8("font-size: 11px; color: rgb(140, 100, 60); border: none; font-weight: bold;"));

        statCard3Layout->addWidget(statTitle3);

        statValue3 = new QLabel(statCard3);
        statValue3->setObjectName("statValue3");
        statValue3->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: rgb(88, 41, 0); border: none;"));

        statCard3Layout->addWidget(statValue3);


        statsLayout->addWidget(statCard3, 1, 0, 1, 1);

        statCard4 = new QFrame(statsGroupBox);
        statCard4->setObjectName("statCard4");
        statCard4->setStyleSheet(QString::fromUtf8("QFrame {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  padding: 8px;\n"
"}"));
        statCard4Layout = new QVBoxLayout(statCard4);
        statCard4Layout->setSpacing(2);
        statCard4Layout->setObjectName("statCard4Layout");
        statTitle4 = new QLabel(statCard4);
        statTitle4->setObjectName("statTitle4");
        statTitle4->setStyleSheet(QString::fromUtf8("font-size: 11px; color: rgb(140, 100, 60); border: none; font-weight: bold;"));

        statCard4Layout->addWidget(statTitle4);

        statValue4 = new QLabel(statCard4);
        statValue4->setObjectName("statValue4");
        statValue4->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: rgb(88, 41, 0); border: none;"));

        statCard4Layout->addWidget(statValue4);


        statsLayout->addWidget(statCard4, 1, 1, 1, 1);

        statCard5 = new QFrame(statsGroupBox);
        statCard5->setObjectName("statCard5");
        statCard5->setStyleSheet(QString::fromUtf8("QFrame {\n"
"  background-color: rgb(248, 241, 227);\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  padding: 8px;\n"
"}"));
        statCard5Layout = new QVBoxLayout(statCard5);
        statCard5Layout->setSpacing(2);
        statCard5Layout->setObjectName("statCard5Layout");
        statTitle5 = new QLabel(statCard5);
        statTitle5->setObjectName("statTitle5");
        statTitle5->setStyleSheet(QString::fromUtf8("font-size: 11px; color: rgb(140, 100, 60); border: none; font-weight: bold;"));

        statCard5Layout->addWidget(statTitle5);

        statValue5 = new QLabel(statCard5);
        statValue5->setObjectName("statValue5");
        statValue5->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: rgb(88, 41, 0); border: none;"));

        statCard5Layout->addWidget(statValue5);


        statsLayout->addWidget(statCard5, 2, 0, 1, 2);


        rightColumnLayout->addWidget(statsGroupBox);

        chatGroupBox = new QGroupBox(employesRightPanel);
        chatGroupBox->setObjectName("chatGroupBox");
        chatGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"  background-color: white;\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 6px;\n"
"  margin-top: 8px;\n"
"  padding-top: 14px;\n"
"  font-weight: bold;\n"
"  font-size: 13px;\n"
"  color: rgb(88, 41, 0);\n"
"}\n"
"QGroupBox::title {\n"
"  subcontrol-origin: margin;\n"
"  left: 14px;\n"
"  padding: 0 6px;\n"
"}\n"
"QTextEdit {\n"
"  border: 1px solid rgb(230, 220, 200);\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: rgb(252, 248, 240);\n"
"  font-weight: normal;\n"
"}\n"
"QLineEdit {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 5px 8px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: white;\n"
"  font-weight: normal;\n"
"}"));
        chatLayout = new QVBoxLayout(chatGroupBox);
        chatLayout->setSpacing(6);
        chatLayout->setObjectName("chatLayout");
        chatLayout->setContentsMargins(10, 8, 10, 8);
        chatDisplay = new QTextEdit(chatGroupBox);
        chatDisplay->setObjectName("chatDisplay");
        chatDisplay->setReadOnly(true);

        chatLayout->addWidget(chatDisplay);

        chatInputLayout = new QHBoxLayout();
        chatInputLayout->setSpacing(6);
        chatInputLayout->setObjectName("chatInputLayout");
        lineEditChat = new QLineEdit(chatGroupBox);
        lineEditChat->setObjectName("lineEditChat");

        chatInputLayout->addWidget(lineEditChat);

        btnSendChat = new QPushButton(chatGroupBox);
        btnSendChat->setObjectName("btnSendChat");
        btnSendChat->setMinimumSize(QSize(70, 30));
        btnSendChat->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnSendChat->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 4px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 5px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        chatInputLayout->addWidget(btnSendChat);


        chatLayout->addLayout(chatInputLayout);


        rightColumnLayout->addWidget(chatGroupBox);


        employesContentLayout->addWidget(employesRightPanel);


        employesMainLayout->addLayout(employesContentLayout);

        contentStack->addWidget(pageEmployes);

        bodyLayout->addWidget(contentStack);


        mainVerticalLayout->addWidget(bodyWidget);

        appFooter = new QWidget(centralwidget);
        appFooter->setObjectName("appFooter");
        appFooter->setMinimumSize(QSize(0, 28));
        appFooter->setMaximumSize(QSize(16777215, 28));
        appFooter->setStyleSheet(QString::fromUtf8("#appFooter {\n"
"  background-color: rgb(243, 233, 215);\n"
"  border-top: 1px solid rgb(230, 220, 200);\n"
"}"));
        footerLayout = new QHBoxLayout(appFooter);
        footerLayout->setObjectName("footerLayout");
        footerLayout->setContentsMargins(16, 0, 16, 0);
        footerLabel = new QLabel(appFooter);
        footerLabel->setObjectName("footerLabel");
        footerLabel->setStyleSheet(QString::fromUtf8("color: rgb(88, 41, 0); font-size: 10px;"));
        footerLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        footerLayout->addWidget(footerLabel);


        mainVerticalLayout->addWidget(appFooter);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        contentStack->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LEATHER HOUSE", nullptr));
        headerTitle->setText(QCoreApplication::translate("MainWindow", "LEATHER HOUSE", nullptr));
        headerUserLabel->setText(QCoreApplication::translate("MainWindow", "Administrateur", nullptr));
        sidebarBrandLabel->setText(QCoreApplication::translate("MainWindow", "  LEATHER HOUSE", nullptr));
        btnAccueil->setText(QCoreApplication::translate("MainWindow", "  Accueil", nullptr));
        btnEmployes->setText(QCoreApplication::translate("MainWindow", "  Employes", nullptr));
        btnFournisseurs->setText(QCoreApplication::translate("MainWindow", "  Fournisseurs", nullptr));
        btnProduits->setText(QCoreApplication::translate("MainWindow", "  Produits", nullptr));
        btnCommandes->setText(QCoreApplication::translate("MainWindow", "  Commandes", nullptr));
        btnParametres->setText(QCoreApplication::translate("MainWindow", "  Parametres", nullptr));
        accueilTitle->setText(QCoreApplication::translate("MainWindow", "Bienvenue sur LEATHER HOUSE", nullptr));
        accueilSub->setText(QCoreApplication::translate("MainWindow", "Systeme de gestion integre. Selectionnez un module dans le menu.", nullptr));
        employesTitle->setText(QCoreApplication::translate("MainWindow", "Gestion des Employes", nullptr));
        lblSort->setText(QCoreApplication::translate("MainWindow", "Trier par :", nullptr));
        comboBoxSort->setItemText(0, QCoreApplication::translate("MainWindow", "Pertinence", nullptr));
        comboBoxSort->setItemText(1, QCoreApplication::translate("MainWindow", "Alphab\303\251tique (Nom)", nullptr));
        comboBoxSort->setItemText(2, QCoreApplication::translate("MainWindow", "Date d'embauche", nullptr));
        comboBoxSort->setItemText(3, QCoreApplication::translate("MainWindow", "Salaire croissant", nullptr));
        comboBoxSort->setItemText(4, QCoreApplication::translate("MainWindow", "Salaire d\303\251croissant", nullptr));

        lineEditSearch->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un employe...", nullptr));
        btnRechercher->setText(QCoreApplication::translate("MainWindow", "Rechercher", nullptr));
        QTableWidgetItem *___qtablewidgetitem = table->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "CIN", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = table->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = table->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = table->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Sexe", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = table->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Salaire", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = table->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Date Embauche", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = table->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Telephone", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = table->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = table->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Adresse", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = table->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "Email", nullptr));
        employeeFormBox->setTitle(QCoreApplication::translate("MainWindow", "Fiche Employe", nullptr));
        labelCIN->setText(QCoreApplication::translate("MainWindow", "CIN", nullptr));
        cin->setPlaceholderText(QCoreApplication::translate("MainWindow", "CIN", nullptr));
        labelNom->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        nom->setPlaceholderText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        labelPrenom->setText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        prenom->setPlaceholderText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        labelSexe->setText(QCoreApplication::translate("MainWindow", "Sexe", nullptr));
        sexe->setItemText(0, QCoreApplication::translate("MainWindow", "Homme", nullptr));
        sexe->setItemText(1, QCoreApplication::translate("MainWindow", "Femme", nullptr));

        labelSalaire->setText(QCoreApplication::translate("MainWindow", "Salaire", nullptr));
        salaire->setPlaceholderText(QCoreApplication::translate("MainWindow", "Salaire", nullptr));
        labelDateEmbauche->setText(QCoreApplication::translate("MainWindow", "Date Embauche", nullptr));
        dateEmbauche->setDisplayFormat(QCoreApplication::translate("MainWindow", "dd/MM/yyyy", nullptr));
        labelTelephone->setText(QCoreApplication::translate("MainWindow", "Telephone", nullptr));
        telephone->setPlaceholderText(QCoreApplication::translate("MainWindow", "Telephone", nullptr));
        labelPoste->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        poste->setPlaceholderText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        labelAdresse->setText(QCoreApplication::translate("MainWindow", "Adresse", nullptr));
        adresse->setPlaceholderText(QCoreApplication::translate("MainWindow", "Adresse", nullptr));
        labelEmail->setText(QCoreApplication::translate("MainWindow", "Email", nullptr));
        email->setPlaceholderText(QCoreApplication::translate("MainWindow", "Email", nullptr));
        btnAjouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btnModifier->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnSupprimer->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        btnExportPDF->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        statsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        statTitle1->setText(QCoreApplication::translate("MainWindow", "Total Employes", nullptr));
        statValue1->setText(QCoreApplication::translate("MainWindow", "--", nullptr));
        statTitle2->setText(QCoreApplication::translate("MainWindow", "Salaire Moyen", nullptr));
        statValue2->setText(QCoreApplication::translate("MainWindow", "--", nullptr));
        statTitle3->setText(QCoreApplication::translate("MainWindow", "Nouveaux ce mois", nullptr));
        statValue3->setText(QCoreApplication::translate("MainWindow", "--", nullptr));
        statTitle4->setText(QCoreApplication::translate("MainWindow", "Genre (H / F)", nullptr));
        statValue4->setText(QCoreApplication::translate("MainWindow", "--", nullptr));
        statTitle5->setText(QCoreApplication::translate("MainWindow", "Poste frequent", nullptr));
        statValue5->setText(QCoreApplication::translate("MainWindow", "--", nullptr));
        chatGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Assistant IA", nullptr));
        chatDisplay->setPlaceholderText(QCoreApplication::translate("MainWindow", "Les reponses de l'assistant apparaitront ici...", nullptr));
        lineEditChat->setPlaceholderText(QCoreApplication::translate("MainWindow", "Posez votre question...", nullptr));
        btnSendChat->setText(QCoreApplication::translate("MainWindow", "Envoyer", nullptr));
        footerLabel->setText(QCoreApplication::translate("MainWindow", "\302\251 2025 LEATHER HOUSE  |  Version 1.0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
