#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QPushButton>
#include <QStackedWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QLabel>
#include <QImage>
#include <QHash>
#include <QPixmap>
#include <QString>
#include <QPointF>
#include <QVariantMap>
#include "clientdata.h"
#include "produit.h"
#include "matierepremiere.h"
#include "employe.h"
#include "recommendationservice.h"
#include "fournisseurstats.h"
#include "supplierchartwidgets.h"
#include "GoogleCalendarService.h"

#include <QVector>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QChart>
#include <QChartView>
#include <QBarSet>
#include <QPieSeries>
#else
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class FournisseurManager;
class ChatbotService;
class FournisseurApiService;
class ClientNotificationService;
class WhatsAppBusinessService;
class QScrollArea;
class QTextEdit;
class QLineEdit;
class QDialog;
class AssistantWindow;
class ChatbotWindow;
class SettingsWindow;
class QTranslator;
class QTextBrowser;
class QTimer;
class QComboBox;
class QCheckBox;
class QProgressBar;
class QListWidget;

class QEvent;
class QFrame;
class QCalendarWidget;
#ifdef LEATHER_HAVE_WEBENGINE
class QWebEngineView;
#endif

struct Fournisseur {
    QString nom;
    double latitude = 0.0;
    double longitude = 0.0;
    int delai = 0;
    int fiabilite = 0;
    int nb_commandes = 0;
    double apiDistanceKm = -1.0;
    double apiDurationMin = -1.0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    struct SmartMapPointInfo;
    Ui::MainWindow *ui;
    QSqlDatabase db;  // objet pour gérer la connexion Oracle
    QNetworkAccessManager *m_networkAccessManager = nullptr;
    QTranslator *m_appTranslator = nullptr;
    FournisseurManager *gestionFournisseur = nullptr;

    // Méthodes internes
    void connectSidebar();                 // navigation (barre du haut)
    void setActiveButton(QPushButton *active);  // bouton actif
    bool connectToDatabase(bool showErrorDialog = true); ///< Connexion ODBC Oracle (connexion par défaut Qt).
    void setupClientUiEnhancements();
    void applyFormStyle(QWidget *formRoot);
    void setupClientFicheScrollAndHeader();
    void setupClientAnalyseSuiviScroll();
    void setupClientValidators();
    void beginClientEditMode();
    void exitClientEditMode(bool restoreSnapshot);
    ClientData currentClientSnapshotFromForm() const;
    void loadClients();
    void onAnalyzeClientsClicked();
    void applyClientScoringResults(const QHash<int, int> &scoresByClientId,
                                   const QHash<int, QString> &statusByClientId);
    bool isNumverifyPhoneValid(const QString &phone, bool *isValid, QString *errorMessage) const;
    void fillClientFormFromSelectedRow();
    void clearClientForm();
    bool readCurrentClientForm(ClientData &out) const;
    void updateAiInsightsPanel(const ClientData &client);
    bool validateClientFormInputs(bool isUpdate);
    void setupFournisseurDashboardBlock();
    void refreshFournisseurDashboard();
    void setupSmartMapFournisseursUi();
    void refreshSmartMapFournisseurs();
    void analyserFournisseurs(QVector<Fournisseur> list);
    void renderSmartMapFournisseurs(const QVector<Fournisseur> &list);
    void renderSmartMapOnCanvas(QLabel *canvas, const QVector<Fournisseur> &list, QVector<SmartMapPointInfo> *pointInfos);
    QPointF smartMapCoordToPixel(double latitude, double longitude, int width, int height) const;
    void applyRouteApiMetrics(QVector<Fournisseur> &list, QString *statusMessage);
    void geocodeAdresse(QString adresse, int fournisseurId);
    void geocodePendingFournisseurs();
    void openMap();
    QList<QVariantMap> getFournisseurs() const;
    QList<QVariantMap> getFournisseursNomAdresse() const;
    void loadFournisseursLeafletMap();
    void injectFournisseursLeafletMarkers();
    void showSmartMapFournisseursDialog();
    void refreshClientRecommendations();

