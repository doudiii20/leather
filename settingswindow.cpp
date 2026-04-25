#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QPair>
#include <QEvent>

namespace {

void populateAccentCombo(QComboBox *combo)
{
    combo->clear();
    const QPair<QString, QString> items[] = {
        { QStringLiteral("Cuir royal"), QStringLiteral("#5d2e06") },
        { QStringLiteral("Bleu profond"), QStringLiteral("#1e3a5f") },
        { QStringLiteral("Vert forêt"), QStringLiteral("#1b5e20") },
        { QStringLiteral("Bordeaux"), QStringLiteral("#6a1b2a") },
        { QStringLiteral("Graphite"), QStringLiteral("#37474f") },
    };
    for (const auto &it : items)
        combo->addItem(it.first, it.second);
}

void populateLanguageCombo(QComboBox *combo)
{
    combo->clear();
    combo->addItem(QStringLiteral("Français"), QStringLiteral("fr"));
    combo->addItem(QStringLiteral("English"), QStringLiteral("en"));
    combo->addItem(QStringLiteral("العربية"), QStringLiteral("ar"));
}

void populateCurrencyCombo(QComboBox *combo)
{
    combo->clear();
    combo->addItem(QStringLiteral("TND — Dinar tunisien"), QStringLiteral("TND"));
    combo->addItem(QStringLiteral("EUR — Euro"), QStringLiteral("EUR"));
    combo->addItem(QStringLiteral("USD — Dollar US"), QStringLiteral("USD"));
}

void populateDateFormatCombo(QComboBox *combo)
{
    combo->clear();
    combo->addItem(QStringLiteral("jj/MM/aaaa (Europe)"), QStringLiteral("dd/MM/yyyy"));
    combo->addItem(QStringLiteral("MM/jj/aaaa (US)"), QStringLiteral("MM/dd/yyyy"));
    combo->addItem(QStringLiteral("aaaa-MM-jj (ISO)"), QStringLiteral("yyyy-MM-dd"));
}

int comboIndexForData(const QComboBox *combo, const QVariant &data)
{
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i) == data)
            return i;
    }
    return 0;
}

} // namespace

