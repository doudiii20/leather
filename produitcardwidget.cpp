#include "produitcardwidget.h"

#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QFileInfo>
#include <QVariantAnimation>
#include <QVBoxLayout>

ProduitCardWidget::ProduitCardWidget(const Catalogue::Produit &produit, QWidget *parent)
    : QFrame(parent)
    , m_produit(produit)
{
    setObjectName(QStringLiteral("catalogueProductCard"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(370);
    setMaximumHeight(370);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(QStringLiteral(
        "QFrame#catalogueProductCard {"
        "  background: #ffffff;"
        "  border: 1px solid #eadfce;"
        "  border-radius: 14px;"
        "  padding: 10px;"
        "}"
        "QLabel#nameLabel { color: #3b2b1e; font-size: 14px; font-weight: 700; }"
        "QLabel#metaLabel { color: #6a5547; font-size: 12px; }"
        "QLabel#priceLabel { color: #7a4d2b; font-size: 15px; font-weight: 700; }"
        "QPushButton { border-radius: 8px; padding: 6px 10px; font-weight: 600; }"
        "QPushButton#detailBtn { background: #ffffff; color: #5d2e06; border: 1px solid #d8c8b2; }"
        "QPushButton#detailBtn:hover { background: #fff4e9; border-color: #c08a5b; }"
        "QPushButton#orderBtn { background: #5d2e06; color: #ffffff; border: none; }"
        "QPushButton#orderBtn:hover { background: #6f3708; }"));

    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setBlurRadius(14.0);
    m_shadow->setOffset(0, 2);
    m_shadow->setColor(QColor(0, 0, 0, 25));
    setGraphicsEffect(m_shadow);

    m_shadowAnimation = new QVariantAnimation(this);
    m_shadowAnimation->setDuration(160);
    connect(m_shadowAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_shadow->setBlurRadius(v.toDouble());
    });

    m_paddingAnimation = new QVariantAnimation(this);
    m_paddingAnimation->setDuration(160);
    connect(m_paddingAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        const int h = v.toInt();
        setMinimumHeight(h);
        setMaximumHeight(h);
    });

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    auto *image = new QLabel(this);
    image->setFixedSize(200, 200);
    image->setAlignment(Qt::AlignCenter);
    image->setPixmap(buildPreviewPixmap(m_produit, QSize(200, 200)));
    image->setStyleSheet(QStringLiteral("border-radius: 10px; border: 1px solid #eadfce;"));

    auto *imageWrap = new QWidget(this);
    imageWrap->setFixedSize(200, 200);
    auto *imageWrapLay = new QVBoxLayout(imageWrap);
    imageWrapLay->setContentsMargins(0, 0, 0, 0);
    imageWrapLay->addWidget(image);

    const QString imagePath = m_produit.imagePath.trimmed().replace(QLatin1Char('\\'), QLatin1Char('/'));
    const bool isQrPreview = imagePath.contains(QStringLiteral("/qrcodes/"), Qt::CaseInsensitive)
                             || imagePath.startsWith(QStringLiteral("qrcodes/"), Qt::CaseInsensitive)
                             || QFileInfo(imagePath).fileName().contains(QStringLiteral("qr"), Qt::CaseInsensitive);
    if (isQrPreview) {
        auto *badge = new QLabel(QStringLiteral("QR"), imageWrap);
        badge->setStyleSheet(QStringLiteral(
            "background:#5d2e06; color:#ffffff; font-weight:700; font-size:11px; "
            "border-radius:9px; padding:2px 8px;"));
        badge->adjustSize();
        badge->move(imageWrap->width() - badge->width() - 8, 8);
        badge->raise();
        badge->show();
    }
    lay->addWidget(imageWrap, 0, Qt::AlignHCenter);

    auto *name = new QLabel(m_produit.nom, this);
    name->setObjectName(QStringLiteral("nameLabel"));
    name->setWordWrap(true);
    lay->addWidget(name);

    auto *price = new QLabel(QStringLiteral("%1 TND").arg(m_produit.prix, 0, 'f', 2), this);
    price->setObjectName(QStringLiteral("priceLabel"));
    lay->addWidget(price);

    auto *category = new QLabel(QStringLiteral("Catégorie : %1").arg(m_produit.categorie), this);
    category->setObjectName(QStringLiteral("metaLabel"));
    lay->addWidget(category);

    if (!m_produit.description.trimmed().isEmpty()) {
        auto *description = new QLabel(m_produit.description, this);
        description->setObjectName(QStringLiteral("metaLabel"));
        description->setWordWrap(true);
        description->setToolTip(m_produit.description);
        lay->addWidget(description);
    }

    auto *status = new QLabel(stockStatusText(m_produit.stock), this);
    status->setStyleSheet(stockStatusStyle(m_produit.stock));
    status->setAlignment(Qt::AlignCenter);
    status->setMinimumHeight(24);
    lay->addWidget(status);

    auto *actions = new QHBoxLayout();
    actions->setSpacing(8);
    auto *detailBtn = new QPushButton(QStringLiteral("Voir détail"), this);
    detailBtn->setObjectName(QStringLiteral("detailBtn"));
    auto *orderBtn = new QPushButton(QStringLiteral("Commander"), this);
    orderBtn->setObjectName(QStringLiteral("orderBtn"));
    actions->addWidget(detailBtn, 1);
    actions->addWidget(orderBtn, 1);
    lay->addLayout(actions);

    connect(detailBtn, &QPushButton::clicked, this, [this]() { emit detailRequested(m_produit); });
    connect(orderBtn, &QPushButton::clicked, this, [this]() { emit orderRequested(m_produit); });
}

const Catalogue::Produit &ProduitCardWidget::produit() const
{
    return m_produit;
}

void ProduitCardWidget::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    animateElevation(true);
}

void ProduitCardWidget::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    animateElevation(false);
}

QString ProduitCardWidget::stockStatusText(int stock)
{
    if (stock <= 0)
        return QStringLiteral("Indisponible");
    if (stock <= 5)
        return QStringLiteral("Stock faible");
    return QStringLiteral("Disponible");
}

QString ProduitCardWidget::stockStatusStyle(int stock)
{
    if (stock <= 0)
        return QStringLiteral("QLabel { color:#ffffff; background:#c0392b; border-radius:10px; font-weight:700; }");
    if (stock <= 5)
        return QStringLiteral("QLabel { color:#2a2118; background:#f39c12; border-radius:10px; font-weight:700; }");
    return QStringLiteral("QLabel { color:#ffffff; background:#2e7d32; border-radius:10px; font-weight:700; }");
}

QPixmap ProduitCardWidget::buildPreviewPixmap(const Catalogue::Produit &produit, const QSize &size)
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
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(canvas.rect().adjusted(10, 8, -10, -8), Qt::AlignCenter | Qt::TextWordWrap, produit.nom);
    return canvas;
}

void ProduitCardWidget::animateElevation(bool elevated)
{
    const double startShadow = m_shadow->blurRadius();
    const double endShadow = elevated ? 24.0 : 14.0;
    m_shadowAnimation->stop();
    m_shadowAnimation->setStartValue(startShadow);
    m_shadowAnimation->setEndValue(endShadow);
    m_shadowAnimation->start();

    const int startHeight = minimumHeight();
    const int endHeight = elevated ? 376 : 370;
    m_paddingAnimation->stop();
    m_paddingAnimation->setStartValue(startHeight);
    m_paddingAnimation->setEndValue(endHeight);
    m_paddingAnimation->start();
}