    QLabel *m_fournisseurStatsSummary = nullptr;
    QWidget *m_fournisseurStatsChartsContainer = nullptr;
    SupplierBarChartWidget *m_fournisseurBarChart = nullptr;
    SupplierPieChartWidget *m_fournisseurPieChart = nullptr;
    QComboBox *m_fournisseurSmartFilter = nullptr;
    QLabel *m_fournisseurPlusProcheLabel = nullptr;
    QLabel *m_fournisseurPlusRapideLabel = nullptr;
    QLabel *m_fournisseurPlusFiableLabel = nullptr;
    QLabel *m_fournisseurMeilleurLabel = nullptr;
    QLabel *m_fournisseurSmartMapCanvas = nullptr;
    QLabel *m_fournisseurSmartMapDialogCanvas = nullptr;
    QPushButton *m_fournisseurOpenSmartMapButton = nullptr;
    QVector<Fournisseur> m_fournisseurSmartMapPoints;
    QString m_fournisseurSmartFilterSelection;
    bool m_fournisseurSmartFilterUpdating = false;
    QLabel *m_fournisseurAlertesLabel = nullptr;
    QString m_fournisseurSmartMissingCoordsMessage;
    QString m_fournisseurSmartApiStatusMessage;
    struct SmartMapPointInfo {
        QPoint center;
        int radius = 0;
        QString tooltip;
    };
    QVector<SmartMapPointInfo> m_fournisseurSmartPointInfos;
#ifdef LEATHER_HAVE_WEBENGINE
    QDialog *m_fournisseurMapDialog = nullptr;
    QWebEngineView *m_fournisseurMapView = nullptr;
    bool m_fournisseurMapHtmlLoaded = false;
#endif

    ChatbotService *m_chatbotService = nullptr;
    FournisseurApiService *m_fournisseurApiService = nullptr;
    WhatsAppBusinessService *m_whatsappBusinessService = nullptr;
    /// Numero E.164 affiche dans le message de succes apres envoi WhatsApp (depuis onClientWhatsAppClicked).
    QString m_lastWhatsAppDestinationDisplay;
    AssistantWindow *m_assistantWindow = nullptr;
    ChatbotWindow *m_globalChatbotWindow = nullptr;
    /// OAuth 2.0 + Google Calendar API (voir GoogleCalendarService.cpp).
    GoogleCalendarService *m_googleCalendarService = nullptr;
    QListWidget *m_googleCalendarEventsList = nullptr;
    QCalendarWidget *m_googleCalendarWidget = nullptr;
    QVector<GoogleCalendarEventRow> m_googleCalendarLastRows;
    /// Réponse ChatbotService : client vs produits vs fournisseurs (un seul service, on route la sortie).
    enum class ChatbotSink { Client, Produit, Fournisseur };
    ChatbotSink m_chatbotSink = ChatbotSink::Client;
    QTextEdit *m_clientRecoDisplay = nullptr;
    QPushButton *m_clientWhatsAppButton = nullptr;
    QPushButton *m_clientSurveySmsButton = nullptr;
    QScrollArea *m_clientFormScrollArea = nullptr;
    QList<RecommendationItem> m_lastRecommendations;
    bool m_clientEditMode = false;
    int m_clientCurrentRowId = -1;
    ClientData m_clientEditSnapshot;
    QPushButton *m_clientCancelEditButton = nullptr;

    int m_produitSelectedId = -1;
    bool m_produitEditMode = false;
    int m_produitEditingId = -1;
    QPushButton *m_produitCancelEditButton = nullptr;
    QPushButton *m_produitVoirQrButton = nullptr;
    QLabel *m_produitQrLabel = nullptr;
    QString m_lastProduitQrPath;
    QHash<QString, QPixmap> m_produitQrPixmapCache;
    bool m_produitUiWired = false;
    bool m_isAuthenticated = false;

    int m_matiereSelectedId = -1;
    bool m_matiereEditMode = false;
    int m_matiereEditingId = -1;
    QPushButton *m_matiereCancelEditButton = nullptr;
    bool m_matiereUiWired = false;
    bool m_employeEditMode = false;
    QString m_employeEditingCin;
    QPushButton *m_employeCancelEditButton = nullptr;
    bool m_employeUiWired = false;
    QPushButton *m_employeExportExcelBtn = nullptr;
    /// 0 = menu principal (hub), 1 = interface avec barre de navigation + contentStack
    QStackedWidget *m_bodyStack = nullptr;
    /// Filtres vue matières (combinés avec la recherche texte).
    bool m_mpFilterDisponible = false;
    int m_mpSeuilCritique = -1; ///< -1 = désactivé, sinon afficher les lignes avec réserve <= valeur.

