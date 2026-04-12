#include "client.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QRegularExpression>

namespace {
static bool execSql(const QString &sql, QString *errorMessage)
{
    QSqlQuery q;
    if (q.exec(sql)) {
        return true;
    }
    if (errorMessage) {
        *errorMessage = q.lastError().text().trimmed();
        if (errorMessage->isEmpty()) {
            *errorMessage = "Erreur SQL inconnue pendant la preparation du schema.";
        }
    }
    return false;
}

static bool columnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME = UPPER(:t) AND COLUMN_NAME = UPPER(:c)");
    q.bindValue(":t", table);
    q.bindValue(":c", column);
    if (!q.exec() || !q.next()) {
        return false;
    }
    return q.value(0).toInt() > 0;
}

static QString categorieFromScore(int score)
{
    if (score >= 85) return "VIP";
    if (score >= 65) return "Gold";
    if (score >= 40) return "Silver";
    return "Bronze";
}

static double remiseFromCategorie(const QString &cat)
{
    if (cat == "VIP") return 20.0;
    if (cat == "Gold") return 15.0;
    if (cat == "Silver") return 8.0;
    return 3.0;
}

struct AiContext
{
    double avgAchats = 0.0;
    double avgFrequence = 0.0;
    double avgRetards = 0.0;
    double avgUtilisationCredit = 0.0;
    int nbPaiements = 0;
    int paiementsRecents = 0;
    double joursDepuisDernierPaiement = 999.0;
};

static AiContext fetchAiContext(int clientId)
{
    AiContext ctx;
    QSqlQuery q1;
    if (q1.exec("SELECT NVL(AVG(TOTAL_ACHATS),0), NVL(AVG(FREQUENCE_ACHAT),0), NVL(AVG(RETARDS_PAIEMENT),0), "
                "NVL(AVG(CASE WHEN LIMITE_CREDIT>0 THEN SOLDE_CREDIT_UTILISE/LIMITE_CREDIT ELSE 0 END),0) FROM CLIENT")
        && q1.next()) {
        ctx.avgAchats = q1.value(0).toDouble();
        ctx.avgFrequence = q1.value(1).toDouble();
        ctx.avgRetards = q1.value(2).toDouble();
        ctx.avgUtilisationCredit = q1.value(3).toDouble();
    }

    if (clientId > 0) {
        QSqlQuery q2;
        q2.prepare("SELECT NVL(COUNT(*),0), "
                   "NVL(SUM(CASE WHEN DATE_PAIEMENT >= SYSDATE-90 THEN 1 ELSE 0 END),0), "
                   "NVL(MIN(SYSDATE - DATE_PAIEMENT),999) "
                   "FROM CLIENT_PAIEMENTS WHERE CLIENT_ID=:id");
        q2.bindValue(":id", clientId);
        if (q2.exec() && q2.next()) {
            ctx.nbPaiements = q2.value(0).toInt();
            ctx.paiementsRecents = q2.value(1).toInt();
            ctx.joursDepuisDernierPaiement = q2.value(2).toDouble();
        }
    }
    return ctx;
}

static double clamp01(double v)
{
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}
}

