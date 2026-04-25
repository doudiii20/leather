#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateTimeEdit;

/// Liens vers les champs de la fiche produit (noms logiques ; widgets Designer inchanges).
struct ProduitEditorWidgets {
    QLineEdit *idProduit = nullptr;
    QLineEdit *nomProduit = nullptr;
    QLineEdit *categorie = nullptr;
    QLineEdit *typeCuir = nullptr;
    QLineEdit *quantiteStock = nullptr;
    QLineEdit *style = nullptr;
    QComboBox *qualite = nullptr;
    QComboBox *etat = nullptr;
    QComboBox *typeDesign = nullptr;
    QDateTimeEdit *dateFabrication = nullptr;
};

/// CRUD produits aligne sur le schema CommerceStore (PRODUITS + STOCK).
class Produit
{
public:
    Produit() {}
    Produit(int id,
            QString nom_produit,
            QString categorie,
            QString type_cuir,
            QString qualite,
            int quantite_stock,
            QString etat_produit,
            QDate date_fabrication,
            QString type_design,
            QString style);

    bool ajouter();
    bool modifier(int oldId, int newId);
    bool supprimer(int id);
    static QSqlQueryModel *afficher();
    static bool idExisteDeja(int id);
    /// Prochain ID libre (MAX(ID)+1), ou 1 si table vide.
    static int nextAvailableId();
    /// Decompresse le champ STYLE (cf. packStyle).
    static void unpackPackedStyle(const QString &packed,
                                  QString *qualite,
                                  QString *etat,
                                  QDate *dateFab,
                                  QString *typeDesign,
                                  QString *userStyle);
    /// Texte court pour le tableau (segment S: / TD:), sans perdre `packed` (voir UserRole dans populateProductTable).
    static QString packedStyleToTableDisplay(const QString &packed);

    /// Vide la fiche produit (combos a l'index 0, date du jour).
    static void clearEditorFields(const ProduitEditorWidgets &w);
    /// Remplit la fiche depuis une ligne du tableau (modele afficher()). Retourne l'ID selectionne ou -1.
    static int fillEditorFromTableRow(const ProduitEditorWidgets &w, QTableWidget *table, int row);

    /// Remplit le QTableWidget depuis afficher(). Retourne false + errorMessage si echec.
    static bool populateProductTable(QTableWidget *table, QString *errorMessage = nullptr);
    /// Filtre les lignes (needle vide = tout afficher). mode : 0 nom (col 1), 1 type cuir (col 3), 2 etat (col 6).
    static void filterProductTable(QTableWidget *table, const QString &needle, int mode);
    static int searchFilterModeFromComboText(const QString &comboCurrentText);
    /// Tolere les formats ODBC/Oracle (ex. "12.0") pour l'ID affiche dans le tableau.
    static int parseProductIdText(const QString &text);

    /// Texte pour la zone alertes (PRODUITS + STOCK). errorMessage rempli si requete SQL invalide.
    static QString defectAlertsPlainText(QString *errorMessage = nullptr);
    /// Contexte texte pour le chatbot (lignes visibles du tableau).
    static QString chatbotContextFromProductTable(QTableWidget *table, int maxRows = 40);

    static QString lastSqlError;

private:
    static QString packStyle(const QString &qualite,
                              const QString &etat,
                              const QDate &dateFab,
                              const QString &typeDesign,
                              const QString &styleUtilisateur);

    int id = 0;
    QString nom_produit;
    QString categorie;
    QString type_cuir;
    QString qualite;
    int quantite_stock = 0;
    QString etat_produit;
    QDate date_fabrication;
    QString type_design;
    QString style;
};

#endif
