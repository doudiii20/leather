#ifndef CHATBOTWINDOW_H
#define CHATBOTWINDOW_H

#include <QByteArray>
#include <QDialog>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class ChatManager;

/// Fenêtre de chat locale (Ollama via ChatManager), indépendante du reste de l’application.
class ChatbotWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit ChatbotWindow(QWidget *parent = nullptr);

private slots:
    void onSendClicked();
    void onDownloadImageClicked();

private:
    /// QTextEdit traite souvent plusieurs insertHtml() comme du flux inline ; on force un nouveau bloc.
    void insertHtmlAsNewBlock(const QString &html);

    void appendMessage(const QString &prefix, const QString &body);
    void appendImageMessage(const QString &prefix, const QByteArray &imageData, const QString &prompt);
    QString renderImageBubbleHtml(const QByteArray &imageData) const;

    ChatManager *m_chatManager = nullptr;
    QTextEdit *m_log = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_send = nullptr;
    QPushButton *m_downloadImage = nullptr;
    QLabel *m_status = nullptr;
    QByteArray m_lastGeneratedImage;
};

#endif // CHATBOTWINDOW_H
