#ifndef FOURNISSEUR_H
#define FOURNISSEUR_H

#include <QObject>
#include <QList>
#include <QString>

namespace Ui {
class MainWindow;
}

class QSpinBox;

class FournisseurManager : public QObject
{
    Q_OBJECT

public:
    explicit FournisseurManager(Ui::MainWindow *ui, QObject *parent = nullptr);

signals:
    void fournisseursChanged();

private:
    struct FournisseurData {
        QString code;
        QString raisonSociale;
        QString fiabilite;
        QString emailAchats;
        QString zone;
        int slaJours = 0;
        int commandes = 0;
    };

    void setupTable();
    void setupFormControls();
    void setupConnections();
    void clearForm();
    void loadRowToForm(int row);
    void refreshTable(const QString &keyword = QString());

    bool validateForm(QString *errorMessage = nullptr) const;
    FournisseurData readFormData() const;
    void writeFormData(const FournisseurData &d);
    bool ensureSchema(QString *errorMessage = nullptr) const;
    bool ajouterDb(const FournisseurData &d, QString *errorMessage = nullptr) const;
    bool modifierDb(const QString &selectedCode, const FournisseurData &d, QString *errorMessage = nullptr) const;
    bool supprimerDb(const QString &code, QString *errorMessage = nullptr) const;
    bool chargerDb(QList<FournisseurData> &rows, const QString &keyword, QString *errorMessage = nullptr) const;
    bool codeExistsDb(const QString &code, const QString &excludeCode = QString(), QString *errorMessage = nullptr) const;

    Ui::MainWindow *m_ui = nullptr;
    QSpinBox *m_spinCommandes = nullptr;
};

#endif // FOURNISSEUR_H