bool Client::ensureSchema(QString *errorMessage)
{
    if (!execSql("BEGIN EXECUTE IMMEDIATE 'CREATE TABLE CLIENT (ID NUMBER PRIMARY KEY)'; EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;", errorMessage)) {
        return false;
    }

    struct Alter {
        QString col;
        QString ddl;
    };
    const QList<Alter> alters = {
        {"NOM", "ALTER TABLE CLIENT ADD (NOM VARCHAR2(100) NOT NULL)"},
        {"PRENOM", "ALTER TABLE CLIENT ADD (PRENOM VARCHAR2(100) NOT NULL)"},
        {"EMAIL", "ALTER TABLE CLIENT ADD (EMAIL VARCHAR2(150) NOT NULL)"},
        {"TELEPHONE", "ALTER TABLE CLIENT ADD (TELEPHONE VARCHAR2(20) NOT NULL)"},
        {"ADRESSE", "ALTER TABLE CLIENT ADD (ADRESSE VARCHAR2(200) NOT NULL)"},
        {"STATUTCLIENT", "ALTER TABLE CLIENT ADD (STATUTCLIENT VARCHAR2(30) DEFAULT 'actif' NOT NULL)"},
        {"CANALACQUISITION", "ALTER TABLE CLIENT ADD (CANALACQUISITION VARCHAR2(50))"},
        {"MODEPAIEMENTPREFERE", "ALTER TABLE CLIENT ADD (MODEPAIEMENTPREFERE VARCHAR2(50))"},
        {"REMISEACCORDEE", "ALTER TABLE CLIENT ADD (REMISEACCORDEE NUMBER(5,2) DEFAULT 0 NOT NULL)"},
        {"TOTAL_ACHATS", "ALTER TABLE CLIENT ADD (TOTAL_ACHATS NUMBER(12,2) DEFAULT 0 NOT NULL)"},
        {"FREQUENCE_ACHAT", "ALTER TABLE CLIENT ADD (FREQUENCE_ACHAT NUMBER DEFAULT 0 NOT NULL)"},
        {"RETARDS_PAIEMENT", "ALTER TABLE CLIENT ADD (RETARDS_PAIEMENT NUMBER DEFAULT 0 NOT NULL)"},
        {"CATEGORIE", "ALTER TABLE CLIENT ADD (CATEGORIE VARCHAR2(10) DEFAULT 'Bronze' NOT NULL)"},
        {"LIMITE_CREDIT", "ALTER TABLE CLIENT ADD (LIMITE_CREDIT NUMBER(12,2) DEFAULT 0 NOT NULL)"},
        {"SOLDE_CREDIT_UTILISE", "ALTER TABLE CLIENT ADD (SOLDE_CREDIT_UTILISE NUMBER(12,2) DEFAULT 0 NOT NULL)"},
        {"SCORE_CLIENT", "ALTER TABLE CLIENT ADD (SCORE_CLIENT NUMBER DEFAULT 0 NOT NULL)"},
        {"SCORE_RISQUE", "ALTER TABLE CLIENT ADD (SCORE_RISQUE NUMBER DEFAULT 100 NOT NULL)"}
    };

    for (const Alter &a : alters) {
        if (!columnExists("CLIENT", a.col)) {
            if (!execSql(a.ddl, errorMessage)) {
                return false;
            }
        }
    }

    if (!execSql(
            "BEGIN EXECUTE IMMEDIATE 'CREATE TABLE CLIENT_PAIEMENTS ("
            "ID NUMBER PRIMARY KEY, "
            "CLIENT_ID NUMBER NOT NULL, MONTANT NUMBER(12,2) NOT NULL, "
            "DATE_PAIEMENT DATE DEFAULT SYSDATE NOT NULL, NOTE VARCHAR2(200), "
            "CONSTRAINT FK_CLIENT_PAIEMENTS FOREIGN KEY (CLIENT_ID) REFERENCES CLIENT(ID) ON DELETE CASCADE)'; "
            "EXCEPTION WHEN OTHERS THEN IF SQLCODE != -955 THEN RAISE; END IF; END;",
            errorMessage)) {
        return false;
    }

    return true;
}

void Client::recalculerScoresEtCategorie(ClientData &client)
{
    // IA locale explicable: pondération sur données client + baseline globale réelle (DB)
    const AiContext ai = fetchAiContext(client.id);
    const double achatsRef = (ai.avgAchats > 1.0 ? ai.avgAchats : 1.0);
    const double freqRef = (ai.avgFrequence > 1.0 ? ai.avgFrequence : 1.0);
    const double retardRef = (ai.avgRetards >= 0.0 ? ai.avgRetards + 1.0 : 1.0);
    const double util = (client.limiteCredit > 0.0) ? (client.soldeCreditUtilise / client.limiteCredit) : 0.0;

    const double achatScore = clamp01((client.totalAchats / achatsRef) / 2.0);
    const double freqScore = clamp01((client.frequenceAchat / freqRef) / 2.0);
    const double retardScore = clamp01(1.0 - ((client.retardsPaiement / retardRef) / 2.0));
    const double recencePaiementScore = clamp01(1.0 - (ai.joursDepuisDernierPaiement / 120.0));
    const double activitePaiementScore = clamp01(static_cast<double>(ai.paiementsRecents) / 6.0);

    const double iaScore =
        achatScore * 0.35 +
        freqScore * 0.25 +
        retardScore * 0.20 +
        recencePaiementScore * 0.10 +
        activitePaiementScore * 0.10;

    const int score = qBound(0, static_cast<int>(iaScore * 100.0), 100);
    client.scoreClient = score;
    client.categorie = categorieFromScore(score);
    client.remiseAccordee = remiseFromCategorie(client.categorie);

    const double utilRef = (ai.avgUtilisationCredit > 0.0 ? ai.avgUtilisationCredit : 0.35);
    const double utilisationPenalite = clamp01((util / (utilRef * 2.0)));
    const double retardPenalite = clamp01((client.retardsPaiement / retardRef) / 2.0);
    const double paiementPenalite = 1.0 - ((recencePaiementScore * 0.6) + (activitePaiementScore * 0.4));

    const int risque = qBound(0, static_cast<int>(100.0 - (utilisationPenalite * 45.0) - (retardPenalite * 35.0) - (paiementPenalite * 20.0)), 100);
    client.scoreRisque = risque;
}

