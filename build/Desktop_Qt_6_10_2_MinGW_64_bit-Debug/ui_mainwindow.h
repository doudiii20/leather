/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
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
    QLabel *accueilTitle;
    QLabel *accueilSub;
    QLabel *headerUserLabel;
    QWidget *bodyWidget;
    QHBoxLayout *bodyLayout;
    QWidget *sidebar;
    QWidget *sidebarLogoArea;
    QHBoxLayout *sidebarLogoLayout;
    QLabel *sidebarBrandLabel;
    QFrame *sidebarSep1;
    QPushButton *btnParametres;
    QPushButton *btnMatieres;
    QPushButton *btnFournisseurs;
    QPushButton *btnCommandes;
    QPushButton *btnAccueil;
    QPushButton *btnProduits;
    QPushButton *btnEmployes;
    QStackedWidget *contentStack;
    QWidget *pageAccueil;
    QVBoxLayout *verticalLayout;
    QWidget *pageEmployes;
    QVBoxLayout *employesMainLayout;
    QHBoxLayout *employesHeaderRow;
    QLabel *employesTitle;
    QSpacerItem *headerSearchSpacer;
    QLineEdit *lineEditSearch;
    QPushButton *btnRechercher;
    QComboBox *comboBoxTri;
    QComboBox *comboBoxOrdre;
    QHBoxLayout *employesContentLayout;
    QVBoxLayout *employesLeftColumn;
    QTableWidget *employeeTable;
    QGroupBox *employeeFormBox;
    QVBoxLayout *formOuterLayout;
    QHBoxLayout *formRow1;
    QVBoxLayout *cinLayout;
    QLabel *labelCIN;
    QLineEdit *lineEditCIN;
    QVBoxLayout *nomLayout;
    QLabel *labelNom;
    QLineEdit *lineEditNom;
    QVBoxLayout *prenomLayout;
    QLabel *labelPrenom;
    QLineEdit *lineEditPrenom;
    QVBoxLayout *sexeLayout;
    QLabel *labelSexe;
    QComboBox *comboBoxSexe;
    QVBoxLayout *salaireLayout;
    QLabel *labelSalaire;
    QLineEdit *lineEditSalaire;
    QHBoxLayout *formRow2;
    QVBoxLayout *dateEmbaucheLayout;
    QLabel *labelDateEmbauche;
    QDateEdit *dateEditEmbauche;
    QVBoxLayout *telephoneLayout;
    QLabel *labelTelephone;
    QLineEdit *lineEditTelephone;
    QVBoxLayout *posteLayout;
    QLabel *labelPoste;
    QLineEdit *lineEditPoste;
    QVBoxLayout *adresseLayout;
    QLabel *labelAdresse;
    QLineEdit *lineEditAdresse;
    QVBoxLayout *emailLayout;
    QLabel *labelEmail;
    QLineEdit *lineEditEmail;
    QHBoxLayout *formBtnLayout;
    QSpacerItem *formBtnSpacer;
    QPushButton *btnAjouter;
    QPushButton *btnModifier;
    QPushButton *btnSupprimer;
    QPushButton *btnExportPDF;
    QPushButton *btnExportExcel;
    QPushButton *btnFiltreDisponible;
    QPushButton *btnFiltreSeuilCritique;
    QPushButton *btnAfficherTous;
    QWidget *employesRightPanel;
    QVBoxLayout *rightColumnLayout;
    QGroupBox *statsGroupBox;
    QVBoxLayout *statsLayout;
    QFrame *statCard1;
    QVBoxLayout *statCard1Layout;
    QLabel *statTitle1;
    QFrame *statCard2;
    QVBoxLayout *statCard2Layout;
    QLabel *statTitle2;
    QFrame *statCard3;
    QVBoxLayout *statCard3Layout;
    QLabel *statTitle3;
    QGroupBox *chatGroupBox;
    QVBoxLayout *chatLayout;
    QCalendarWidget *calendarWidget;
    QHBoxLayout *chatInputLayout;
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

        accueilTitle = new QLabel(appHeader);
        accueilTitle->setObjectName("accueilTitle");
        accueilTitle->setStyleSheet(QString::fromUtf8("font-size: 24px; font-weight: bold; color: rgb(88, 41, 0);"));

        headerLayout->addWidget(accueilTitle);

        accueilSub = new QLabel(appHeader);
        accueilSub->setObjectName("accueilSub");
        accueilSub->setStyleSheet(QString::fromUtf8("font-size: 13px; color: rgb(140, 100, 60);"));

        headerLayout->addWidget(accueilSub);

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
        sidebarLogoArea = new QWidget(sidebar);
        sidebarLogoArea->setObjectName("sidebarLogoArea");
        sidebarLogoArea->setGeometry(QRect(0, 0, 147, 70));
        sidebarLogoArea->setMinimumSize(QSize(0, 70));
        sidebarLogoArea->setMaximumSize(QSize(16777215, 70));
        sidebarLogoLayout = new QHBoxLayout(sidebarLogoArea);
        sidebarLogoLayout->setObjectName("sidebarLogoLayout");
        sidebarBrandLabel = new QLabel(sidebarLogoArea);
        sidebarBrandLabel->setObjectName("sidebarBrandLabel");
        sidebarBrandLabel->setStyleSheet(QString::fromUtf8("color: rgb(248, 241, 227); font-size: 15px; font-weight: bold;"));

        sidebarLogoLayout->addWidget(sidebarBrandLabel);

        sidebarSep1 = new QFrame(sidebar);
        sidebarSep1->setObjectName("sidebarSep1");
        sidebarSep1->setGeometry(QRect(0, 71, 220, 1));
        sidebarSep1->setMaximumSize(QSize(16777215, 1));
        sidebarSep1->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 205, 185, 80);"));
        sidebarSep1->setFrameShape(QFrame::Shape::HLine);
        btnParametres = new QPushButton(sidebar);
        btnParametres->setObjectName("btnParametres");
        btnParametres->setEnabled(false);
        btnParametres->setGeometry(QRect(6, 346, 110, 42));
        btnParametres->setMinimumSize(QSize(0, 42));
        btnMatieres = new QPushButton(sidebar);
        btnMatieres->setObjectName("btnMatieres");
        btnMatieres->setGeometry(QRect(6, 215, 150, 41));
        btnFournisseurs = new QPushButton(sidebar);
        btnFournisseurs->setObjectName("btnFournisseurs");
        btnFournisseurs->setEnabled(false);
        btnFournisseurs->setGeometry(QRect(6, 171, 117, 42));
        btnFournisseurs->setMinimumSize(QSize(0, 42));
        btnCommandes = new QPushButton(sidebar);
        btnCommandes->setObjectName("btnCommandes");
        btnCommandes->setEnabled(false);
        btnCommandes->setGeometry(QRect(6, 302, 117, 42));
        btnCommandes->setMinimumSize(QSize(0, 42));
        btnAccueil = new QPushButton(sidebar);
        btnAccueil->setObjectName("btnAccueil");
        btnAccueil->setGeometry(QRect(6, 83, 84, 42));
        btnAccueil->setMinimumSize(QSize(0, 42));
        btnAccueil->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnProduits = new QPushButton(sidebar);
        btnProduits->setObjectName("btnProduits");
        btnProduits->setEnabled(false);
        btnProduits->setGeometry(QRect(6, 258, 92, 42));
        btnProduits->setMinimumSize(QSize(0, 42));
        btnEmployes = new QPushButton(sidebar);
        btnEmployes->setObjectName("btnEmployes");
        btnEmployes->setGeometry(QRect(6, 127, 100, 42));
        btnEmployes->setMinimumSize(QSize(0, 42));
        btnEmployes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        bodyLayout->addWidget(sidebar);

        contentStack = new QStackedWidget(bodyWidget);
        contentStack->setObjectName("contentStack");
        contentStack->setStyleSheet(QString::fromUtf8("  background-color: rgb(243, 233, 215);"));
        pageAccueil = new QWidget();
        pageAccueil->setObjectName("pageAccueil");
        verticalLayout = new QVBoxLayout(pageAccueil);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(40, 40, 40, 40);
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

        comboBoxTri = new QComboBox(pageEmployes);
        comboBoxTri->addItem(QString());
        comboBoxTri->addItem(QString());
        comboBoxTri->addItem(QString());
        comboBoxTri->addItem(QString());
        comboBoxTri->addItem(QString());
        comboBoxTri->addItem(QString());
        comboBoxTri->setObjectName("comboBoxTri");
        comboBoxTri->setMinimumSize(QSize(160, 32));
        comboBoxTri->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 5px 8px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: white;\n"
