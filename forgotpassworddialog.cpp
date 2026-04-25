#include "forgotpassworddialog.h"
#include "smtpsender.h"

#include <QCryptographicHash>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QVBoxLayout>

namespace {

const int kCodeLength = 6;
const int kCodeValiditySeconds = 300;
const int kResendCooldownSeconds = 60;
const int kMinPasswordLength = 8;
const int kMaxAttempts = 3;
const int kBlockSeconds = 60;

} // namespace

ForgotPasswordDialog::ForgotPasswordDialog(QSqlDatabase db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
    , m_resendTimer(new QTimer(this))
{
    setWindowTitle(QStringLiteral("Mot de passe oublie (Employes)"));
    setModal(true);
    resize(500, 340);

    buildUi();

    m_resendTimer->setInterval(1000);
    connect(m_resendTimer, &QTimer::timeout, this, &ForgotPasswordDialog::onResendTimeoutTick);

    connect(m_sendCodeButton, &QPushButton::clicked, this, &ForgotPasswordDialog::onSendCodeClicked);
    connect(m_verifyCodeButton, &QPushButton::clicked, this, &ForgotPasswordDialog::onVerifyCodeClicked);
    connect(m_resendCodeButton, &QPushButton::clicked, this, &ForgotPasswordDialog::onSendCodeClicked);
    connect(m_confirmPasswordButton, &QPushButton::clicked, this, &ForgotPasswordDialog::onConfirmPasswordClicked);

    connect(m_emailEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::refreshUiState);
    connect(m_codeEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::refreshUiState);
    connect(m_newPasswordEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::refreshUiState);
    connect(m_confirmPasswordEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::refreshUiState);

    setCurrentStep(Step::EmailEntry);
    refreshUiState();
}

ForgotPasswordDialog::~ForgotPasswordDialog() = default;

void ForgotPasswordDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("Réinitialisation du mot de passe"), this);
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #2f3542;"));
    mainLayout->addWidget(title);

    auto *subtitle = new QLabel(QStringLiteral("Étape 1: saisissez votre e-mail puis recevez un code de vérification."), this);
    subtitle->setWordWrap(true);
    subtitle->setObjectName(QStringLiteral("subtitleLabel"));
    subtitle->setStyleSheet(QStringLiteral("color: #5f6368;"));
    mainLayout->addWidget(subtitle);

    m_emailGroup = new QGroupBox(QStringLiteral("1) E-mail"), this);
    auto *emailLayout = new QFormLayout(m_emailGroup);
    m_emailEdit = new QLineEdit(m_emailGroup);
    m_emailEdit->setPlaceholderText(QStringLiteral("utilisateur@domaine.com"));
    m_sendCodeButton = new QPushButton(QStringLiteral("Envoyer code"), m_emailGroup);
    auto *emailRow = new QHBoxLayout();
    emailRow->addWidget(m_emailEdit, 1);
    emailRow->addWidget(m_sendCodeButton);
    emailLayout->addRow(QStringLiteral("Adresse e-mail:"), emailRow);

    m_codeGroup = new QGroupBox(QStringLiteral("2) Vérification du code"), this);
    auto *codeLayout = new QFormLayout(m_codeGroup);
    m_codeInfoLabel = new QLabel(QStringLiteral("Entrez le code à 6 chiffres envoyé par e-mail."), m_codeGroup);
    m_codeInfoLabel->setWordWrap(true);
    m_blockedLabel = new QLabel(QStringLiteral("Compte temporairement bloque."), m_codeGroup);
    m_blockedLabel->setStyleSheet(QStringLiteral("color: #b00020; font-weight: 700;"));
    m_blockedLabel->setVisible(false);
    m_codeEdit = new QLineEdit(m_codeGroup);
    m_codeEdit->setPlaceholderText(QStringLiteral("123456"));
    m_codeEdit->setMaxLength(kCodeLength);
    m_verifyCodeButton = new QPushButton(QStringLiteral("Vérifier"), m_codeGroup);
    m_resendCodeButton = new QPushButton(QStringLiteral("Renvoyer code"), m_codeGroup);
    m_resendTimerLabel = new QLabel(m_codeGroup);

    auto *codeButtonRow = new QHBoxLayout();
    codeButtonRow->addWidget(m_verifyCodeButton);
    codeButtonRow->addWidget(m_resendCodeButton);
    codeButtonRow->addStretch();

    codeLayout->addRow(m_codeInfoLabel);
    codeLayout->addRow(m_blockedLabel);
    codeLayout->addRow(QStringLiteral("Code reçu:"), m_codeEdit);
    codeLayout->addRow(codeButtonRow);
    codeLayout->addRow(QStringLiteral("Anti-spam:"), m_resendTimerLabel);

    m_passwordGroup = new QGroupBox(QStringLiteral("3) Nouveau mot de passe"), this);
    auto *passwordLayout = new QFormLayout(m_passwordGroup);
    m_newPasswordEdit = new QLineEdit(m_passwordGroup);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setPlaceholderText(QStringLiteral("Minimum 8 caractères"));
    m_confirmPasswordEdit = new QLineEdit(m_passwordGroup);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText(QStringLiteral("Répéter le mot de passe"));
    m_confirmPasswordButton = new QPushButton(QStringLiteral("Confirmer"), m_passwordGroup);

    passwordLayout->addRow(QStringLiteral("Nouveau mot de passe:"), m_newPasswordEdit);
    passwordLayout->addRow(QStringLiteral("Confirmation:"), m_confirmPasswordEdit);
    passwordLayout->addRow(QString(), m_confirmPasswordButton);

    mainLayout->addWidget(m_emailGroup);
    mainLayout->addWidget(m_codeGroup);
    mainLayout->addWidget(m_passwordGroup);
    mainLayout->addStretch();
}