bool Client::validerClient(const ClientData &client, QString *errorMessage)
{
    if (client.id <= 0) {
        if (errorMessage) *errorMessage = "ID client invalide (doit être > 0).";
        return false;
    }
    if (client.nom.trimmed().isEmpty() || client.prenom.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "Nom et prénom sont obligatoires.";
        return false;
    }
    if (client.email.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "Email obligatoire.";
        return false;
    }
    const QRegularExpression emailRegex("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$", QRegularExpression::CaseInsensitiveOption);
    if (!emailRegex.match(client.email.trimmed()).hasMatch()) {
        if (errorMessage) *errorMessage = "Format email invalide.";
        return false;
    }
    const QRegularExpression phoneRegex("^\\+?[0-9 ]{8,15}$");
    if (!phoneRegex.match(client.telephone.trimmed()).hasMatch()) {
        if (errorMessage) *errorMessage = "Téléphone invalide (8 à 15 chiffres, + et espaces autorisés).";
        return false;
    }
    if (client.adresse.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "Adresse obligatoire.";
        return false;
    }
    if (client.limiteCredit < 0.0 || client.soldeCreditUtilise < 0.0) {
        if (errorMessage) *errorMessage = "Les montants de crédit doivent être positifs.";
        return false;
    }
    return true;
}

