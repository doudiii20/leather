#include "matieredao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QDebug>
#include <QDateTime>
#include <QPageSize>
#include <QPageLayout>

MatiereDAO::MatiereDAO() {}

bool MatiereDAO::connecterOracle(const QString &username,
                                 const QString &password,
                                 const QString &host,
                                 int port,
                                 const QString &sid)
{
    Q_UNUSED(host); Q_UNUSED(port); Q_UNUSED(sid);

    QSqlDatabase db;

    if (QSqlDatabase::contains("qt_oracle_connection")) {
        db = QSqlDatabase::database("qt_oracle_connection");
        if (db.isOpen()) return true;
    } else {
        db = QSqlDatabase::addDatabase("QODBC", "qt_oracle_connection");
    }

    // Utilisation d'une chaîne Driver directe au lieu d'un DSN "capricieux"
    QString connexionId = QString("Driver={Oracle in XE};dbq=127.0.0.1:1521/XE;uid=%1;pwd=%2;").arg(username, password);
    db.setDatabaseName(connexionId);

    if (!db.open()) {
        qDebug() << "[Oracle] Connexion echouee:" << db.lastError().text();
        qDebug() << "[Oracle] Drivers disponibles:" << QSqlDatabase::drivers();
        return false;
    }

    qDebug() << "[Oracle] Connexion reussie en tant que" << username;
    return true;
}

bool MatiereDAO::isConnected()
{
    if (!QSqlDatabase::contains("qt_oracle_connection")) return false;
    return QSqlDatabase::database("qt_oracle_connection").isOpen();
}

MatierePremiere MatiereDAO::fromQuery(const QSqlQuery &q)
{
    MatierePremiere m;
    m.setId(q.value("ID").toInt());
    m.setReference(q.value("REFERENCE_INTERNE").toString());
    m.setNomCuir(q.value("NOM_CUIR").toString());
    m.setTypeCuir(q.value("TYPE_CUIR").toString());
    m.setGamme(q.value("GAME").toString());
    m.setCouleur(q.value("COULEUR").toString());
    m.setStatut(q.value("STATUT").toString());
    m.setEpaisseur(q.value("EPAISSEUR").toDouble());
    m.setOrigine(q.value("ORIGINE").toString());
    m.setReserve(q.value("RESERVE").toInt());
    return m;
}

QList<MatierePremiere> MatiereDAO::executerRequete(const QString &sql,
                                                    const QVariantList &params)
{
    QList<MatierePremiere> liste;
    if (!isConnected()) { qDebug() << "[SQL] Non connecte."; return liste; }

    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    q.prepare(sql);
    for (const QVariant &v : params) q.addBindValue(v);

    if (!q.exec()) {
        qDebug() << "[SQL Error]" << q.lastError().text();
        return liste;
    }
    while (q.next()) liste << fromQuery(q);
    return liste;
}

bool MatiereDAO::ajouter(const MatierePremiere &m)
{
    if (m.getNomCuir().trimmed().isEmpty() || m.getReference().trimmed().isEmpty())
        return false;

    QString sql = "INSERT INTO MATIERES_PREMIERES "
                  "(ID, REFERENCE_INTERNE, NOM_CUIR, TYPE_CUIR, GAME, "
                  " COULEUR, STATUT, EPAISSEUR, ORIGINE, RESERVE) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    q.prepare(sql);
    q.addBindValue(m.getId());
    q.addBindValue(m.getReference());
    q.addBindValue(m.getNomCuir());
    q.addBindValue(m.getTypeCuir());
    q.addBindValue(m.getGamme());
    q.addBindValue(m.getCouleur());
    q.addBindValue(m.getStatut().isEmpty() ? QString("Disponible") : m.getStatut());
    q.addBindValue(m.getEpaisseur());
    q.addBindValue(m.getOrigine());
    q.addBindValue(m.getReserve());

    if (!q.exec()) { qDebug() << "[Ajouter]" << q.lastError().text(); return false; }
    return true;
}

bool MatiereDAO::modifier(const MatierePremiere &m)
{
    QString sql = "UPDATE MATIERES_PREMIERES "
                  "SET REFERENCE_INTERNE=?, NOM_CUIR=?, TYPE_CUIR=?, GAME=?, "
                  "    COULEUR=?, STATUT=?, EPAISSEUR=?, ORIGINE=?, RESERVE=? "
                  "WHERE ID=?";

    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    q.prepare(sql);
    q.addBindValue(m.getReference());
    q.addBindValue(m.getNomCuir());
    q.addBindValue(m.getTypeCuir());
    q.addBindValue(m.getGamme());
    q.addBindValue(m.getCouleur());
    q.addBindValue(m.getStatut().isEmpty() ? QString("Disponible") : m.getStatut());
    q.addBindValue(m.getEpaisseur());
    q.addBindValue(m.getOrigine());
    q.addBindValue(m.getReserve());
    q.addBindValue(m.getId());

    if (!q.exec()) { qDebug() << "[Modifier]" << q.lastError().text(); return false; }
    return true;
}

bool MatiereDAO::supprimer(int id)
{
    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    q.prepare("DELETE FROM MATIERES_PREMIERES WHERE ID=?");
    q.addBindValue(id);
    if (!q.exec()) { qDebug() << "[Supprimer]" << q.lastError().text(); return false; }
    return true;
}

QList<MatierePremiere> MatiereDAO::afficherTous()
{
    return executerRequete("SELECT * FROM MATIERES_PREMIERES ORDER BY ID");
}

