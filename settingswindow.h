#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsWindow;
}
QT_END_NAMESPACE

/// Fenêtre Paramètres : préférences persistées via QSettings, signaux pour application en direct.
class SettingsWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow() override;

    void setSessionUsername(const QString &username);
    void clearPasswordFields();

signals:
    void appearanceChanged();
    void languageChanged(const QString &languageCode);
    void passwordChangeRequested(const QString &username, const QString &oldPassword, const QString &newPassword);
    void profileChanged();
    void notificationSettingsChanged();
    void businessPreferencesChanged();
    void securitySettingsChanged();

private slots:
    void loadFromSettings();
    void saveAppearanceAndNotify();
    void saveNotificationsAndNotify();
    void saveLanguageAndNotify();
    void saveSecurityPrefs();
    void saveDataPrefs();
    void saveBusinessPrefs();
    void onChooseProfilePhoto();
    void onSaveProfile();
    void onApplyPasswordChange();

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::SettingsWindow *ui;
    QString m_sessionUsername;
    QString m_profilePhotoPath;

    static QString settingsGroup();
};

#endif // SETTINGSWINDOW_H