bool Client::ajouter(ClientData &client, QString *errorMessage)
{
    if (!QSqlDatabase::database().isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de donnees fermee. Verifiez le DSN ODBC, l'utilisateur et le mot de passe.";
        return false;
    }
    if (!ensureSchema(errorMessage)) return false;
    if (!validerClient(client, errorMessage)) return false;
    recalculerScoresEtCategorie(client);

    // Controle explicite du doublon d'ID pour afficher un message clair.
    QSqlQuery checkId;
    checkId.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID = :id");
    checkId.bindValue(":id", client.id);
    if (!checkId.exec() || !checkId.next()) {
        if (errorMessage) *errorMessage = "Verification de l'ID impossible: " + checkId.lastError().text();
        return false;
    }
    if (checkId.value(0).toInt() > 0) {
        if (errorMessage) *errorMessage = "ID deja utilise. Veuillez saisir un autre ID.";
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO CLIENT (ID, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, STATUTCLIENT, REMISEACCORDEE, "
        "CANALACQUISITION, MODEPAIEMENTPREFERE, TOTAL_ACHATS, FREQUENCE_ACHAT, RETARDS_PAIEMENT, "
        "CATEGORIE, LIMITE_CREDIT, SOLDE_CREDIT_UTILISE, SCORE_CLIENT, SCORE_RISQUE) "
        "VALUES (:id, :nom, :prenom, :email, :telephone, :adresse, :statut, :remise, :canal, :mode, "
        ":total, :freq, :retard, :cat, :limite, :solde, :score, :risque)");
    query.bindValue(":id", client.id);
    query.bindValue(":nom", client.nom.trimmed());
    query.bindValue(":prenom", client.prenom.trimmed());
    query.bindValue(":email", client.email.trimmed().toLower());
    query.bindValue(":telephone", client.telephone.trimmed());
    query.bindValue(":adresse", client.adresse.trimmed());
    query.bindValue(":statut", client.statutClient.trimmed());
    query.bindValue(":remise", client.remiseAccordee);
    query.bindValue(":canal", client.canalAcquisition.trimmed());
    query.bindValue(":mode", client.modePaiementPrefere.trimmed());
    query.bindValue(":total", client.totalAchats);
    query.bindValue(":freq", client.frequenceAchat);
    query.bindValue(":retard", client.retardsPaiement);
    query.bindValue(":cat", client.categorie);
    query.bindValue(":limite", client.limiteCredit);
    query.bindValue(":solde", client.soldeCreditUtilise);
    query.bindValue(":score", client.scoreClient);
    query.bindValue(":risque", client.scoreRisque);
    if (!query.exec()) {
        if (errorMessage) {
            const QString sqlErr = query.lastError().text().trimmed();
            if (sqlErr.contains("ORA-00001", Qt::CaseInsensitive)) {
                *errorMessage = "Doublon detecte: cet ID (ou un autre champ unique) existe deja. Utilisez un autre ID.";
            } else if (!sqlErr.isEmpty()) {
                *errorMessage = sqlErr;
            } else {
                *errorMessage = "Insertion client impossible. Verifiez que l'ID n'existe pas deja et que la table CLIENT est accessible.";
            }
        }
        return false;
    }
    return true;
}

bool Client::modifier(const ClientData &client, QString *errorMessage)
{
    if (!validerClient(client, errorMessage)) return false;
    ClientData tmp = client;
    recalculerScoresEtCategorie(tmp);

    QSqlQuery query;
    query.prepare(
        "UPDATE CLIENT SET NOM=:nom, PRENOM=:prenom, EMAIL=:email, TELEPHONE=:telephone, ADRESSE=:adresse, "
        "STATUTCLIENT=:statut, REMISEACCORDEE=:remise, CANALACQUISITION=:canal, MODEPAIEMENTPREFERE=:mode, "
        "TOTAL_ACHATS=:total, FREQUENCE_ACHAT=:freq, RETARDS_PAIEMENT=:retard, CATEGORIE=:cat, "
        "LIMITE_CREDIT=:limite, SOLDE_CREDIT_UTILISE=:solde, SCORE_CLIENT=:score, SCORE_RISQUE=:risque "
        "WHERE ID=:id");
    query.bindValue(":id", tmp.id);
    query.bindValue(":nom", tmp.nom.trimmed());
    query.bindValue(":prenom", tmp.prenom.trimmed());
    query.bindValue(":email", tmp.email.trimmed().toLower());
    query.bindValue(":telephone", tmp.telephone.trimmed());
    query.bindValue(":adresse", tmp.adresse.trimmed());
    query.bindValue(":statut", tmp.statutClient.trimmed());
    query.bindValue(":remise", tmp.remiseAccordee);
    query.bindValue(":canal", tmp.canalAcquisition.trimmed());
    query.bindValue(":mode", tmp.modePaiementPrefere.trimmed());
    query.bindValue(":total", tmp.totalAchats);
    query.bindValue(":freq", tmp.frequenceAchat);
    query.bindValue(":retard", tmp.retardsPaiement);
    query.bindValue(":cat", tmp.categorie);
    query.bindValue(":limite", tmp.limiteCredit);
    query.bindValue(":solde", tmp.soldeCreditUtilise);
    query.bindValue(":score", tmp.scoreClient);
    query.bindValue(":risque", tmp.scoreRisque);
    if (!query.exec()) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        if (errorMessage) *errorMessage = "Aucun client trouvé avec cet ID.";
        return false;
    }
    return true;
}

bool Client::supprimer(int id, QString *errorMessage)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENT WHERE ID=:id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        if (errorMessage) *errorMessage = "Aucun client trouvé avec cet ID.";
        return false;
    }
    return true;
}

bool Client::chargerTous(QList<ClientData> &clients, QString *errorMessage)
{
    clients.clear();
    QSqlQuery query;
    if (!query.exec("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, STATUTCLIENT, REMISEACCORDEE, "
                    "CANALACQUISITION, MODEPAIEMENTPREFERE, TOTAL_ACHATS, FREQUENCE_ACHAT, RETARDS_PAIEMENT, "
                    "CATEGORIE, LIMITE_CREDIT, SOLDE_CREDIT_UTILISE, SCORE_CLIENT, SCORE_RISQUE "
                    "FROM CLIENT ORDER BY ID")) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }
    while (query.next()) {
        ClientData c;
        c.id = query.value(0).toInt();
        c.nom = query.value(1).toString();
        c.prenom = query.value(2).toString();
        c.email = query.value(3).toString();
        c.telephone = query.value(4).toString();
        c.adresse = query.value(5).toString();
        c.statutClient = query.value(6).toString();
        c.remiseAccordee = query.value(7).toDouble();
        c.canalAcquisition = query.value(8).toString();
        c.modePaiementPrefere = query.value(9).toString();
        c.totalAchats = query.value(10).toDouble();
        c.frequenceAchat = query.value(11).toInt();
        c.retardsPaiement = query.value(12).toInt();
        c.categorie = query.value(13).toString();
        c.limiteCredit = query.value(14).toDouble();
        c.soldeCreditUtilise = query.value(15).toDouble();
        c.scoreClient = query.value(16).toInt();
        c.scoreRisque = query.value(17).toInt();
        clients.push_back(c);
    }
    return true;
}

