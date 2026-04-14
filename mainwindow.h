
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSqlQuery>
#include <QPair>
#include <QStringList>
#include <QVector>
#include "produit.h"

                                                   class ApiClient;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAjouter_6_clicked();
    void on_btnModifier_4_clicked();
    void on_btnSupprimer_4_clicked();
    void on_btnRechercher_4_clicked();
    void on_pushButton_6_clicked();   // voir alerte (stock 0)
    void on_pushButton_7_clicked();   // recherche vocale (texte dans textEdit_2)
    void on_pushButton_8_clicked();   // chatbot (lineEdit_19 -> textEdit_3)

    void remplirFormulaire(int row, int col);   // ✅ ajoutée

private:
    Ui::MainWindow *ui;
    ApiClient *apiClient = nullptr;
    Produit produit;
    int selectedProduitId = -1;
    QLabel *validationLabel = nullptr;
    QStringList chatHistoryTurns;
    int maxChatHistoryTurns = 16;
    QVector<QPair<QString, QString>> questionAnswerHistory;

    void afficherProduits(const QString &texteFiltre = QString());   // filtre vide = tout afficher
    void remplirTableProduitsDepuisRequete(QSqlQuery &query);
    void rechercherProduitParNomOuId(const QString &texteBrut);
    QPair<bool, QString> transcrireVoixWindows();
    QString construireReponseLocaleAvancee(const QString &question);
    QString construireReponseChatbot(const QString &entree);
    QString construireContexteCataloguePourApi();
    QString trouverMeilleurMatch(const QString &transcrit);
    bool envoyerEmailAlerteStockSiConfigure(const QString &corpsTexte, int nombreProduits, QString &outInfo);
    void viderFormulaire();
    bool validerFormulaire(int &outId, int &outQte, QString &outErrorMsg, QWidget *&outInvalidWidget);
    void cacherErreurSaisie();
    void afficherErreurSaisie(QWidget *champ, const QString &message);
};

#endif

