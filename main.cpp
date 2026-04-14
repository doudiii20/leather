#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // استعمال QODBC
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    // 🔵 نستعمل اسم الـ DSN كما هو موجود في ODBC
    db.setDatabaseName("projet_cuir");

    // نحط اليوزر والباس (حتى لو موجودين في DSN)
    db.setUserName("moufida");
    db.setPassword("moufida");

    qDebug() << "Drivers disponibles:" << QSqlDatabase::drivers();

    if(!db.open())
    {
        QMessageBox::critical(nullptr,
                              "Erreur connexion Oracle",
                              db.lastError().text());
        return -1;
    }

    qDebug() << "Connexion Oracle réussie ✔";

    MainWindow w;
    w.show();

    return a.exec();
}