    void applyMatieresViewFilters();
    void applyMatieresTableSortIfNeeded();

    /// Reconnexion ODBC si besoin (même connexion que clients / commerce).
    bool ensureDbOpenForProduits();
    /// Remplace le positionnement absolu Designer (page produits) par un layout qui remplit la largeur.
    void installProduitsPageResponsiveLayout();
    void installMatieresPageResponsiveLayout();
    void installEmployesPageClientLikeLayout();
    void installFournisseursPageClientLikeLayout();
    void installMatieresPageClientLikeLayout();
    /// Page clients : même structure que matières / fournisseurs (marges, tableau + fiche + panneau droit).
    void installClientPageResponsiveLayout();
    void setupProduitPage();
    void refreshProduitsTable();
    void afficherQR();
    void showProduitQrDialog(int row);
    QImage generateQRCode(const QString &data) const;
    QString buildProduitQrPayload(int row) const;
    QPixmap generateProduitQrPixmap(const QString &payload, int size, QString *errorMessage = nullptr) const;
    bool ensureProduitQrSchema(QString *errorMessage = nullptr) const;
    bool saveProduitQrPathInDb(int produitId, const QString &qrPath, QString *errorMessage = nullptr) const;
    QString loadProduitQrPathFromDb(int produitId) const;
    void setProduitQrPreview(const QString &qrPath);
    void fillProduitFormFromTableRow(int row);
    void clearProduitForm();
    /// Raccourcis widgets fiche produit (delegues a Produit::* dans produit.cpp).
    ProduitEditorWidgets produitEditorBindings() const;

    void setupMatierePage();
    void setupEmployePage();
    void openEmployesModule();
    void refreshEmployesTable();
    void updateEmployesStatsPanel();
    void clearEmployeForm();
    void setEmployeFormReadOnlyCin(bool readOnly);
    EmployeEditorWidgets employeEditorBindings() const;
    void refreshMatieresTable();
    void fillMatiereFormFromTableRow(int row);
    void clearMatiereForm();
    MatierePremiereEditorWidgets matiereEditorBindings() const;
    bool readMatiereFromForm(MatierePremiere &out) const;
    void updateMatieresStatsPanel();
    QString buildFournisseurAiContext() const;
    void onFournisseurChatbotRequested();
    void onFournisseurAiRequested();
    void applyAuthenticationUiState(bool authenticated);
    bool ensureAuthSchema(QString *errorMessage = nullptr);
    bool ensureDefaultAdminUser(QString *errorMessage = nullptr);
    bool validateLoginCredentials(const QString &username, const QString &password);
    QString hashPassword(const QString &plain) const;
    void onForgotPasswordRequested();
    void onOpenAssistantRequested();
    void setupHubAndBodyStack();
    void updateShellChromeVisibility();
    void setupLoginPageChrome();
    void refreshLoginFieldFocusHighlight();
    void applyGlobalAppearanceFromSettings();
    void installApplicationTranslators();
    void refreshFaceLoginButtonVisibility();
    void finishFaceIdLoginSession();
    void applyProfileDisplayFromSettings();
    void onPasswordChangeFromSettings(const QString &username, const QString &oldPassword, const QString &newPassword);
    void setupAdministratorDashboard();
    void refreshAdministratorDashboard();
    void onStatisticsDataChanged();
    void loadLineChart();
    void loadBarChart();
    void loadPieChart();
    void applyDashboardPresentation();
    void applyDashboardChartColors(bool dark);
    Qt::Alignment dashboardLegendAlignment() const;

    QFrame *m_loginUserFieldBox = nullptr;
    QFrame *m_loginPassFieldBox = nullptr;