"}\n"
"QComboBox::drop-down { border: none; }\n"
"QComboBox QAbstractItemView { color: rgb(88, 41, 0); }"));

        employesHeaderRow->addWidget(comboBoxTri);

        comboBoxOrdre = new QComboBox(pageEmployes);
        comboBoxOrdre->addItem(QString());
        comboBoxOrdre->addItem(QString());
        comboBoxOrdre->setObjectName("comboBoxOrdre");
        comboBoxOrdre->setMinimumSize(QSize(100, 32));
        comboBoxOrdre->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"  border: 1px solid rgb(200, 150, 100);\n"
"  border-radius: 4px;\n"
"  padding: 5px 8px;\n"
"  font-size: 12px;\n"
"  color: rgb(88, 41, 0);\n"
"  background-color: white;\n"
"}\n"
"QComboBox::drop-down { border: none; }"));

        employesHeaderRow->addWidget(comboBoxOrdre);


        employesMainLayout->addLayout(employesHeaderRow);

        employesContentLayout = new QHBoxLayout();
        employesContentLayout->setSpacing(14);
        employesContentLayout->setObjectName("employesContentLayout");
        employesLeftColumn = new QVBoxLayout();
        employesLeftColumn->setSpacing(10);
        employesLeftColumn->setObjectName("employesLeftColumn");
        employeeTable = new QTableWidget(pageEmployes);
        if (employeeTable->columnCount() < 13)
            employeeTable->setColumnCount(13);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(10, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(11, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        employeeTable->setHorizontalHeaderItem(12, __qtablewidgetitem12);
        employeeTable->setObjectName("employeeTable");
        employeeTable->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
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
"  padding: 8px 10px;\n"
"  border: none;\n"
"  font-weight: bold;\n"
"  font-size: 11px;\n"
"}"));
        employeeTable->setAlternatingRowColors(true);
        employeeTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        employeeTable->setColumnCount(13);

        employesLeftColumn->addWidget(employeeTable);

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

        lineEditCIN = new QLineEdit(employeeFormBox);
        lineEditCIN->setObjectName("lineEditCIN");

        cinLayout->addWidget(lineEditCIN);


        formRow1->addLayout(cinLayout);

        nomLayout = new QVBoxLayout();
        nomLayout->setSpacing(2);
        nomLayout->setObjectName("nomLayout");
        labelNom = new QLabel(employeeFormBox);
        labelNom->setObjectName("labelNom");

        nomLayout->addWidget(labelNom);

        lineEditNom = new QLineEdit(employeeFormBox);
        lineEditNom->setObjectName("lineEditNom");

        nomLayout->addWidget(lineEditNom);


        formRow1->addLayout(nomLayout);

        prenomLayout = new QVBoxLayout();
        prenomLayout->setSpacing(2);
        prenomLayout->setObjectName("prenomLayout");
        labelPrenom = new QLabel(employeeFormBox);
        labelPrenom->setObjectName("labelPrenom");

        prenomLayout->addWidget(labelPrenom);

        lineEditPrenom = new QLineEdit(employeeFormBox);
        lineEditPrenom->setObjectName("lineEditPrenom");

        prenomLayout->addWidget(lineEditPrenom);


        formRow1->addLayout(prenomLayout);

        sexeLayout = new QVBoxLayout();
        sexeLayout->setSpacing(2);
        sexeLayout->setObjectName("sexeLayout");
        labelSexe = new QLabel(employeeFormBox);
        labelSexe->setObjectName("labelSexe");

        sexeLayout->addWidget(labelSexe);

        comboBoxSexe = new QComboBox(employeeFormBox);
        comboBoxSexe->addItem(QString());
        comboBoxSexe->addItem(QString());
        comboBoxSexe->addItem(QString());
        comboBoxSexe->setObjectName("comboBoxSexe");

        sexeLayout->addWidget(comboBoxSexe);


        formRow1->addLayout(sexeLayout);

        salaireLayout = new QVBoxLayout();
        salaireLayout->setSpacing(2);
        salaireLayout->setObjectName("salaireLayout");
        labelSalaire = new QLabel(employeeFormBox);
        labelSalaire->setObjectName("labelSalaire");

        salaireLayout->addWidget(labelSalaire);

        lineEditSalaire = new QLineEdit(employeeFormBox);
        lineEditSalaire->setObjectName("lineEditSalaire");

        salaireLayout->addWidget(lineEditSalaire);


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

        dateEditEmbauche = new QDateEdit(employeeFormBox);
        dateEditEmbauche->setObjectName("dateEditEmbauche");
        dateEditEmbauche->setCalendarPopup(true);

        dateEmbaucheLayout->addWidget(dateEditEmbauche);


        formRow2->addLayout(dateEmbaucheLayout);

        telephoneLayout = new QVBoxLayout();
        telephoneLayout->setSpacing(2);
        telephoneLayout->setObjectName("telephoneLayout");
        labelTelephone = new QLabel(employeeFormBox);
        labelTelephone->setObjectName("labelTelephone");

        telephoneLayout->addWidget(labelTelephone);

        lineEditTelephone = new QLineEdit(employeeFormBox);
        lineEditTelephone->setObjectName("lineEditTelephone");

        telephoneLayout->addWidget(lineEditTelephone);


        formRow2->addLayout(telephoneLayout);

        posteLayout = new QVBoxLayout();
        posteLayout->setSpacing(2);
        posteLayout->setObjectName("posteLayout");
        labelPoste = new QLabel(employeeFormBox);
        labelPoste->setObjectName("labelPoste");

        posteLayout->addWidget(labelPoste);

        lineEditPoste = new QLineEdit(employeeFormBox);
        lineEditPoste->setObjectName("lineEditPoste");

        posteLayout->addWidget(lineEditPoste);


        formRow2->addLayout(posteLayout);

        adresseLayout = new QVBoxLayout();
        adresseLayout->setSpacing(2);
        adresseLayout->setObjectName("adresseLayout");
        labelAdresse = new QLabel(employeeFormBox);
        labelAdresse->setObjectName("labelAdresse");

        adresseLayout->addWidget(labelAdresse);

        lineEditAdresse = new QLineEdit(employeeFormBox);
        lineEditAdresse->setObjectName("lineEditAdresse");

        adresseLayout->addWidget(lineEditAdresse);


        formRow2->addLayout(adresseLayout);

        emailLayout = new QVBoxLayout();
        emailLayout->setSpacing(2);
        emailLayout->setObjectName("emailLayout");
        labelEmail = new QLabel(employeeFormBox);
        labelEmail->setObjectName("labelEmail");

        emailLayout->addWidget(labelEmail);

        lineEditEmail = new QLineEdit(employeeFormBox);
        lineEditEmail->setObjectName("lineEditEmail");

        emailLayout->addWidget(lineEditEmail);


        formRow2->addLayout(emailLayout);


        formOuterLayout->addLayout(formRow2);

        formBtnLayout = new QHBoxLayout();
        formBtnLayout->setSpacing(10);
        formBtnLayout->setObjectName("formBtnLayout");
        formBtnSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        formBtnLayout->addItem(formBtnSpacer);

        btnAjouter = new QPushButton(employeeFormBox);
        btnAjouter->setObjectName("btnAjouter");
        btnAjouter->setMinimumSize(QSize(110, 36));
        btnAjouter->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAjouter->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        formBtnLayout->addWidget(btnAjouter);

        btnModifier = new QPushButton(employeeFormBox);
        btnModifier->setObjectName("btnModifier");
        btnModifier->setMinimumSize(QSize(110, 36));
        btnModifier->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnModifier->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(255, 205, 185);\n"
