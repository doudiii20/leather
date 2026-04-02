#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("DRIVER={Oracle in XE};DBQ=127.0.0.1:1521/XE;Uid=ranim;Pwd=107108;");
    if (!db.open()) {
        qDebug() << "FAILED:" << db.lastError().text();
    } else {
        qDebug() << "SUCCESS!";
    }
    return 0;
}