QList<MatierePremiere> MatiereDAO::trierPar(const QString &champ, const QString &ordre)
{
    QStringList valides = {"ID","NOM_CUIR","TYPE_CUIR","GAME","COULEUR",
                           "EPAISSEUR","RESERVE","ORIGINE","STATUT"};
    QString c = champ.toUpper();
    QString o = (ordre.toUpper() == "DESC") ? "DESC" : "ASC";
    if (!valides.contains(c)) return afficherTous();
    return executerRequete(
        QString("SELECT * FROM MATIERES_PREMIERES ORDER BY %1 %2").arg(c).arg(o));
}

QList<MatierePremiere> MatiereDAO::rechercher(const QString &motCle)
{
    QString p = "%" + motCle + "%";
    return executerRequete(
        "SELECT * FROM MATIERES_PREMIERES "
        "WHERE UPPER(COULEUR) LIKE UPPER(?) "
        "OR UPPER(TYPE_CUIR) LIKE UPPER(?) "
        "OR UPPER(ORIGINE) LIKE UPPER(?) "
        "OR UPPER(STATUT) LIKE UPPER(?) "
        "OR UPPER(NOM_CUIR) LIKE UPPER(?) "
        "OR UPPER(REFERENCE_INTERNE) LIKE UPPER(?) "
        "OR UPPER(GAME) LIKE UPPER(?)",
        {p,p,p,p,p,p,p});
}

QList<MatierePremiere> MatiereDAO::filtrerDisponibles()
{
    return executerRequete(
        "SELECT * FROM MATIERES_PREMIERES WHERE UPPER(STATUT)='DISPONIBLE' ORDER BY ID");
}

QList<MatierePremiere> MatiereDAO::filtrerSeuilCritique(int seuil)
{
    return executerRequete(
        "SELECT * FROM MATIERES_PREMIERES WHERE RESERVE<=? ORDER BY RESERVE",
        {seuil});
}

QMap<QString, int> MatiereDAO::statsParType()
{
    QMap<QString, int> stats;
    if (!isConnected()) return stats;
    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    if (q.exec("SELECT TYPE_CUIR, COUNT(*) AS CNT FROM MATIERES_PREMIERES GROUP BY TYPE_CUIR"))
        while (q.next())
            stats[q.value("TYPE_CUIR").toString()] = q.value("CNT").toInt();
    return stats;
}

int MatiereDAO::prochainId()
{
    if (!isConnected()) return 1;
    QSqlQuery q(QSqlDatabase::database("qt_oracle_connection"));
    q.exec("SELECT NVL(MAX(ID),0)+1 AS NEXT_ID FROM MATIERES_PREMIERES");
    return q.next() ? q.value("NEXT_ID").toInt() : 1;
}

bool MatiereDAO::exporterPDF(const QList<MatierePremiere> &liste, const QString &cheminFichier)
{
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(cheminFichier);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument doc;
    QString html;
    html += "<html><body style='font-family:Arial;'>";
    html += "<h2 style='color:#582900;'>LEATHER HOUSE &mdash; Stock Matieres Premieres</h2>";
    html += "<p style='font-size:9px;color:#888;'>Exporte le: "
            + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm") + "</p>";
    html += "<table border='1' cellspacing='0' cellpadding='3' "
            "style='border-collapse:collapse;font-size:9px;width:100%;'>";
    html += "<tr style='background:#582900;color:white;font-weight:bold;'>"
            "<th>ID</th><th>Reference</th><th>Nom Cuir</th><th>Type</th>"
            "<th>Gamme</th><th>Couleur</th><th>Epaiss.</th>"
            "<th>Origine</th><th>Reserve</th><th>Statut</th></tr>";
    for (const MatierePremiere &m : liste) {
        QString bg = m.isSeuilCritique() ? "#ffe0e0" : "#fff8f0";
        html += QString("<tr style='background:%1;'>"
                        "<td align='center'>%2</td><td>%3</td><td>%4</td>"
                        "<td>%5</td><td>%6</td><td>%7</td><td align='center'>%8</td>"
                        "<td>%9</td><td align='center'><b>%10</b></td><td>%11</td></tr>")
                    .arg(bg).arg(m.getId())
                    .arg(m.getReference().toHtmlEscaped())
                    .arg(m.getNomCuir().toHtmlEscaped())
                    .arg(m.getTypeCuir().toHtmlEscaped())
                    .arg(m.getGamme().toHtmlEscaped())
                    .arg(m.getCouleur().toHtmlEscaped())
                    .arg(QString::number(m.getEpaisseur(),'f',2))
                    .arg(m.getOrigine().toHtmlEscaped())
                    .arg(m.getReserve())
                    .arg(m.getStatut().toHtmlEscaped());
    }
    html += "</table><p style='font-size:8px;color:#aaa;'>Total: "
            + QString::number(liste.size()) + " article(s)</p></body></html>";
    doc.setHtml(html);
    doc.print(&printer);
    return true;
}

bool MatiereDAO::exporterExcel(const QList<MatierePremiere> &liste, const QString &cheminFichier)
{
    QFile file(cheminFichier);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF"; // BOM pour Excel
    out << "ID;Reference;Nom Cuir;Type Cuir;Gamme;Couleur;Epaisseur;Origine;Reserve;Statut\n";

    for (const MatierePremiere &m : liste) {
        out << m.getId() << ";"
            << m.getReference() << ";"
            << m.getNomCuir() << ";"
            << m.getTypeCuir() << ";"
            << m.getGamme() << ";"
            << m.getCouleur() << ";"
            << QString::number(m.getEpaisseur(),'f',2) << ";"
            << m.getOrigine() << ";"
            << m.getReserve() << ";"
            << m.getStatut() << "\n";
    }
    file.close();
    return true;
}
