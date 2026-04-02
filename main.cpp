#include "mainwindow.h"
#include "conexion.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    conexion c;
    c.createconnect();

    MainWindow w;
    w.show();
    return a.exec();
}