bool Client::chargerParId(int id, ClientData &client, QString *errorMessage)
{
    QList<ClientData> clients;
    QSqlQuery query;
    query.prepare("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, STATUTCLIENT, REMISEACCORDEE, "
                  "CANALACQUISITION, MODEPAIEMENTPREFERE, TOTAL_ACHATS, FREQUENCE_ACHAT, RETARDS_PAIEMENT, "
                  "CATEGORIE, LIMITE_CREDIT, SOLDE_CREDIT_UTILISE, SCORE_CLIENT, SCORE_RISQUE FROM CLIENT WHERE ID=:id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }
    if (!query.next()) {
        if (errorMessage) *errorMessage = "Client introuvable.";
        return false;
    }
    client.id = query.value(0).toInt();
    client.nom = query.value(1).toString();
    client.prenom = query.value(2).toString();
    client.email = query.value(3).toString();
    client.telephone = query.value(4).toString();
    client.adresse = query.value(5).toString();
    client.statutClient = query.value(6).toString();
    client.remiseAccordee = query.value(7).toDouble();
    client.canalAcquisition = query.value(8).toString();
    client.modePaiementPrefere = query.value(9).toString();
    client.totalAchats = query.value(10).toDouble();
    client.frequenceAchat = query.value(11).toInt();
    client.retardsPaiement = query.value(12).toInt();
    client.categorie = query.value(13).toString();
    client.limiteCredit = query.value(14).toDouble();
    client.soldeCreditUtilise = query.value(15).toDouble();
    client.scoreClient = query.value(16).toInt();
    client.scoreRisque = query.value(17).toInt();
    return true;
}

bool Client::rechercherParMotCle(const QString &motCle, QList<ClientData> &clients, QString *errorMessage)
{
    clients.clear();
    QSqlQuery query;
    query.prepare("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, STATUTCLIENT, REMISEACCORDEE, "
                  "CANALACQUISITION, MODEPAIEMENTPREFERE, TOTAL_ACHATS, FREQUENCE_ACHAT, RETARDS_PAIEMENT, "
                  "CATEGORIE, LIMITE_CREDIT, SOLDE_CREDIT_UTILISE, SCORE_CLIENT, SCORE_RISQUE "
                  "FROM CLIENT WHERE UPPER(NOM) LIKE UPPER(:k) OR UPPER(PRENOM) LIKE UPPER(:k) OR UPPER(EMAIL) LIKE UPPER(:k) "
                  "OR UPPER(TELEPHONE) LIKE UPPER(:k) OR UPPER(ADRESSE) LIKE UPPER(:k) OR TO_CHAR(ID) LIKE :k "
                  "ORDER BY ID");
    query.bindValue(":k", "%" + motCle + "%");
    if (!query.exec()) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }
    while (query.next()) {
        ClientData c;
        c.id = query.value(0).toInt();
        c.nom = query.value(1).toString();
        c.prenom = query.value(2).toString();
        c.email = query.value(3).toString();
        c.telephone = query.value(4).toString();
        c.adresse = query.value(5).toString();
        c.statutClient = query.value(6).toString();
        c.remiseAccordee = query.value(7).toDouble();
        c.canalAcquisition = query.value(8).toString();
        c.modePaiementPrefere = query.value(9).toString();
        c.totalAchats = query.value(10).toDouble();
        c.frequenceAchat = query.value(11).toInt();
        c.retardsPaiement = query.value(12).toInt();
        c.categorie = query.value(13).toString();
        c.limiteCredit = query.value(14).toDouble();
        c.soldeCreditUtilise = query.value(15).toDouble();
        c.scoreClient = query.value(16).toInt();
        c.scoreRisque = query.value(17).toInt();
        clients.push_back(c);
    }
    return true;
}