void ForgotPasswordDialog::setCurrentStep(Step step)
{
    m_currentStep = step;
    const bool emailStep = (step == Step::EmailEntry);
    const bool codeStep = (step == Step::CodeVerification);
    const bool passwordStep = (step == Step::NewPassword);

    m_emailGroup->setVisible(emailStep || codeStep || passwordStep);
    m_codeGroup->setVisible(codeStep || passwordStep);
    m_passwordGroup->setVisible(passwordStep);

    QLabel *subtitle = findChild<QLabel *>(QStringLiteral("subtitleLabel"));
    if (!subtitle)
        return;

    if (emailStep) {
        subtitle->setText(QStringLiteral("Étape 1: saisissez votre e-mail puis recevez un code de vérification."));
    } else if (codeStep) {
        subtitle->setText(QStringLiteral("Étape 2: entrez le code reçu pour valider votre identité."));
    } else {
        subtitle->setText(QStringLiteral("Étape 3: choisissez un nouveau mot de passe sécurisé."));
    }
}

bool ForgotPasswordDialog::isValidEmail(const QString &email)
{
    static const QRegularExpression emailRegex(
        QStringLiteral("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$"),
        QRegularExpression::CaseInsensitiveOption);
    return emailRegex.match(email.trimmed()).hasMatch();
}

QString ForgotPasswordDialog::hashPasswordSha256(const QString &plainPassword)
{
    return QString::fromLatin1(QCryptographicHash::hash(plainPassword.toUtf8(), QCryptographicHash::Sha256).toHex());
}

int ForgotPasswordDialog::generateSixDigitCode()
{
    return QRandomGenerator::global()->bounded(100000, 1000000);
}

bool ForgotPasswordDialog::isBlockedNow() const
{
    return m_blockTime.isValid() && QDateTime::currentDateTime() < m_blockTime;
}

QString ForgotPasswordDialog::employeeTableName(QString *errorOut) const
{
    if (!m_db.isOpen()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Connexion base de donnees indisponible.");
        }
        return QString();
    }

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT TABLE_NAME FROM USER_TABLES "
        "WHERE TABLE_NAME IN ('EMPLOYEE','EMPLOYES') "
        "ORDER BY CASE WHEN TABLE_NAME='EMPLOYEE' THEN 0 ELSE 1 END"));
    if (!q.exec()) {
        if (errorOut) {
            *errorOut = q.lastError().text().trimmed();
        }
        return QString();
    }
    if (!q.next()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Aucune table employee/employes trouvee.");
        }
        return QString();
    }
    return q.value(0).toString().trimmed();
}

