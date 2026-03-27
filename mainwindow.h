#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "produit.h"

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

    void remplirFormulaire(int row, int col);   // ✅ ajoutée

private:
    Ui::MainWindow *ui;
    Produit produit;
    int selectedProduitId = -1;
    QLabel *validationLabel = nullptr;

    void afficherProduits();   // ✅ ajoutée
    void viderFormulaire();
    bool validerFormulaire(int &outId, int &outQte, QString &outErrorMsg, QWidget *&outInvalidWidget);
    void cacherErreurSaisie();
    void afficherErreurSaisie(QWidget *champ, const QString &message);
};

#endif
