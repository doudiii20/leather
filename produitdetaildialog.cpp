#include "produitdetaildialog.h"

#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {
// Fiche produit GitHub Pages (depot doudiii20/leather, dossier /docs).
const QString kProductPageUrl = QStringLiteral("https://doudiii20.github.io/leather/produit.html");

// Chemin relatif web (images/catalogue/...) copie sous docs/images sur GitHub Pages.
QString catalogueImageWebPath(const QString &imagePath)
{
    if (imagePath.trimmed().isEmpty())
        return QString();
    QString p = imagePath.trimmed();
    if (p.startsWith(QLatin1String(":/")))
        p = p.mid(2);
    else if (p.startsWith(QLatin1Char('/')))
        p = p.mid(1);
    if (p.startsWith(QLatin1String("images/catalogue/")))
        return p;
    return QString();
}
} // namespace

ProduitDetailDialog::ProduitDetailDialog(const Catalogue::Produit &produit, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Détail produit"));
    setModal(true);
    resize(560, 360);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #f6efe4; }"
        "QLabel#titleLabel { color: #5d2e06; font-size: 20px; font-weight: 700; }"
        "QLabel#priceLabel { color: #7a4d2b; font-size: 16px; font-weight: 700; }"
        "QLabel#metaLabel { color: #3b2b1e; font-size: 13px; }"
        "QPushButton { border-radius: 8px; padding: 8px 14px; font-weight: 600; }"
        "QPushButton#orderButton { background: #5d2e06; color: #ffffff; border: none; }"
        "QPushButton#orderButton:hover { background: #6f3708; }"
        "QPushButton#orderButton:pressed { background: #4d2504; }"
        "QPushButton#closeButton { background: #ffffff; color: #5d2e06; border: 1px solid #d8c8b2; }"
        "QPushButton#closeButton:hover { background: #fff4e9; border-color: #c08a5b; }"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto *container = new QWidget(scroll);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *containerLay = new QVBoxLayout(container);
    containerLay->setContentsMargins(20, 20, 20, 20);
    containerLay->setSpacing(10);

    auto *content = new QWidget(container);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *contentLay = new QHBoxLayout(content);
    contentLay->setContentsMargins(0, 0, 0, 0);
    contentLay->setSpacing(16);

    auto *previewLabel = new QLabel(content);
    previewLabel->setMinimumSize(200, 200);
    previewLabel->setMaximumWidth(260);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setPixmap(buildPreviewPixmap(produit, QSize(220, 220)));
    previewLabel->setStyleSheet(QStringLiteral("border-radius: 12px; border: 1px solid #e0d4c4; background: #ffffff;"));
    contentLay->addWidget(previewLabel, 1, Qt::AlignTop);

    auto *infoWrap = new QWidget(content);
    infoWrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *infoLay = new QVBoxLayout(infoWrap);
    infoLay->setContentsMargins(0, 0, 0, 0);
    infoLay->setSpacing(10);

    auto *title = new QLabel(produit.nom, infoWrap);
    title->setObjectName(QStringLiteral("titleLabel"));
    title->setWordWrap(true);
    infoLay->addWidget(title);

    auto *price = new QLabel(QStringLiteral("%1 TND").arg(produit.prix, 0, 'f', 2), infoWrap);
    price->setObjectName(QStringLiteral("priceLabel"));
    infoLay->addWidget(price);

    auto *metaForm = new QFormLayout();
    metaForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    metaForm->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    metaForm->setHorizontalSpacing(10);
    metaForm->setVerticalSpacing(8);

    auto *categoryValue = new QLabel(produit.categorie, infoWrap);
    categoryValue->setObjectName(QStringLiteral("metaLabel"));
    auto *stockValue = new QLabel(QString::number(produit.stock), infoWrap);
    stockValue->setObjectName(QStringLiteral("metaLabel"));
    auto *statusValue = new QLabel(stockStatusText(produit.stock), infoWrap);
    statusValue->setStyleSheet(stockStatusStyle(produit.stock));
    statusValue->setContentsMargins(8, 4, 8, 4);

    auto *imgPathValue = new QLabel(produit.imagePath.isEmpty() ? QStringLiteral("Aucune image") : produit.imagePath, infoWrap);
    imgPathValue->setWordWrap(true);
    imgPathValue->setObjectName(QStringLiteral("metaLabel"));
    auto *descValue = new QLabel(produit.description.isEmpty() ? QStringLiteral("-") : produit.description, infoWrap);
    descValue->setWordWrap(true);
    descValue->setObjectName(QStringLiteral("metaLabel"));

    metaForm->addRow(QStringLiteral("Catégorie :"), categoryValue);
    metaForm->addRow(QStringLiteral("Stock :"), stockValue);
    metaForm->addRow(QStringLiteral("Statut :"), statusValue);
    metaForm->addRow(QStringLiteral("Description :"), descValue);
    metaForm->addRow(QStringLiteral("Image :"), imgPathValue);
    infoLay->addLayout(metaForm);
    auto *actions = new QHBoxLayout();
    actions->addStretch(1);
    auto *closeBtn = new QPushButton(QStringLiteral("Fermer"), infoWrap);
    closeBtn->setObjectName(QStringLiteral("closeButton"));
    auto *qrBtn = new QPushButton(QStringLiteral("Afficher QR Code"), infoWrap);
    qrBtn->setObjectName(QStringLiteral("closeButton"));
    auto *orderBtn = new QPushButton(QStringLiteral("Commander"), infoWrap);
    orderBtn->setObjectName(QStringLiteral("orderButton"));
    actions->addWidget(closeBtn);
    actions->addWidget(qrBtn);
    actions->addWidget(orderBtn);
    infoLay->addLayout(actions);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(qrBtn, &QPushButton::clicked, this, [this, produit]() {
        showQRCodeDialog(produit);
    });
    connect(orderBtn, &QPushButton::clicked, this, [this, produit]() {
        QMessageBox::information(this,
                                 QStringLiteral("Commande"),
                                 QStringLiteral("Produit \"%1\" ajouté à la commande.").arg(produit.nom));
    });

    infoLay->addStretch(0);
    contentLay->addWidget(infoWrap, 2);

    containerLay->addWidget(content);
    scroll->setWidget(container);
    root->addWidget(scroll);
}

