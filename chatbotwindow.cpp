#include "chatbotwindow.h"
#include "chatbotreplyformat.h"
#include "chatmanager.h"

#include <QBuffer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

ChatbotWindow::ChatbotWindow(QWidget *parent)
    : QDialog(parent)
    , m_chatManager(new ChatManager(this))
{
    setWindowTitle(QStringLiteral("Chatbot — assistant local (Ollama)"));
    setModal(false);
    resize(520, 560);
    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background: #f7f3ee;"
        "  color: #2a2118;"
        "}"));

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setPlaceholderText(QStringLiteral("Les échanges apparaissent ici…"));
    m_log->setStyleSheet(QStringLiteral(
        "QTextEdit {"
        "  background: #faf8f5;"
        "  color: #1f1f1f;"
        "  border: 1px solid #e0d4c4;"
        "  border-radius: 10px;"
        "  padding: 10px;"
        "  font-size: 12px;"
        "}"));

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("Écrivez votre message…"));
    m_input->setClearButtonEnabled(true);
    m_input->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "  background: #ffffff;"
        "  color: #1f1f1f;"
        "  border: 1px solid #d2d2d2;"
        "  border-radius: 8px;"
        "  padding: 8px 10px;"
        "}"
        "QLineEdit:disabled {"
        "  background: #f0f0f0;"
        "  color: #666666;"
        "}"));

    m_send = new QPushButton(QStringLiteral("Envoyer"), this);
    m_send->setDefault(true);
    m_send->setMinimumWidth(100);
    m_send->setCursor(Qt::PointingHandCursor);
    m_send->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: #5d2e06;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #6f3708; }"
        "QPushButton:pressed { background: #4d2504; }"
        "QPushButton:disabled { background: #b8a894; color: #f5f0e8; }"));

    // Bouton supprimé visuellement: l'image générée reste visible dans le fil de discussion.
    m_downloadImage = nullptr;

    m_status = new QLabel(QStringLiteral("Prêt — assurez-vous qu’Ollama tourne (localhost:11434)."), this);
    m_status->setWordWrap(true);
    m_status->setStyleSheet(QStringLiteral("color: #6b5345; font-size: 11px;"));

    auto *row = new QHBoxLayout();
    row->setSpacing(10);
    row->addWidget(m_input, 1);
    row->addWidget(m_send, 0);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(14, 14, 14, 14);
    mainLay->setSpacing(10);
    mainLay->addWidget(m_log, 1);
    mainLay->addLayout(row);
    mainLay->addWidget(m_status);

    connect(m_send, &QPushButton::clicked, this, &ChatbotWindow::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &ChatbotWindow::onSendClicked);

    connect(m_chatManager, &ChatManager::requestStarted, this, [this](const QString &label) {
        m_status->setText(label);
    });
    connect(m_chatManager, &ChatManager::assistantReplyReady, this, [this](const QString &text) {
        appendMessage(QStringLiteral("Assistant"), text);
    });
    connect(m_chatManager, &ChatManager::imageGeneratedReady, this, [this](const QByteArray &imageData, const QString &prompt) {
        appendImageMessage(QStringLiteral("Assistant"), imageData, prompt);
    });
    connect(m_chatManager, &ChatManager::errorOccurred, this, [this](const QString &message) {
        appendMessage(QStringLiteral("Erreur"), message);
    });
    connect(m_chatManager, &ChatManager::requestFinished, this, [this]() {
        m_status->setText(QStringLiteral("Prêt."));
        m_send->setEnabled(true);
        m_input->setEnabled(true);
    });

}

void ChatbotWindow::insertHtmlAsNewBlock(const QString &html)
{
    if (!m_log)
        return;
    m_log->moveCursor(QTextCursor::End);
    QTextCursor cur = m_log->textCursor();
    cur.movePosition(QTextCursor::End);
    if (cur.position() > 0)
        cur.insertBlock();
    m_log->setTextCursor(cur);
    m_log->insertHtml(html);
    m_log->moveCursor(QTextCursor::End);
}