bool Client::rechercherEtFiltrer(const QString &motCle,
                                 const QString &categorie,
                                 const QString &statut,
                                 const QString &niveauRisque,
                                 QList<ClientData> &clients,
                                 QString *errorMessage)
{
    clients.clear();
    QSqlQuery query;

    QString sql = "SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, ADRESSE, STATUTCLIENT, REMISEACCORDEE, "
                  "CANALACQUISITION, MODEPAIEMENTPREFERE, TOTAL_ACHATS, FREQUENCE_ACHAT, RETARDS_PAIEMENT, "
                  "CATEGORIE, LIMITE_CREDIT, SOLDE_CREDIT_UTILISE, SCORE_CLIENT, SCORE_RISQUE "
                  "FROM CLIENT WHERE 1=1 ";

    if (!motCle.trimmed().isEmpty()) {
        sql += "AND (UPPER(NOM) LIKE UPPER(:k) OR UPPER(PRENOM) LIKE UPPER(:k) OR UPPER(EMAIL) LIKE UPPER(:k) "
               "OR UPPER(TELEPHONE) LIKE UPPER(:k) OR UPPER(ADRESSE) LIKE UPPER(:k) OR TO_CHAR(ID) LIKE :k) ";
    }
    if (categorie != "Toutes") {
        sql += "AND CATEGORIE = :categorie ";
    }
    if (statut != "Tous") {
        sql += "AND STATUTCLIENT = :statut ";
    }
    if (niveauRisque == "Faible") {
        sql += "AND SCORE_RISQUE >= 70 ";
    } else if (niveauRisque == "Moyen") {
        sql += "AND SCORE_RISQUE >= 40 AND SCORE_RISQUE < 70 ";
    } else if (niveauRisque == "Eleve") {
        sql += "AND SCORE_RISQUE < 40 ";
    }

    sql += "ORDER BY ID";

    query.prepare(sql);
    if (!motCle.trimmed().isEmpty()) {
        query.bindValue(":k", "%" + motCle.trimmed() + "%");
    }
    if (categorie != "Toutes") {
        query.bindValue(":categorie", categorie);
    }
    if (statut != "Tous") {
        query.bindValue(":statut", statut);
    }

    if (!query.exec()) {
        if (errorMessage) *errorMessage = query.lastError().text();
        return false;
    }

    while (query.next()) {
        ClientData c;
        c.id = query.value(0).toInt();
        c.nom = query.value(1).toString();
        c.prenom = query.value(2).toString();
        c.email = query.value(3).toString();
        c.telephone = query.value(4).toString();
        c.adresse = query.value(5).toString();
        c.statutClient = query.value(6).toString();
        c.remiseAccordee = query.value(7).toDouble();
        c.canalAcquisition = query.value(8).toString();
        c.modePaiementPrefere = query.value(9).toString();
        c.totalAchats = query.value(10).toDouble();
        c.frequenceAchat = query.value(11).toInt();
        c.retardsPaiement = query.value(12).toInt();
        c.categorie = query.value(13).toString();
        c.limiteCredit = query.value(14).toDouble();
        c.soldeCreditUtilise = query.value(15).toDouble();
        c.scoreClient = query.value(16).toInt();
        c.scoreRisque = query.value(17).toInt();
        clients.push_back(c);
    }
    return true;
}

CreditCheckResult Client::verifierBlocageCommande(int clientId, double montantCommande, QString *errorMessage)
{
    CreditCheckResult r;
    ClientData c;
    if (!chargerParId(clientId, c, errorMessage)) {
        r.message = errorMessage ? *errorMessage : "Client introuvable.";
        return r;
    }

    recalculerScoresEtCategorie(c);
    const double restant = c.limiteCredit - c.soldeCreditUtilise;
    r.restant = restant;
    if (c.scoreRisque < 30) {
        r.allowed = false;
        r.message = "Commande bloquée: risque client critique détecté par le scoring IA.";
    } else if (montantCommande > restant) {
        r.allowed = false;
        r.message = "Commande bloquée: limite de crédit dépassée.";
    } else if (c.scoreRisque < 50 && montantCommande > (restant * 0.8)) {
        r.allowed = false;
        r.message = "Commande bloquée préventivement: risque moyen et consommation crédit élevée.";
    } else {
        r.allowed = true;
        r.message = "Commande autorisée.";
    }
    return r;
}