void ForgotPasswordDialog::startResendCooldown(int seconds)
{
    m_resendSecondsLeft = seconds;
    if (!m_resendTimer->isActive()) {
        m_resendTimer->start();
    }
    refreshUiState();
}

void ForgotPasswordDialog::onResendTimeoutTick()
{
    if (m_resendSecondsLeft > 0) {
        --m_resendSecondsLeft;
    }
    if (m_resendSecondsLeft <= 0) {
        m_resendTimer->stop();
    }
    refreshUiState();
}

void ForgotPasswordDialog::refreshUiState()
{
    const bool blocked = isBlockedNow();
    const bool hasValidEmail = isValidEmail(m_emailEdit->text());
    m_sendCodeButton->setEnabled(hasValidEmail && !m_isSendInProgress && m_resendSecondsLeft <= 0 && !blocked);

    const bool canVerifyCode = (m_codeEdit->text().trimmed().size() == kCodeLength) && (m_resetCode >= 100000);
    m_verifyCodeButton->setEnabled(canVerifyCode && m_currentStep != Step::EmailEntry && !blocked);

    m_resendCodeButton->setEnabled(m_currentStep != Step::EmailEntry && !m_isSendInProgress && m_resendSecondsLeft <= 0 && !blocked);
    if (blocked) {
        const int left = QDateTime::currentDateTime().secsTo(m_blockTime);
        m_resendTimerLabel->setText(QStringLiteral("Trop de tentatives. Verification bloquee pendant %1 s.").arg(qMax(0, left)));
        if (m_blockedLabel)
            m_blockedLabel->setText(QStringLiteral("Compte temporairement bloque (%1 s restantes).").arg(qMax(0, left)));
    } else if (m_resendSecondsLeft > 0) {
        m_resendTimerLabel->setText(QStringLiteral("Vous pouvez renvoyer dans %1 s.").arg(m_resendSecondsLeft));
    } else {
        m_resendTimerLabel->setText(QStringLiteral("Vous pouvez renvoyer un code maintenant."));
    }
    if (m_blockedLabel)
        m_blockedLabel->setVisible(blocked);

    const bool hasPassword = !m_newPasswordEdit->text().isEmpty();
    const bool hasConfirm = !m_confirmPasswordEdit->text().isEmpty();
    m_confirmPasswordButton->setEnabled(hasPassword && hasConfirm && m_currentStep == Step::NewPassword);
}

bool ForgotPasswordDialog::lookupEmployeCinByEmail(const QString &email, QString *cinOut, QString *errorOut) const
{
    if (!m_db.isOpen()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Connexion base de données non disponible.");
        }
        return false;
    }

    const QString tableName = employeeTableName(errorOut);
    if (tableName.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT CIN FROM %1 "
        "WHERE LOWER(TRIM(EMAIL)) = LOWER(TRIM(:email)) "
        "AND EMAIL IS NOT NULL").arg(tableName));
    query.bindValue(QStringLiteral(":email"), email.trimmed());

    if (!query.exec()) {
        if (errorOut) {
            *errorOut = query.lastError().text();
        }
        return false;
    }

    if (!query.next()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Aucun employe trouve pour cet e-mail.");
        }
        return false;
    }

    const QString cin = query.value(0).toString().trimmed();
    if (cin.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Aucun employe trouve pour cet e-mail.");
        }
        return false;
    }

    if (cinOut) {
        *cinOut = cin;
    }
    return true;
}

bool ForgotPasswordDialog::ensureEmployePasswordColumn(QString *errorOut) const
{
    if (!m_db.isOpen()) {
        if (errorOut)
            *errorOut = QStringLiteral("Connexion base de donnees indisponible.");
        return false;
    }

    const QString tableName = employeeTableName(errorOut);
    if (tableName.isEmpty()) {
        return false;
    }

    QSqlQuery check(m_db);
    if (!check.exec(QStringLiteral(
            "SELECT COUNT(*) FROM USER_TAB_COLUMNS "
            "WHERE TABLE_NAME = '%1' AND COLUMN_NAME = 'PASSWORD_HASH'").arg(tableName))
        || !check.next()) {
        if (errorOut)
            *errorOut = check.lastError().text().trimmed();
        return false;
    }
    if (check.value(0).toInt() > 0)
        return true;

    QSqlQuery alter(m_db);
    if (!alter.exec(QStringLiteral("ALTER TABLE %1 ADD (PASSWORD_HASH VARCHAR2(128))").arg(tableName))) {
        if (errorOut)
            *errorOut = alter.lastError().text().trimmed();
        return false;
    }
    return true;
}