"  color: rgb(88, 41, 0);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(230, 180, 150);\n"
"}"));

        formBtnLayout->addWidget(btnModifier);

        btnSupprimer = new QPushButton(employeeFormBox);
        btnSupprimer->setObjectName("btnSupprimer");
        btnSupprimer->setMinimumSize(QSize(110, 36));
        btnSupprimer->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnSupprimer->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(200, 50, 50);\n"
"  color: white;\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(180, 40, 40);\n"
"}"));

        formBtnLayout->addWidget(btnSupprimer);

        btnExportPDF = new QPushButton(employeeFormBox);
        btnExportPDF->setObjectName("btnExportPDF");
        btnExportPDF->setMinimumSize(QSize(120, 36));
        btnExportPDF->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportPDF->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        formBtnLayout->addWidget(btnExportPDF);

        btnExportExcel = new QPushButton(employeeFormBox);
        btnExportExcel->setObjectName("btnExportExcel");
        btnExportExcel->setMinimumSize(QSize(120, 36));
        btnExportExcel->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportExcel->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(255, 205, 185);\n"
"  color: rgb(88, 41, 0);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(230, 180, 150);\n"
"}"));

        formBtnLayout->addWidget(btnExportExcel);

        btnFiltreDisponible = new QPushButton(employeeFormBox);
        btnFiltreDisponible->setObjectName("btnFiltreDisponible");
        btnFiltreDisponible->setMinimumSize(QSize(120, 36));
        btnFiltreDisponible->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnFiltreDisponible->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        formBtnLayout->addWidget(btnFiltreDisponible);

        btnFiltreSeuilCritique = new QPushButton(employeeFormBox);
        btnFiltreSeuilCritique->setObjectName("btnFiltreSeuilCritique");
        btnFiltreSeuilCritique->setMinimumSize(QSize(140, 36));
        btnFiltreSeuilCritique->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnFiltreSeuilCritique->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(255, 205, 185);\n"