    bool m_adminDashboardSetup = false;
    QWidget *m_dashContentRoot = nullptr;
    QLabel *m_dashKpiEmployes = nullptr;
    QLabel *m_dashKpiFournisseurs = nullptr;
    QLabel *m_dashKpiProduits = nullptr;
    QLabel *m_dashKpiClients = nullptr;
    QLabel *m_dashKpiMatieres = nullptr;
    QLabel *m_dashCatalogueStats = nullptr;
    QLabel *m_dashCatalogueTopCategories = nullptr;
    QProgressBar *m_dashCatalogueAvailableBar = nullptr;
    QProgressBar *m_dashCatalogueLowBar = nullptr;
    QProgressBar *m_dashCatalogueOutBar = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QChart *m_dashBarChart = nullptr;
    QChartView *m_dashBarChartView = nullptr;
    QPieSeries *m_dashPieSeries = nullptr;
    QChart *m_dashPieChart = nullptr;
    QChartView *m_dashPieChartView = nullptr;
    QChart *m_dashOrdersBarChart = nullptr;
    QChartView *m_dashOrdersBarChartView = nullptr;
#else
    QtCharts::QChart *m_dashBarChart = nullptr;
    QtCharts::QChartView *m_dashBarChartView = nullptr;
    QtCharts::QPieSeries *m_dashPieSeries = nullptr;
    QtCharts::QChart *m_dashPieChart = nullptr;
    QtCharts::QChartView *m_dashPieChartView = nullptr;
    QtCharts::QChart *m_dashOrdersBarChart = nullptr;
    QtCharts::QChartView *m_dashOrdersBarChartView = nullptr;
#endif
    QTimer *m_statsAutoRefreshTimer = nullptr;
    QComboBox *m_dashThemeCombo = nullptr;
    QCheckBox *m_dashAnimCheck = nullptr;
    QComboBox *m_dashLegendCombo = nullptr;
    QLabel *m_dashPageTitle = nullptr;
    QLabel *m_dashPageSubtitle = nullptr;
    QFrame *m_dashCardBar = nullptr;
    QFrame *m_dashCardPie = nullptr;
    QFrame *m_dashCardLine = nullptr;

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void sendStockAlert();
    void loadStats();
    void loadStockFaible();
    void loadPrediction();
    void on_btnAjouter_8_clicked();
    void onFaceLoginRequested();
    void onGlobalChatbotClicked();
    void on_btnSignUp_clicked();
    void on_btnMpremieres_2_clicked();
    void on_btnSettings_clicked();
    void on_pushButton_ajouter_clicked();
    void on_pushButton_supprimer_clicked();
    void on_pushButton_modifier_clicked();
    void on_btnRechercher_3_clicked();
    void on_pushButton_resetFiltres_clicked();
    void on_clientTable_cellClicked(int row, int column);
    void onVerifierCommandeClicked();
    void onEnregistrerPaiementClicked();
    void onVoirHistoriqueClicked();
    void onExporterClientsClicked();
    void onExporterMatieresPremieresClicked();
    void onClientSurveySmsClicked();
    void onClientWhatsAppClicked();
    void on_employeeTable_4_cellClicked(int row, int column);
    void on_produitTable_cellClicked(int row, int column);
    void on_btnAjouter_6_clicked();
    void on_btnModifier_4_clicked();
    void on_btnSupprimer_4_clicked();
    void on_btnRechercher_4_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_7_clicked();
    void on_employeeTable_5_cellClicked(int row, int column);
    void on_btnAjouter_7_clicked();
    void on_btnModifier_5_clicked();
    void on_btnSupprimer_5_clicked();
    void on_btnRechercher_5_clicked();
    void on_btnExportPDF_2_clicked();
    void on_btnExportExcel_clicked();
    void on_btnFiltreDisponible_clicked();
    void on_btnFiltreSeuilCritique_clicked();
    void on_btnAfficherTous_clicked();
    void onOpenBusySeasonCalendar();
    /// Boutons page Matières premières : OAuth Google Calendar (voir installMatieresPageClientLikeLayout dans mainwindow.cpp).
    void connectToGoogle();
    void syncCalendar();
    void onGoogleCalendarEventsReady(const QVector<GoogleCalendarEventRow> &rows);
    void onGoogleEventCreated(bool success, const QString &detail);
    void on_comboBoxTri_mp_currentIndexChanged(int index);
    void on_comboBoxOrdre_mp_currentIndexChanged(int index);

    void applyGoogleCalendarMonthHighlights();

    void onEmployeAjouterClicked();
    void onEmployeModifierClicked();
    void onEmployeSupprimerClicked();
    void onEmployeRechercherClicked();
    void onEmployeSearchTextChanged(const QString &text);
    void onEmployeSelectionChanged();
    void onEmployeExportPdfClicked();
    void onEmployeExportExcelClicked();
    void onEmployeSendChatClicked();
};

#endif // MAINWINDOW_H