bool ForgotPasswordDialog::sendCodeByEmail(const QString &recipientEmail, const QString &code, QString *errorOut) const
{
    bool ok = false;
    const int codeInt = code.toInt(&ok);
    if (!ok) {
        if (errorOut) {
            *errorOut = QStringLiteral("Code de verification invalide.");
        }
        return false;
    }
    return LeatherSmtp::sendResetCode(recipientEmail, codeInt, errorOut);
}

void ForgotPasswordDialog::onSendCodeClicked()
{
    if (isBlockedNow()) {
        const int left = QDateTime::currentDateTime().secsTo(m_blockTime);
        QMessageBox::warning(this, QStringLiteral("Verification bloquee"),
                             QStringLiteral("Trop de tentatives incorrectes. Reessayez dans %1 secondes.").arg(qMax(0, left)));
        refreshUiState();
        return;
    }

    const QString email = m_emailEdit->text().trimmed();
    if (!isValidEmail(email)) {
        QMessageBox::warning(this, QStringLiteral("E-mail invalide"),
                             QStringLiteral("Veuillez saisir une adresse e-mail valide."));
        return;
    }

    QString employeeCin;
    QString lookupError;
    if (!lookupEmployeCinByEmail(email, &employeeCin, &lookupError)) {
        QMessageBox::warning(this, QStringLiteral("Employe introuvable"),
                             lookupError.isEmpty() ? QStringLiteral("Aucun employe lie a cet e-mail.") : lookupError);
        return;
    }

    m_pendingEmail = email;
    m_pendingEmployeeCin = employeeCin;
    m_resetCode = generateSixDigitCode();
    m_attempts = 0;
    m_codeTime = QDateTime::currentDateTime();
    m_blockTime = QDateTime();
    m_isSendInProgress = true;
    refreshUiState();

    QString smtpErr;
    if (!sendCodeByEmail(email, QString::number(m_resetCode), &smtpErr)) {
        m_isSendInProgress = false;
        refreshUiState();
        QMessageBox::critical(
            this,
            QStringLiteral("Erreur SMTP Gmail"),
            QStringLiteral("Echec de l'envoi du code.\n%1\n\n"
                           "Configuration requise:\n"
                           "- LEATHER_SMTP_RESET_USER = votre Gmail de recuperation\n"
                           "- LEATHER_SMTP_RESET_PASS = mot de passe d'application Google\n"
                           "- (ou fallback) LEATHER_SMTP_USER / LEATHER_SMTP_PASS\n"
                           "- LEATHER_SMTP_HOST = smtp.gmail.com\n"
                           "- LEATHER_SMTP_PORT = 587")
                .arg(smtpErr));
        return;
    }

    m_isSendInProgress = false;
    setCurrentStep(Step::CodeVerification);
    startResendCooldown(kResendCooldownSeconds);
    m_codeEdit->clear();
    QMessageBox::information(this, QStringLiteral("Code envoyé"),
                             QStringLiteral("Un code de vérification a été envoyé à %1.").arg(m_pendingEmail));
    refreshUiState();
}

