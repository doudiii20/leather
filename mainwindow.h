#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QLabel>
#include "clientdata.h"
#include "recommendationservice.h"
#include "fournisseurstats.h"
#include "supplierchartwidgets.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FournisseurManager;
class ChatbotService;
class QTextEdit;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;  // objet pour gérer la connexion Oracle
    QNetworkAccessManager *m_networkAccessManager = nullptr;
    FournisseurManager *gestionFournisseur = nullptr;

    // Méthodes internes
    void connectSidebar();                 // gérer la sidebar
    void setActiveButton(QPushButton *active);  // bouton actif
    bool connectToDatabase();              // connexion Oracle
    void setupClientUiEnhancements();
    void setupClientValidators();
    void loadClients();
    void fillClientFormFromSelectedRow();
    void clearClientForm();
    void updateAiInsightsPanel(const ClientData &client);
    bool validateClientFormInputs(bool isUpdate);
    void setupFournisseurDashboardBlock();
    void refreshFournisseurDashboard();
    void setupClientIntelligencePanel();
    void refreshClientRecommendations();

    QLabel *m_fournisseurStatsSummary = nullptr;
    QPushButton *m_fournisseurStatsToggle = nullptr;
    QWidget *m_fournisseurStatsChartsContainer = nullptr;
    SupplierBarChartWidget *m_fournisseurBarChart = nullptr;
    SupplierPieChartWidget *m_fournisseurPieChart = nullptr;

    ChatbotService *m_chatbotService = nullptr;
    QTextEdit *m_clientRecoDisplay = nullptr;
    QTextEdit *m_clientChatLog = nullptr;
    QLineEdit *m_clientChatInput = nullptr;
    QPushButton *m_clientChatSendBtn = nullptr;
    QPushButton *m_demoOrderButton = nullptr;
    QList<RecommendationItem> m_lastRecommendations;

private slots:
    void on_pushButton_ajouter_clicked();
    void on_pushButton_supprimer_clicked();
    void on_pushButton_modifier_clicked();
    void on_btnRechercher_3_clicked();
    void on_pushButton_resetFiltres_clicked();
    void on_clientTable_cellClicked(int row, int column);
    void onRecalculerSegmentationClicked();
    void onVerifierCommandeClicked();
    void onEnregistrerPaiementClicked();
    void onVoirHistoriqueClicked();
    void onExporterClientsClicked();
    void onDemoTopProductOrderClicked();
    void onClientChatSendRequested();
};

#endif // MAINWINDOW_H
