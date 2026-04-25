#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QDate>
#include <QString>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;

/// Champs fiche « Employés » (pageEmployes).
struct EmployeEditorWidgets {
    QLineEdit *cin = nullptr;
    QLineEdit *nom = nullptr;
    QLineEdit *prenom = nullptr;
    QComboBox *sexe = nullptr;
    QLineEdit *salaire = nullptr;
    QDateEdit *dateEmbauche = nullptr;
    QLineEdit *telephone = nullptr;
    QLineEdit *poste = nullptr;
    QLineEdit *adresse = nullptr;
    QLineEdit *email = nullptr;
};

/// Accès Oracle (connexion par défaut Qt) — aligné sur le projet « nour employee » (table EMPLOYES).
class Employe
{
public:
    Employe();
    Employe(const QString &cin,
            const QString &nom,
            const QString &prenom,
            const QString &sexe,
            double salaire,
            const QDate &dateEmbauche,
            const QString &telephone,
            const QString &poste,
            const QString &adresse,
            const QString &email);

    QString cin() const { return m_cin; }

    static bool ensureSchema(QString *errorMessage = nullptr);
    static void seedDemoIfEmpty(QString *errorMessage = nullptr);

    static bool populateTable(QTableWidget *table, QString *errorMessage = nullptr);
    static void applySearchFilter(QTableWidget *table, const QString &text);
    static void clearEditor(const EmployeEditorWidgets &w);
    static int fillEditorFromTableRow(const EmployeEditorWidgets &w, QTableWidget *table, int row);

    /// Vide si OK, sinon message pour QMessageBox.
    static QString validateForm(const EmployeEditorWidgets &w);

    static void installInputValidators(const EmployeEditorWidgets &w);

    static bool cinExists(const QString &cin, QString *errorMessage = nullptr);

    bool ajouter(QString *errorMessage = nullptr) const;
    bool modifier(QString *errorMessage = nullptr) const;
    static bool supprimer(const QString &cin, QString *errorMessage = nullptr);

    static bool exportPdfFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage = nullptr);
    static bool exportCsvFromTable(QTableWidget *table, const QString &filePath, QString *errorMessage = nullptr);

private:
    QString m_cin;
    QString m_nom;
    QString m_prenom;
    QString m_sexe;
    double m_salaire = 0.0;
    QDate m_dateEmbauche;
    QString m_telephone;
    QString m_poste;
    QString m_adresse;
    QString m_email;
};

#endif