QPixmap ProduitDetailDialog::generateQRCode(const QString &data)
{
    QUrl url(QStringLiteral("https://quickchart.io/qr"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("text"), data);
    query.addQueryItem(QStringLiteral("size"), QStringLiteral("320"));
    query.addQueryItem(QStringLiteral("margin"), QStringLiteral("2"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("png"));
    url.setQuery(query);

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("LeatherQt/1.0"));

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QPixmap qrPixmap;
    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray bytes = reply->readAll();
        qrPixmap.loadFromData(bytes, "PNG");
    }
    reply->deleteLater();
    return qrPixmap;
}

void ProduitDetailDialog::showQRCodeDialog(const Catalogue::Produit &produit)
{
    QUrl productUrl(kProductPageUrl);
    QUrlQuery productQuery;
    productQuery.addQueryItem(QStringLiteral("id"), QString::number(produit.id));
    productQuery.addQueryItem(QStringLiteral("nom"), produit.nom);
    productQuery.addQueryItem(QStringLiteral("prix"), QString::number(produit.prix, 'f', 2));
    const QString webImg = catalogueImageWebPath(produit.imagePath);
    if (!webImg.isEmpty())
        productQuery.addQueryItem(QStringLiteral("img"), webImg);
    productUrl.setQuery(productQuery);

    // Le QR encode une URL directe pour ouverture immédiate sur mobile.
    const QString qrData = productUrl.toString(QUrl::FullyEncoded);
    const QPixmap qrPixmap = generateQRCode(qrData);

    if (qrPixmap.isNull()) {
        QMessageBox::warning(this,
                             QStringLiteral("QR Code"),
                             QStringLiteral("Impossible de générer le QR code. Vérifiez la connexion réseau."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("QR Code Produit"));
    dialog.setModal(true);
    dialog.setMinimumSize(420, 500);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog { background: #f6efe4; }"
        "QLabel#qrTitle { color: #5d2e06; font-size: 18px; font-weight: 700; }"
        "QLabel#qrMeta { color: #6a5547; font-size: 12px; }"
        "QLabel#qrFrame { background: #ffffff; border: 1px solid #e0d4c4; border-radius: 12px; }"
        "QPushButton { border-radius: 8px; padding: 8px 14px; font-weight: 600; }"
        "QPushButton#saveBtn { background: #5d2e06; color: #ffffff; border: none; }"
        "QPushButton#saveBtn:hover { background: #6f3708; }"
        "QPushButton#closeBtn { background: #ffffff; color: #5d2e06; border: 1px solid #d8c8b2; }"
        "QPushButton#closeBtn:hover { background: #fff4e9; border-color: #c08a5b; }"));

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("QR Code du produit"), &dialog);
    title->setObjectName(QStringLiteral("qrTitle"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto *meta = new QLabel(QStringLiteral("Lien article :\n%1").arg(productUrl.toString()), &dialog);
    meta->setObjectName(QStringLiteral("qrMeta"));
    meta->setAlignment(Qt::AlignCenter);
    meta->setWordWrap(true);
    layout->addWidget(meta);

    auto *qrLabel = new QLabel(&dialog);
    qrLabel->setObjectName(QStringLiteral("qrFrame"));
    qrLabel->setAlignment(Qt::AlignCenter);
    qrLabel->setMinimumHeight(320);
    qrLabel->setPixmap(qrPixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(qrLabel);

    auto *actionLayout = new QHBoxLayout();
    actionLayout->addStretch(1);
    auto *closeBtn = new QPushButton(QStringLiteral("Fermer"), &dialog);
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    auto *saveBtn = new QPushButton(QStringLiteral("Sauvegarder PNG"), &dialog);
    saveBtn->setObjectName(QStringLiteral("saveBtn"));
    actionLayout->addWidget(closeBtn);
    actionLayout->addWidget(saveBtn);
    layout->addLayout(actionLayout);

    QObject::connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(saveBtn, &QPushButton::clicked, &dialog, [this, &dialog, qrPixmap, produit]() {
        const QString defaultName = QStringLiteral("qr_produit_%1.png").arg(produit.id);
        const QString filePath = QFileDialog::getSaveFileName(&dialog,
                                                              QStringLiteral("Enregistrer le QR code"),
                                                              defaultName,
                                                              QStringLiteral("Image PNG (*.png)"));
        if (filePath.isEmpty())
            return;

        QString outputPath = filePath;
        if (!outputPath.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive))
            outputPath += QStringLiteral(".png");

        if (qrPixmap.save(outputPath, "PNG")) {
            QMessageBox::information(this,
                                     QStringLiteral("QR Code"),
                                     QStringLiteral("QR code sauvegardé avec succès."));
        } else {
            QMessageBox::warning(this,
                                 QStringLiteral("QR Code"),
                                 QStringLiteral("Échec de sauvegarde du fichier PNG."));
        }
    });

    dialog.exec();
}

QString ProduitDetailDialog::stockStatusText(int stock)
{
    if (stock <= 0)
        return QStringLiteral("Indisponible");
    if (stock <= 5)
        return QStringLiteral("Stock faible");
    return QStringLiteral("Disponible");
}

QString ProduitDetailDialog::stockStatusStyle(int stock)
{
    if (stock <= 0)
        return QStringLiteral("color:#ffffff; background:#c0392b; border-radius:10px; font-weight:700;");
    if (stock <= 5)
        return QStringLiteral("color:#2a2118; background:#f39c12; border-radius:10px; font-weight:700;");
    return QStringLiteral("color:#ffffff; background:#2e7d32; border-radius:10px; font-weight:700;");
}

QPixmap ProduitDetailDialog::buildPreviewPixmap(const Catalogue::Produit &produit, const QSize &size)
{
    QPixmap canvas(size);
    canvas.fill(QColor(QStringLiteral("#f5f5f5")));

    QPixmap pixmap;
    if (!produit.imagePath.trimmed().isEmpty())
        pixmap.load(produit.imagePath);
    if (!pixmap.isNull()) {
        const QPixmap scaled = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const int x = (size.width() - scaled.width()) / 2;
        const int y = (size.height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        return canvas;
    }

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QColor(QStringLiteral("#7a4d2b")));
    QFont f = painter.font();
    f.setPointSize(12);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(canvas.rect().adjusted(12, 12, -12, -12), Qt::AlignCenter | Qt::TextWordWrap, produit.nom);
    return canvas;
}
