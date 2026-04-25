#ifndef FORGOTPASSWORDDIALOG_H
#define FORGOTPASSWORDDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QSqlDatabase>

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

class ForgotPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordDialog(QSqlDatabase db = QSqlDatabase::database(), QWidget *parent = nullptr);
    ~ForgotPasswordDialog() override;
    QString recoveredUsername() const { return m_pendingEmployeeCin; }

private slots:
    void onSendCodeClicked();
    void onVerifyCodeClicked();
    void onConfirmPasswordClicked();
    void onResendTimeoutTick();
    void refreshUiState();

private:
    enum class Step {
        EmailEntry = 0,
        CodeVerification,
        NewPassword
    };

    QSqlDatabase m_db;
    QTimer *m_resendTimer = nullptr;
    Step m_currentStep = Step::EmailEntry;
    QString m_pendingEmail;
    QString m_pendingEmployeeCin;
    int m_resetCode = -1;
    int m_attempts = 0;
    QDateTime m_codeTime;
    QDateTime m_blockTime;
    int m_resendSecondsLeft = 0;
    bool m_isSendInProgress = false;

    QGroupBox *m_emailGroup = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QPushButton *m_sendCodeButton = nullptr;

    QGroupBox *m_codeGroup = nullptr;
    QLabel *m_codeInfoLabel = nullptr;
    QLabel *m_blockedLabel = nullptr;
    QLineEdit *m_codeEdit = nullptr;
    QPushButton *m_verifyCodeButton = nullptr;
    QPushButton *m_resendCodeButton = nullptr;
    QLabel *m_resendTimerLabel = nullptr;

    QGroupBox *m_passwordGroup = nullptr;
    QLineEdit *m_newPasswordEdit = nullptr;
    QLineEdit *m_confirmPasswordEdit = nullptr;
    QPushButton *m_confirmPasswordButton = nullptr;

    void buildUi();
    void setCurrentStep(Step step);
    bool sendCodeByEmail(const QString &recipientEmail, const QString &code, QString *errorOut) const;
    QString employeeTableName(QString *errorOut) const;
    bool ensureEmployePasswordColumn(QString *errorOut) const;
    bool lookupEmployeCinByEmail(const QString &email, QString *cinOut, QString *errorOut) const;
    bool updateEmployePasswordInDatabase(const QString &cin, const QString &plainPassword, QString *errorOut) const;
    static bool isValidEmail(const QString &email);
    static QString hashPasswordSha256(const QString &plainPassword);
    static int generateSixDigitCode();
    void startResendCooldown(int seconds);
    bool isBlockedNow() const;
};

#endif // FORGOTPASSWORDDIALOG_H