QString SettingsWindow::settingsGroup()
{
    return QStringLiteral("LeatherApp");
}

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    resize(860, 640);

    populateAccentCombo(ui->comboAccentColor);
    populateLanguageCombo(ui->comboLanguage);
    populateCurrencyCombo(ui->comboCurrency);
    populateDateFormatCombo(ui->comboDateFormat);

    ui->labelTitle->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 800; color: #3d2410;"));
    ui->labelSubtitle->setStyleSheet(QStringLiteral("color: #6b5d4f; font-size: 11px;"));
    ui->tabWidget->setDocumentMode(true);

    const QString panelStyle = QStringLiteral(
        "QGroupBox {"
        "  font-weight: 700;"
        "  font-size: 11pt;"
        "  border: 1px solid #e0d6c8;"
        "  border-radius: 10px;"
        "  margin-top: 14px;"
        "  padding: 14px 12px 12px 12px;"
        "  background: #fdfbf7;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 6px;"
        "  color: #5d2e06;"
        "}");
    setStyleSheet(QStringLiteral(
                      "QDialog { background-color: #f6f1e8; }"
                      "QTabWidget::pane { border: 1px solid #dcd2c4; border-radius: 8px; background: #fffdf8; }"
                      "QTabBar::tab { padding: 10px 18px; font-weight: 600; }"
                      "QTabBar::tab:selected { background: #fffdf8; color: #5d2e06; }"
                      "QLineEdit, QSpinBox, QComboBox { padding: 6px; border-radius: 6px; border: 1px solid #d8cfc2; }"
                      "QPushButton { padding: 8px 14px; border-radius: 8px; }"
                      "QCheckBox, QRadioButton { spacing: 8px; }")
                  + panelStyle);

    loadFromSettings();

    // Mode clair impose dans l'application: on masque l'option sombre.
    ui->radioThemeDark->setVisible(false);
    ui->radioThemeDark->setEnabled(false);
    ui->radioThemeLight->setVisible(true);
    ui->radioThemeLight->setEnabled(true);

    connect(ui->radioThemeLight, &QRadioButton::toggled, this, [this](bool on) {
        if (on)
            saveAppearanceAndNotify();
    });
    connect(ui->radioThemeDark, &QRadioButton::toggled, this, [this](bool on) {
        if (on)
            saveAppearanceAndNotify();
    });
    connect(ui->comboAccentColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &SettingsWindow::saveAppearanceAndNotify);
    connect(ui->spinFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::saveAppearanceAndNotify);

    connect(ui->checkNotificationsEnabled, &QCheckBox::toggled, this, &SettingsWindow::saveNotificationsAndNotify);
    connect(ui->checkNotifyStockLow, &QCheckBox::toggled, this, &SettingsWindow::saveNotificationsAndNotify);
    connect(ui->checkNotifyOrders, &QCheckBox::toggled, this, &SettingsWindow::saveNotificationsAndNotify);
    connect(ui->checkNotifyClients, &QCheckBox::toggled, this, &SettingsWindow::saveNotificationsAndNotify);

    connect(ui->comboLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWindow::saveLanguageAndNotify);

    connect(ui->checkFaceRecognition, &QCheckBox::toggled, this, &SettingsWindow::saveSecurityPrefs);
    connect(ui->spinSessionTimeout, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::saveSecurityPrefs);
    connect(ui->btnApplyPassword, &QPushButton::clicked, this, &SettingsWindow::onApplyPasswordChange);

    connect(ui->checkAutoSave, &QCheckBox::toggled, this, &SettingsWindow::saveDataPrefs);
    connect(ui->spinAutoSaveInterval, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::saveDataPrefs);

    connect(ui->comboCurrency, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWindow::saveBusinessPrefs);
    connect(ui->comboDateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWindow::saveBusinessPrefs);
    connect(ui->spinStockMinimum, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::saveBusinessPrefs);

    connect(ui->btnProfileChoosePhoto, &QPushButton::clicked, this, &SettingsWindow::onChooseProfilePhoto);
    connect(ui->btnProfileSave, &QPushButton::clicked, this, &SettingsWindow::onSaveProfile);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange && ui)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}

void SettingsWindow::setSessionUsername(const QString &username)
{
    m_sessionUsername = username.trimmed();
}

