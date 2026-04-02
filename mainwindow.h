#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_btnAjouter_clicked();
    void on_btnModifier_clicked();
    void on_btnSupprimer_clicked();
    void on_btnRechercher_clicked();
    void on_btnExportPDF_clicked();
    void on_btnExportExcel_clicked();
    void onTableSelectionChanged();
    void onHeaderClicked(int logicalIndex);
    void on_comboBoxSort_currentIndexChanged(int index);

    // Nouveaux slots pour la navigation et l'IA
    void on_btnAccueil_clicked();
    void on_btnEmployes_clicked();
    void on_btnSendChat_clicked();

private:
    Ui::MainWindow *ui;
    void refreshTable();
    void applySearchFilter(const QString &text);
    void updateStats();
    void clearChamps();
    bool champsValides();
    int lastSortColumn = -1;
    bool sortAsc = true;
};

#endif
