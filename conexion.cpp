#include "conexion.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

conexion::conexion()
{
}

void conexion::createconnect()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Projet_2A");
    db.setUserName("nour");
    db.setPassword("nour");

    if(db.open())
        qDebug() << "Connexion OK";
    else
        qDebug() << db.lastError().text();
}