void SettingsWindow::loadFromSettings()
{
    QSettings s;
    s.beginGroup(settingsGroup());

    const QString accent = s.value(QStringLiteral("appearance/accent"), QStringLiteral("#5d2e06")).toString();
    const int fontPt = s.value(QStringLiteral("appearance/fontPointSize"), 11).toInt();

    {
        QSignalBlocker b1(ui->radioThemeLight);
        QSignalBlocker b2(ui->radioThemeDark);
        ui->radioThemeDark->setChecked(false);
        ui->radioThemeLight->setChecked(true);
    }
    {
        QSignalBlocker b(ui->comboAccentColor);
        int idx = 0;
        for (int i = 0; i < ui->comboAccentColor->count(); ++i) {
            if (ui->comboAccentColor->itemData(i).toString().compare(accent, Qt::CaseInsensitive) == 0) {
                idx = i;
                break;
            }
        }
        ui->comboAccentColor->setCurrentIndex(idx);
    }
    {
        QSignalBlocker b(ui->spinFontSize);
        ui->spinFontSize->setValue(qBound(8, fontPt, 20));
    }

    {
        QSignalBlocker b1(ui->checkNotificationsEnabled);
        QSignalBlocker b2(ui->checkNotifyStockLow);
        QSignalBlocker b3(ui->checkNotifyOrders);
        QSignalBlocker b4(ui->checkNotifyClients);
        ui->checkNotificationsEnabled->setChecked(s.value(QStringLiteral("notifications/enabled"), true).toBool());
        ui->checkNotifyStockLow->setChecked(s.value(QStringLiteral("notifications/stockLow"), true).toBool());
        ui->checkNotifyOrders->setChecked(s.value(QStringLiteral("notifications/orders"), true).toBool());
        ui->checkNotifyClients->setChecked(s.value(QStringLiteral("notifications/clients"), true).toBool());
    }
    const bool master = ui->checkNotificationsEnabled->isChecked();
    ui->checkNotifyStockLow->setEnabled(master);
    ui->checkNotifyOrders->setEnabled(master);
    ui->checkNotifyClients->setEnabled(master);

    {
        QSignalBlocker b(ui->comboLanguage);
        const QString lang = s.value(QStringLiteral("language"), QStringLiteral("fr")).toString();
        ui->comboLanguage->setCurrentIndex(comboIndexForData(ui->comboLanguage, lang));
    }

    {
        QSignalBlocker b1(ui->checkFaceRecognition);
        QSignalBlocker b2(ui->spinSessionTimeout);
        ui->checkFaceRecognition->setChecked(s.value(QStringLiteral("security/faceRecognition"), true).toBool());
        ui->spinSessionTimeout->setValue(qBound(5, s.value(QStringLiteral("security/sessionTimeoutMinutes"), 30).toInt(), 480));
    }

    {
        QSignalBlocker b1(ui->checkAutoSave);
        QSignalBlocker b2(ui->spinAutoSaveInterval);
        ui->checkAutoSave->setChecked(s.value(QStringLiteral("data/autoSave"), false).toBool());
        ui->spinAutoSaveInterval->setValue(qBound(1, s.value(QStringLiteral("data/autoSaveIntervalMinutes"), 5).toInt(), 60));
    }
    ui->spinAutoSaveInterval->setEnabled(ui->checkAutoSave->isChecked());

    {
        QSignalBlocker b1(ui->comboCurrency);
        QSignalBlocker b2(ui->comboDateFormat);
        QSignalBlocker b3(ui->spinStockMinimum);
        const QString cur = s.value(QStringLiteral("business/currency"), QStringLiteral("TND")).toString();
        ui->comboCurrency->setCurrentIndex(comboIndexForData(ui->comboCurrency, cur));
        const QString df = s.value(QStringLiteral("business/dateFormat"), QStringLiteral("dd/MM/yyyy")).toString();
        ui->comboDateFormat->setCurrentIndex(comboIndexForData(ui->comboDateFormat, df));
        ui->spinStockMinimum->setValue(qMax(0, s.value(QStringLiteral("business/stockMinimum"), 5).toInt()));
    }

    m_profilePhotoPath = s.value(QStringLiteral("profile/photoPath")).toString();
    {
        QSignalBlocker b1(ui->editProfileDisplayName);
        QSignalBlocker b2(ui->editProfileEmail);
        ui->editProfileDisplayName->setText(s.value(QStringLiteral("profile/displayName")).toString());
        ui->editProfileEmail->setText(s.value(QStringLiteral("profile/email")).toString());
    }
    if (!m_profilePhotoPath.isEmpty() && QFileInfo::exists(m_profilePhotoPath)) {
        const QPixmap pm(m_profilePhotoPath);
        if (!pm.isNull())
            ui->labelProfilePhotoPreview->setPixmap(
                pm.scaled(ui->labelProfilePhotoPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelProfilePhotoPreview->setText(QStringLiteral("—"));
        ui->labelProfilePhotoPreview->setPixmap(QPixmap());
    }

    s.endGroup();
}

void SettingsWindow::saveAppearanceAndNotify()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("appearance/theme"), QStringLiteral("light"));
    if (ui->comboAccentColor->currentIndex() >= 0)
        s.setValue(QStringLiteral("appearance/accent"), ui->comboAccentColor->currentData().toString());
    s.setValue(QStringLiteral("appearance/fontPointSize"), ui->spinFontSize->value());
    s.sync();
    s.endGroup();
    emit appearanceChanged();
}

