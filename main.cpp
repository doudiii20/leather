#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QSqlDatabase>
#include <QDebug>
#include "matieredao.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont appFont("Segoe UI", 10);
    a.setFont(appFont);

    // ✅ CONNECT DATABASE FIRST
    if (!MatiereDAO::connecterOracle("ranim", "107108", "localhost", 1521, "XE")) {
        qDebug() << "Database connection failed!";
    }

    MainWindow w;
    w.show();

    qDebug() <<QSqlDatabase::drivers();
    return a.exec();
}
