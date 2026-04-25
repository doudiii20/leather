#ifndef MATIEREPREMIERE_H
#define MATIEREPREMIERE_H

#include <QString>
#include <QList>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;

/// Liens vers les champs de la fiche « Matières premières » (page_3).
struct MatierePremiereEditorWidgets {
    QLineEdit *idMp = nullptr;
    QLineEdit *reference = nullptr;
    QLineEdit *nomCuir = nullptr;
    QLineEdit *typeCuir = nullptr;
    QComboBox *gamme = nullptr;
    QLineEdit *couleur = nullptr;
    QLineEdit *statut = nullptr;
    QLineEdit *epaisseur = nullptr;
    QLineEdit *origine = nullptr;
    QLineEdit *reserve = nullptr;
    QLineEdit *fournisseurAffiche = nullptr;
    QLineEdit *prixAffiche = nullptr;
    QDateEdit *dateAchatAffiche = nullptr;
};

/// Donnée + accès Oracle (même connexion par défaut que Client / Produit).
class MatierePremiere
{
public:
    MatierePremiere();
    MatierePremiere(int id,
                    const QString &ref,
                    const QString &nomCuir,
                    const QString &typeCuir,
                    const QString &gamme,
                    const QString &couleur,
                    const QString &statut,
                    double epaisseur,
                    const QString &origine,
                    int reserve);

    int getId() const { return m_id; }
    QString getReference() const { return m_reference; }
    QString getNomCuir() const { return m_nomCuir; }
    QString getTypeCuir() const { return m_typeCuir; }
    QString getGamme() const { return m_gamme; }
    QString getCouleur() const { return m_couleur; }
    QString getStatut() const { return m_statut; }
    double getEpaisseur() const { return m_epaisseur; }
    QString getOrigine() const { return m_origine; }
    int getReserve() const { return m_reserve; }

    bool isSeuilCritique(int seuil = 10) const { return m_reserve <= seuil; }

    static QString lastSqlError;
    static bool ensureSchema(QString *errorMessage = nullptr);
    /// Quelques lignes si la table est vide (demonstration).
    static void seedDemoIfEmpty(QString *errorMessage = nullptr);
    static int nextAvailableId();
    static bool idExiste(int id);
    static bool populateTable(QTableWidget *table, QString *errorMessage = nullptr);
    static void filterTable(QTableWidget *table, const QString &needle);
    static void clearEditorFields(const MatierePremiereEditorWidgets &w);
    static int fillEditorFromTableRow(const MatierePremiereEditorWidgets &w, QTableWidget *table, int row);
    static QString statsSummaryPlain(QString *errorMessage = nullptr);

    bool ajouter() const;
    bool modifier(int oldId, int newId) const;
    static bool supprimer(int id);

    static bool exportPdfFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage = nullptr);
    static bool exportCsvFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage = nullptr);

private:
    int m_id = 0;
    QString m_reference;
    QString m_nomCuir;
    QString m_typeCuir;
    QString m_gamme;
    QString m_couleur;
    QString m_statut;
    double m_epaisseur = 0.0;
    QString m_origine;
    int m_reserve = 0;
};

#endif