"  color: rgb(88, 41, 0);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(230, 180, 150);\n"
"}"));

        formBtnLayout->addWidget(btnFiltreSeuilCritique);

        btnAfficherTous = new QPushButton(employeeFormBox);
        btnAfficherTous->setObjectName("btnAfficherTous");
        btnAfficherTous->setMinimumSize(QSize(120, 36));
        btnAfficherTous->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAfficherTous->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: rgb(88, 41, 0);\n"
"  color: rgb(248, 241, 227);\n"
"  border: none;\n"
"  border-radius: 5px;\n"
"  font-size: 12px;\n"
"  font-weight: bold;\n"
"  padding: 8px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: rgb(110, 55, 10);\n"
"}"));

        formBtnLayout->addWidget(btnAfficherTous);


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
        statsLayout = new QVBoxLayout(statsGroupBox);
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


        statsLayout->addWidget(statCard1);

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


        statsLayout->addWidget(statCard2);

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


        statsLayout->addWidget(statCard3);


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
        calendarWidget = new QCalendarWidget(chatGroupBox);
        calendarWidget->setObjectName("calendarWidget");

        chatLayout->addWidget(calendarWidget);

        chatInputLayout = new QHBoxLayout();
        chatInputLayout->setSpacing(6);
        chatInputLayout->setObjectName("chatInputLayout");

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
        accueilTitle->setText(QCoreApplication::translate("MainWindow", "Bienvenue sur LEATHER HOUSE", nullptr));
        accueilSub->setText(QCoreApplication::translate("MainWindow", "Systeme de gestion integre. Selectionnez un module dans le menu.", nullptr));
        headerUserLabel->setText(QCoreApplication::translate("MainWindow", "Administrateur", nullptr));
        sidebarBrandLabel->setText(QCoreApplication::translate("MainWindow", "  LEATHER HOUSE", nullptr));
        btnParametres->setText(QCoreApplication::translate("MainWindow", "  Parametres", nullptr));
        btnMatieres->setText(QCoreApplication::translate("MainWindow", "  Matieres Premiers", nullptr));
        btnFournisseurs->setText(QCoreApplication::translate("MainWindow", "  Fournisseurs", nullptr));
        btnCommandes->setText(QCoreApplication::translate("MainWindow", "  Commandes", nullptr));
        btnAccueil->setText(QCoreApplication::translate("MainWindow", "  Accueil", nullptr));
        btnProduits->setText(QCoreApplication::translate("MainWindow", "  Produits", nullptr));
        btnEmployes->setText(QCoreApplication::translate("MainWindow", "  Employes", nullptr));
        employesTitle->setText(QCoreApplication::translate("MainWindow", "Gestion Matieres Premiers", nullptr));
        lineEditSearch->setText(QCoreApplication::translate("MainWindow", "Recherche un cuir...", nullptr));
        lineEditSearch->setPlaceholderText(QCoreApplication::translate("MainWindow", "Recherche un cuir...", nullptr));
        btnRechercher->setText(QCoreApplication::translate("MainWindow", "Rechercher", nullptr));
        comboBoxTri->setItemText(0, QCoreApplication::translate("MainWindow", "Trier par...", nullptr));
        comboBoxTri->setItemText(1, QCoreApplication::translate("MainWindow", "Nom (A\342\206\222Z)", nullptr));
        comboBoxTri->setItemText(2, QCoreApplication::translate("MainWindow", "Gamme", nullptr));
        comboBoxTri->setItemText(3, QCoreApplication::translate("MainWindow", "\303\211paisseur", nullptr));
        comboBoxTri->setItemText(4, QCoreApplication::translate("MainWindow", "Quantit\303\251 stock", nullptr));
        comboBoxTri->setItemText(5, QCoreApplication::translate("MainWindow", "Type de cuir", nullptr));

        comboBoxOrdre->setItemText(0, QCoreApplication::translate("MainWindow", "Croissant", nullptr));
        comboBoxOrdre->setItemText(1, QCoreApplication::translate("MainWindow", "D\303\251croissant", nullptr));

        QTableWidgetItem *___qtablewidgetitem = employeeTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = employeeTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Refference", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = employeeTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Nom du cuir", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = employeeTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Type de cuir", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = employeeTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Gamme", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = employeeTable->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Couleur", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = employeeTable->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Epaisseur", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = employeeTable->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Origine", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = employeeTable->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Fournisseur associe", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = employeeTable->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "Quantite de stock", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = employeeTable->horizontalHeaderItem(10);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "Prix", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = employeeTable->horizontalHeaderItem(11);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "Date d'achat", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = employeeTable->horizontalHeaderItem(12);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "Status ", nullptr));
        employeeFormBox->setTitle(QCoreApplication::translate("MainWindow", "Fiche Cuir", nullptr));
        labelCIN->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        lineEditCIN->setText(QString());
        lineEditCIN->setPlaceholderText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        labelNom->setText(QCoreApplication::translate("MainWindow", "Refference", nullptr));
        lineEditNom->setText(QString());
        lineEditNom->setPlaceholderText(QCoreApplication::translate("MainWindow", "R\303\251f\303\251rence interne", nullptr));
        labelPrenom->setText(QCoreApplication::translate("MainWindow", "Nom de cuir", nullptr));
        lineEditPrenom->setText(QString());
        lineEditPrenom->setPlaceholderText(QCoreApplication::translate("MainWindow", "Nom du cuir", nullptr));
        labelSexe->setText(QCoreApplication::translate("MainWindow", "Gamme", nullptr));
        comboBoxSexe->setItemText(0, QCoreApplication::translate("MainWindow", "Supreme", nullptr));
        comboBoxSexe->setItemText(1, QCoreApplication::translate("MainWindow", "Midium", nullptr));
        comboBoxSexe->setItemText(2, QCoreApplication::translate("MainWindow", "Economy", nullptr));

        labelSalaire->setText(QCoreApplication::translate("MainWindow", "Type de cuir", nullptr));
        lineEditSalaire->setText(QString());
        lineEditSalaire->setPlaceholderText(QCoreApplication::translate("MainWindow", "Type de cuir", nullptr));
        labelDateEmbauche->setText(QCoreApplication::translate("MainWindow", "Date d'achat", nullptr));
        dateEditEmbauche->setDisplayFormat(QCoreApplication::translate("MainWindow", "dd/MM/yyyy", nullptr));
        labelTelephone->setText(QCoreApplication::translate("MainWindow", "Couleur", nullptr));
        lineEditTelephone->setText(QString());
        lineEditTelephone->setPlaceholderText(QCoreApplication::translate("MainWindow", "Couleur", nullptr));
        labelPoste->setText(QCoreApplication::translate("MainWindow", "Qte de stock", nullptr));
        lineEditPoste->setText(QString());
        lineEditPoste->setPlaceholderText(QCoreApplication::translate("MainWindow", "Quantit\303\251 stock", nullptr));
        labelAdresse->setText(QCoreApplication::translate("MainWindow", "Epaisseur", nullptr));
        lineEditAdresse->setText(QString());
        lineEditAdresse->setPlaceholderText(QCoreApplication::translate("MainWindow", "\303\211paisseur (mm)", nullptr));
        labelEmail->setText(QCoreApplication::translate("MainWindow", "Origine", nullptr));
        lineEditEmail->setText(QString());
        lineEditEmail->setPlaceholderText(QCoreApplication::translate("MainWindow", "Origine", nullptr));
        btnAjouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btnModifier->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnSupprimer->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        btnExportPDF->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        btnExportExcel->setText(QCoreApplication::translate("MainWindow", "Export Excel", nullptr));
        btnFiltreDisponible->setText(QCoreApplication::translate("MainWindow", "Disponibles", nullptr));
        btnFiltreSeuilCritique->setText(QCoreApplication::translate("MainWindow", "Seuil critique", nullptr));
        btnAfficherTous->setText(QCoreApplication::translate("MainWindow", "Afficher tous", nullptr));
        statsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        statTitle1->setText(QCoreApplication::translate("MainWindow", "Quantite achete", nullptr));
        statTitle2->setText(QCoreApplication::translate("MainWindow", "Cuir utilise", nullptr));
        statTitle3->setText(QCoreApplication::translate("MainWindow", "Cuir restant", nullptr));
        chatGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Busy season tracker", nullptr));
        footerLabel->setText(QCoreApplication::translate("MainWindow", "\302\251 2025 LEATHER HOUSE  |  Version 1.0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