void ChatbotWindow::appendMessage(const QString &prefix, const QString &body)
{
    const QString safePrefix = prefix.toHtmlEscaped();
    const QString normalizedBody = formatChatbotDialogueLineBreaks(body);
    const QString safeBody =
        normalizedBody.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br/>"));
    const QString html = QStringLiteral(
                             "<p style='margin:8px 0;padding:10px 12px;background:#ffffff;"
                             "border:1px solid #eadfce;border-radius:10px;'>"
                             "<b style='color:#5d2e06;'>%1 :</b> "
                             "<span style='color:#2a2118;'>%2</span>"
                             "</p>")
                             .arg(safePrefix, safeBody);
    insertHtmlAsNewBlock(html);
    insertHtmlAsNewBlock(QStringLiteral("<p style='margin:0;line-height:4px;'>&nbsp;</p>"));
}

void ChatbotWindow::onSendClicked()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty())
        return;

    appendMessage(QStringLiteral("Vous"), text);
    m_input->clear();
    m_send->setEnabled(false);
    m_input->setEnabled(false);

    const QString lower = text.toLower();
    static const QString kPrefix1 = QStringLiteral("generate image:");
    static const QString kPrefix2 = QStringLiteral("image:");
    if (lower.startsWith(kPrefix1) || lower.startsWith(kPrefix2)) {
        QString prompt = text;
        if (lower.startsWith(kPrefix1))
            prompt = text.mid(kPrefix1.size()).trimmed();
        else
            prompt = text.mid(kPrefix2.size()).trimmed();
        m_chatManager->sendImageGeneration(prompt);
        return;
    }

    m_chatManager->sendTextMessage(text);
}

void ChatbotWindow::onDownloadImageClicked()
{
    if (m_lastGeneratedImage.isEmpty())
        return;
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Télécharger image"),
        QStringLiteral("chatbot_generated.png"),
        QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;Tous les fichiers (*)"));
    if (filePath.isEmpty())
        return;
    QPixmap pm;
    if (!pm.loadFromData(m_lastGeneratedImage)) {
        appendMessage(QStringLiteral("Erreur"), QStringLiteral("Impossible de sauvegarder l'image."));
        return;
    }
    if (!pm.save(filePath))
        appendMessage(QStringLiteral("Erreur"), QStringLiteral("Échec de sauvegarde de l'image."));
}

void ChatbotWindow::appendImageMessage(const QString &prefix, const QByteArray &imageData, const QString &prompt)
{
    m_lastGeneratedImage = imageData;
    if (m_downloadImage)
        m_downloadImage->setEnabled(true);

    const QString safePrefix = prefix.toHtmlEscaped();
    const QString safePrompt = prompt.toHtmlEscaped();
    insertHtmlAsNewBlock(QStringLiteral(
                             "<p style='margin:8px 0;padding:10px 12px;background:#ffffff;"
                             "border:1px solid #eadfce;border-radius:10px;'>"
                             "<b style='color:#5d2e06;'>%1 :</b> <span style='color:#2a2118;'>Image générée</span><br/>"
                             "<span style='color:#7a6652;font-size:11px;'>Prompt: %2</span><br/>%3"
                             "</p>")
                             .arg(safePrefix, safePrompt, renderImageBubbleHtml(imageData)));
    insertHtmlAsNewBlock(QStringLiteral("<p style='margin:0;line-height:4px;'>&nbsp;</p>"));
}

QString ChatbotWindow::renderImageBubbleHtml(const QByteArray &imageData) const
{
    QPixmap src;
    if (!src.loadFromData(imageData))
        return QStringLiteral("<i style='color:#9a3b2f;'>Image invalide.</i>");

    const QSize frameSize(200, 200);
    QPixmap canvas(frameSize);
    canvas.fill(QColor(QStringLiteral("#f5f5f5")));
    {
        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const QPixmap scaled = src.scaled(frameSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const int x = (frameSize.width() - scaled.width()) / 2;
        const int y = (frameSize.height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    }

    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    canvas.save(&buffer, "PNG");
    const QString b64 = QString::fromLatin1(pngBytes.toBase64());
    return QStringLiteral(
        "<div style='width:200px;height:200px;border:1px solid #e3d7c8;border-radius:10px;overflow:hidden;background:#f5f5f5;margin-top:6px;'>"
        "<img src='data:image/png;base64,%1' width='200' height='200'/>"
        "</div>")
        .arg(b64);
}