bool Client::enregistrerPaiement(int clientId, double montant, const QString &note, QString *errorMessage)
{
    if (montant <= 0.0) {
        if (errorMessage) *errorMessage = "Le montant du paiement doit être > 0.";
        return false;
    }

    QSqlQuery q1;
    q1.prepare("INSERT INTO CLIENT_PAIEMENTS (ID, CLIENT_ID, MONTANT, NOTE) "
               "VALUES ((SELECT NVL(MAX(ID),0)+1 FROM CLIENT_PAIEMENTS), :id, :m, :n)");
    q1.bindValue(":id", clientId);
    q1.bindValue(":m", montant);
    q1.bindValue(":n", note.trimmed());
    if (!q1.exec()) {
        if (errorMessage) *errorMessage = q1.lastError().text();
        return false;
    }

    QSqlQuery q2;
    q2.prepare("UPDATE CLIENT SET SOLDE_CREDIT_UTILISE = GREATEST(SOLDE_CREDIT_UTILISE - :m, 0) WHERE ID=:id");
    q2.bindValue(":m", montant);
    q2.bindValue(":id", clientId);
    if (!q2.exec()) {
        if (errorMessage) *errorMessage = q2.lastError().text();
        return false;
    }
    ClientData c;
    if (chargerParId(clientId, c, errorMessage)) {
        c.soldeCreditUtilise = qMax(0.0, c.soldeCreditUtilise - montant);
        if (!modifier(c, errorMessage)) {
            return false;
        }
    }
    return true;
}

bool Client::historiquePaiements(int clientId, QList<QPair<QString, QString>> &rows, QString *errorMessage)
{
    rows.clear();
    QSqlQuery q;
    q.prepare("SELECT TO_CHAR(DATE_PAIEMENT, 'YYYY-MM-DD HH24:MI'), TO_CHAR(MONTANT), NVL(NOTE, '-') "
              "FROM CLIENT_PAIEMENTS WHERE CLIENT_ID=:id ORDER BY DATE_PAIEMENT DESC");
    q.bindValue(":id", clientId);
    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    while (q.next()) {
        rows.append(qMakePair(q.value(0).toString(), q.value(1).toString() + " | " + q.value(2).toString()));
    }
    return true;
}

QString Client::genererExplicationIA(const ClientData &client)
{
    ClientData tmp = client;
    if (tmp.id <= 0) {
        // For simulation on form fields without persisted client.
        tmp.id = -1;
    }
    recalculerScoresEtCategorie(tmp);

    const double utilisation = (tmp.limiteCredit > 0.0) ? ((tmp.soldeCreditUtilise / tmp.limiteCredit) * 100.0) : 0.0;
    QString recommandation;
    if (tmp.categorie == "VIP") {
        recommandation = "Recommandation: fidélisation premium, offres exclusives, limite crédit flexible.";
    } else if (tmp.categorie == "Gold") {
        recommandation = "Recommandation: upsell ciblé et relance proactive avant baisse d'activité.";
    } else if (tmp.categorie == "Silver") {
        recommandation = "Recommandation: campagnes de réactivation et suivi des retards.";
    } else {
        recommandation = "Recommandation: onboarding commercial, risque à surveiller, crédit prudent.";
    }

    QString risqueMsg = (tmp.scoreRisque >= 70) ? "Risque faible"
                      : (tmp.scoreRisque >= 40) ? "Risque moyen"
                                                : "Risque élevé";

    return QString(
        "Modele IA explicable (scoring multicritere)\n"
        "- Score client: %1/100\n"
        "- Categorie predite: %2 (remise automatique %3%)\n"
        "- Risque paiement: %4/100 (%5)\n"
        "- Variables clefs:\n"
        "  * Total achats: %6\n"
        "  * Frequence achat: %7\n"
        "  * Retards paiement: %8\n"
        "  * Utilisation credit: %9%\n"
        "%10")
        .arg(tmp.scoreClient)
        .arg(tmp.categorie)
        .arg(QString::number(tmp.remiseAccordee, 'f', 1))
        .arg(tmp.scoreRisque)
        .arg(risqueMsg)
        .arg(QString::number(tmp.totalAchats, 'f', 2))
        .arg(tmp.frequenceAchat)
        .arg(tmp.retardsPaiement)
        .arg(QString::number(utilisation, 'f', 1))
        .arg(recommandation);
}