void ForgotPasswordDialog::onVerifyCodeClicked()
{
    if (isBlockedNow()) {
        const int left = QDateTime::currentDateTime().secsTo(m_blockTime);
        QMessageBox::warning(this, QStringLiteral("Verification bloquee"),
                             QStringLiteral("Verification temporairement bloquee. Reessayez dans %1 secondes.").arg(qMax(0, left)));
        refreshUiState();
        return;
    }

    if (!m_codeTime.isValid() || m_resetCode < 100000) {
        QMessageBox::warning(this, QStringLiteral("Code manquant"),
                             QStringLiteral("Veuillez d'abord demander un code de recuperation."));
        return;
    }

    const QString enteredCode = m_codeEdit->text().trimmed();
    if (QDateTime::currentDateTime() > m_codeTime.addSecs(kCodeValiditySeconds)) {
        QMessageBox::warning(this, QStringLiteral("Code expire"),
                             QStringLiteral("Code expire. Veuillez demander un nouveau code."));
        return;
    }

    bool ok = false;
    const int entered = enteredCode.toInt(&ok);
    if (!ok || entered != m_resetCode) {
        ++m_attempts;
        if (m_attempts >= kMaxAttempts) {
            m_blockTime = QDateTime::currentDateTime().addSecs(kBlockSeconds);
            QMessageBox::critical(this, QStringLiteral("Blocage temporaire"),
                                  QStringLiteral("3 tentatives incorrectes. Verification bloquee pendant %1 secondes.")
                                      .arg(kBlockSeconds));
        } else {
            QMessageBox::warning(this, QStringLiteral("Code incorrect"),
                                 QStringLiteral("Code incorrect. Tentatives restantes : %1.")
                                     .arg(kMaxAttempts - m_attempts));
        }
        refreshUiState();
        return;
    }

    m_attempts = 0;
    m_blockTime = QDateTime();
    setCurrentStep(Step::NewPassword);
    m_newPasswordEdit->clear();
    m_confirmPasswordEdit->clear();
    refreshUiState();
}

bool ForgotPasswordDialog::updateEmployePasswordInDatabase(const QString &cin, const QString &plainPassword, QString *errorOut) const
{
    if (!m_db.isOpen()) {
        if (errorOut) {
            *errorOut = QStringLiteral("Connexion base de données indisponible.");
        }
        return false;
    }

    const QString tableName = employeeTableName(errorOut);
    if (tableName.isEmpty()) {
        return false;
    }

    QSqlQuery updateQuery(m_db);
    updateQuery.prepare(QStringLiteral(
        "UPDATE %1 "
        "SET PASSWORD_HASH = :password "
        "WHERE UPPER(CIN) = UPPER(:cin)").arg(tableName));
    updateQuery.bindValue(QStringLiteral(":password"), hashPasswordSha256(plainPassword));
    updateQuery.bindValue(QStringLiteral(":cin"), cin);

    if (!updateQuery.exec()) {
        if (errorOut) {
            *errorOut = updateQuery.lastError().text();
        }
        return false;
    }

    if (updateQuery.numRowsAffected() <= 0) {
        if (errorOut) {
            *errorOut = QStringLiteral("Employe introuvable pendant la mise a jour.");
        }
        return false;
    }

    return true;
}

void ForgotPasswordDialog::onConfirmPasswordClicked()
{
    const QString newPassword = m_newPasswordEdit->text();
    const QString confirmPassword = m_confirmPasswordEdit->text();

    if (newPassword.size() < kMinPasswordLength) {
        QMessageBox::warning(this, QStringLiteral("Mot de passe invalide"),
                             QStringLiteral("Le mot de passe doit contenir au moins %1 caractères.").arg(kMinPasswordLength));
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, QStringLiteral("Confirmation invalide"),
                             QStringLiteral("Les deux mots de passe ne correspondent pas."));
        return;
    }

    QString schemaErr;
    if (!ensureEmployePasswordColumn(&schemaErr)) {
        QMessageBox::critical(this, QStringLiteral("Schema employes"),
                              QStringLiteral("Impossible de preparer la colonne PASSWORD_HASH.\n%1").arg(schemaErr));
        return;
    }

    QString dbError;
    if (!updateEmployePasswordInDatabase(m_pendingEmployeeCin, newPassword, &dbError)) {
        QMessageBox::critical(this, QStringLiteral("Mise à jour échouée"),
                              QStringLiteral("Impossible de mettre à jour le mot de passe.\n%1").arg(dbError));
        return;
    }

    QMessageBox::information(this, QStringLiteral("Succès"),
                             QStringLiteral("Votre mot de passe a été réinitialisé avec succès."));
    accept();
}