void SettingsWindow::saveNotificationsAndNotify()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("notifications/enabled"), ui->checkNotificationsEnabled->isChecked());
    s.setValue(QStringLiteral("notifications/stockLow"), ui->checkNotifyStockLow->isChecked());
    s.setValue(QStringLiteral("notifications/orders"), ui->checkNotifyOrders->isChecked());
    s.setValue(QStringLiteral("notifications/clients"), ui->checkNotifyClients->isChecked());
    s.sync();
    s.endGroup();

    const bool master = ui->checkNotificationsEnabled->isChecked();
    ui->checkNotifyStockLow->setEnabled(master);
    ui->checkNotifyOrders->setEnabled(master);
    ui->checkNotifyClients->setEnabled(master);
    emit notificationSettingsChanged();
}

void SettingsWindow::saveLanguageAndNotify()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    const QString code = ui->comboLanguage->currentData().toString();
    s.setValue(QStringLiteral("language"), code);
    s.sync();
    s.endGroup();
    emit languageChanged(code);
}

void SettingsWindow::saveSecurityPrefs()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("security/faceRecognition"), ui->checkFaceRecognition->isChecked());
    s.setValue(QStringLiteral("security/sessionTimeoutMinutes"), ui->spinSessionTimeout->value());
    s.sync();
    s.endGroup();
    emit securitySettingsChanged();
}

void SettingsWindow::saveDataPrefs()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("data/autoSave"), ui->checkAutoSave->isChecked());
    s.setValue(QStringLiteral("data/autoSaveIntervalMinutes"), ui->spinAutoSaveInterval->value());
    s.sync();
    s.endGroup();
    ui->spinAutoSaveInterval->setEnabled(ui->checkAutoSave->isChecked());
}

void SettingsWindow::saveBusinessPrefs()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("business/currency"), ui->comboCurrency->currentData().toString());
    s.setValue(QStringLiteral("business/dateFormat"), ui->comboDateFormat->currentData().toString());
    s.setValue(QStringLiteral("business/stockMinimum"), ui->spinStockMinimum->value());
    s.sync();
    s.endGroup();
    emit businessPreferencesChanged();
}

void SettingsWindow::onChooseProfilePhoto()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choisir une photo"),
        QString(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp);;Tous les fichiers (*)"));
    if (path.isEmpty())
        return;
    m_profilePhotoPath = path;
    const QPixmap pm(path);
    if (!pm.isNull()) {
        ui->labelProfilePhotoPreview->setPixmap(
            pm.scaled(ui->labelProfilePhotoPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->labelProfilePhotoPreview->setText(QString());
    }
}

void SettingsWindow::onSaveProfile()
{
    QSettings s;
    s.beginGroup(settingsGroup());
    s.setValue(QStringLiteral("profile/displayName"), ui->editProfileDisplayName->text().trimmed());
    s.setValue(QStringLiteral("profile/email"), ui->editProfileEmail->text().trimmed());
    s.setValue(QStringLiteral("profile/photoPath"), m_profilePhotoPath);
    s.sync();
    s.endGroup();
    QMessageBox::information(this, QStringLiteral("Profil"), QStringLiteral("Profil enregistré."));
    emit profileChanged();
}

void SettingsWindow::onApplyPasswordChange()
{
    if (m_sessionUsername.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Mot de passe"),
                             QStringLiteral("Aucun utilisateur de session n'est défini. Connectez-vous puis rouvrez les paramètres."));
        return;
    }
    const QString oldPw = ui->editPasswordCurrent->text();
    const QString newPw = ui->editPasswordNew->text();
    const QString confirm = ui->editPasswordConfirm->text();
    if (newPw.size() < 4) {
        QMessageBox::warning(this, QStringLiteral("Mot de passe"),
                             QStringLiteral("Le nouveau mot de passe doit contenir au moins 4 caractères."));
        return;
    }
    if (newPw != confirm) {
        QMessageBox::warning(this, QStringLiteral("Mot de passe"),
                             QStringLiteral("La confirmation ne correspond pas au nouveau mot de passe."));
        return;
    }
    emit passwordChangeRequested(m_sessionUsername, oldPw, newPw);
}

void SettingsWindow::clearPasswordFields()
{
    ui->editPasswordCurrent->clear();
    ui->editPasswordNew->clear();
    ui->editPasswordConfirm->clear();
}
