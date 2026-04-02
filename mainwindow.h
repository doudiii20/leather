#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QList>
#include "matierepremiere.h"
#include "matieredao.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- CRUD ---
    void on_btnAjouter_clicked();
    void on_btnModifier_clicked();
    void on_btnSupprimer_clicked();
    void on_btnRechercher_clicked();

    // --- Tri ---
    void on_comboBoxTri_currentIndexChanged(int index);
    void on_comboBoxOrdre_currentIndexChanged(int index);

    // --- Filtres ---
    void on_btnFiltreDisponible_clicked();
    void on_btnFiltreSeuilCritique_clicked();
    void on_btnAfficherTous_clicked();

    // --- Export ---
    void on_btnExportPDF_clicked();
    void on_btnExportExcel_clicked();

    // --- Selection ---
    void on_employeeTable_itemSelectionChanged();

private:
    void connectSidebar();
    void setActiveButton(QPushButton *active);
    void connecterBD();
    void chargerTableau(const QList<MatierePremiere> &liste);
    void viderFormulaire();
    MatierePremiere lireFormulaire();
    void remplirFormulaire(const MatierePremiere &m);
    bool validerFormulaire();
    void rafraichirStats();

    Ui::MainWindow *ui;
    MatiereDAO m_dao;
    QList<MatierePremiere> m_listeCourante;
};

#endif // MAINWINDOW_H
