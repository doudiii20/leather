#include "catalogueproduitswidget.h"

#include "produitcardwidget.h"
#include "produitdetaildialog.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QHash>
#include <QDebug>
#include <algorithm>

namespace {
QString normalizeCategoryLabel(QString category)
{
    category = category.trimmed();
    category.replace(QStringLiteral("é"), QStringLiteral("e"), Qt::CaseInsensitive);
    return category;
}

QString categoryToLegacyFolder(const QString &category)
{
    const QString normalized = normalizeCategoryLabel(category).toLower();
    if (normalized.contains(QStringLiteral("sac")))
        return QStringLiteral("sac");
    if (normalized.contains(QStringLiteral("chaussure")))
        return QStringLiteral("chaussures");
    if (normalized.contains(QStringLiteral("veste")))
        return QStringLiteral("vestes");
    if (normalized.contains(QStringLiteral("ceinture")))
        return QStringLiteral("ceintures");
    if (normalized.contains(QStringLiteral("accessoire")))
        return QStringLiteral("accessoires");
    return QString();
}

QHash<QString, QStringList> buildLegacyImagePools()
{
    QHash<QString, QStringList> pools;
    const QStringList folders = {
        QStringLiteral("sac"),
        QStringLiteral("chaussures"),
        QStringLiteral("vestes"),
        QStringLiteral("ceintures"),
        QStringLiteral("accessoires"),
    };

    for (const QString &folder : folders) {
        const QString dirPath = QStringLiteral(":/images/catalogue/%1").arg(folder);
        QDir dir(dirPath);
        pools.insert(folder, dir.entryList(
                                 { QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"), QStringLiteral("*.webp") },
                                 QDir::Files,
                                 QDir::Name));
    }

    return pools;
}

QString normalizeProductImagePath(QString imagePath)
{
    imagePath = imagePath.trimmed().replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (imagePath.startsWith(QStringLiteral(":/")))
        return imagePath;
    if (imagePath.startsWith(QStringLiteral("images/catalogue/"), Qt::CaseInsensitive))
        return QStringLiteral(":/") + imagePath;
    if (imagePath.startsWith(QStringLiteral("/images/catalogue/"), Qt::CaseInsensitive))
        return QStringLiteral(":") + imagePath;
    if (!imagePath.isEmpty() && !imagePath.contains(QLatin1Char('/')))
        return QStringLiteral(":/images/catalogue/%1").arg(imagePath);
    return imagePath;
}

bool looksLikeQrPath(const QString &imagePath)
{
    if (imagePath.isEmpty())
        return false;

    return imagePath.contains(QStringLiteral("/qrcodes/"), Qt::CaseInsensitive)
           || imagePath.startsWith(QStringLiteral("qrcodes/"), Qt::CaseInsensitive)
           || QFileInfo(imagePath).fileName().contains(QStringLiteral("qr"), Qt::CaseInsensitive);
}

QString firstImageInResourceFolder(const QString &folder)
{
    const QString dirPath = QStringLiteral(":/images/catalogue/%1").arg(folder);
    QDir dir(dirPath);
    const QStringList files = dir.entryList(
        { QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"), QStringLiteral("*.webp") },
        QDir::Files,
        QDir::Name);
    for (const QString &fileName : files) {
        const QString candidate = dirPath + QLatin1Char('/') + fileName;
        QPixmap test;
        if (test.load(candidate))
            return candidate;
    }
    return QString();
}

QString defaultCatalogueImagePath()
{
    static const QStringList candidates = {
        QStringLiteral(":/images/catalogue/default.png"),
        QStringLiteral(":/images/logo.png"),
    };
    for (const QString &path : candidates) {
        if (QFileInfo::exists(path))
            return path;
    }
    return QString();
}
} // namespace

CatalogueProduitsWidget::CatalogueProduitsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
    loadMockProducts();
    applyFilters();
}

bool CatalogueProduitsWidget::loadProductsFromDatabase()
{
    m_allProducts.clear();
    qDebug() << "[Catalogue] Chargement des produits depuis la base...";
    const QHash<QString, QStringList> legacyPools = buildLegacyImagePools();
    QHash<QString, int> legacyPoolIndex;
    const QString defaultImage = defaultCatalogueImagePath();

    QSqlQuery q;
    const QString sqlPreferred = QStringLiteral(
        "SELECT P.ID, NVL(P.NOM, 'Produit'), NVL(P.CATEGORIE, 'Divers'), NVL(P.PRIX, 0), "
        "NVL(S.QTE_DISPONIBLE, 0), NVL(P.IMAGE, ''), NVL(P.DESCRIPTION, '') "
        "FROM PRODUITS P "
        "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
        "ORDER BY P.ID");

    bool usingLegacyImageColumn = false;
    if (!q.exec(sqlPreferred)) {
        qDebug() << "[Catalogue] Requete IMAGE indisponible, tentative fallback QR_CODE:" << q.lastError().text();
        const QString sqlFallback = QStringLiteral(
            "SELECT P.ID, NVL(P.NOM, 'Produit'), NVL(P.CATEGORIE, 'Divers'), NVL(P.PRIX, 0), "
            "NVL(S.QTE_DISPONIBLE, 0), NVL(P.QR_CODE, ''), '' "
            "FROM PRODUITS P "
            "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
            "ORDER BY P.ID");
        if (!q.exec(sqlFallback)) {
            qDebug() << "[Catalogue] Echec chargement DB:" << q.lastError().text();
            return false;
        }
        usingLegacyImageColumn = true;
    }

    while (q.next()) {
        Catalogue::Produit p;
        p.id = q.value(0).toInt();
        p.nom = q.value(1).toString().trimmed();
        p.categorie = q.value(2).toString().trimmed();
        p.prix = q.value(3).toDouble();
        p.stock = q.value(4).toInt();
        p.imagePath = normalizeProductImagePath(q.value(5).toString());
        p.description = q.value(6).toString().trimmed();

        const QString folder = categoryToLegacyFolder(p.categorie);
        const QStringList legacyImages = legacyPools.value(folder);
        const bool shouldUseLegacyImage = p.imagePath.isEmpty()
                                          || (usingLegacyImageColumn && looksLikeQrPath(p.imagePath))
                                          || !QFileInfo::exists(p.imagePath);
        if (shouldUseLegacyImage && !legacyImages.isEmpty()) {
            const int index = legacyPoolIndex.value(folder, 0) % legacyImages.size();
            p.imagePath = QStringLiteral(":/images/catalogue/%1/%2").arg(folder, legacyImages.at(index));
            legacyPoolIndex[folder] = index + 1;
        }
        if ((p.imagePath.isEmpty() || !QFileInfo::exists(p.imagePath)) && !defaultImage.isEmpty())
            p.imagePath = defaultImage;

        qDebug() << "[Catalogue] Produit"
                 << "id=" << p.id
                 << "nom=" << p.nom
                 << "prix=" << p.prix
                 << "image=" << p.imagePath;

        m_allProducts.push_back(p);
    }

    if (m_allProducts.isEmpty())
        qDebug() << "[Catalogue] Base vide: aucun produit trouve.";
    else
        qDebug() << "[Catalogue] Produits charges:" << m_allProducts.size();

    // Requete valide => on ne bascule pas sur les mock en cas de base vide.
    return true;
}

void CatalogueProduitsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    rebuildProductGrid();
}

void CatalogueProduitsWidget::setupUi()
{
    setObjectName(QStringLiteral("catalogueProduitsRoot"));
    setStyleSheet(QStringLiteral(
        "QWidget#catalogueProduitsRoot { background: #f6efe4; }"
        "QFrame#catalogueCategoryCard, QFrame#catalogueTopCard {"
        "  background: #ffffff;"
        "  border: 1px solid #e8dbc9;"
        "  border-radius: 12px;"
        "}"
        "QListWidget#categoryList {"
        "  background: transparent;"
        "  border: none;"
        "  outline: none;"
        "}"
        "QListWidget#categoryList::item {"
        "  border-radius: 8px;"
        "  padding: 10px 8px;"
        "  margin: 2px 0;"
        "  color: #6b4b34;"
        "  font-weight: 600;"
        "}"
        "QListWidget#categoryList::item:hover { background: #fff4e9; color: #7a4d2b; }"
        "QListWidget#categoryList::item:selected {"
        "  background: #ecd7c3;"
        "  color: #5d2e06;"
        "  border: 1px solid #d8b18f;"
        "}"
        "QLineEdit, QComboBox {"
        "  border: 1px solid #d5dbe5;"
        "  border-radius: 8px;"
        "  padding: 8px 10px;"
        "  background: #ffffff;"
        "}"
        "QLineEdit:focus, QComboBox:focus { border-color: #c08a5b; }"
        "QLabel#resultLabel { color: #6a5547; font-size: 12px; font-weight: 600; }"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(12);

    auto *topCard = new QFrame(this);
    topCard->setObjectName(QStringLiteral("catalogueTopCard"));
    auto *topLay = new QHBoxLayout(topCard);
    topLay->setContentsMargins(12, 10, 12, 10);
    topLay->setSpacing(10);

    m_searchEdit = new QLineEdit(topCard);
    m_searchEdit->setPlaceholderText(QStringLiteral("Rechercher un produit..."));
    m_searchEdit->setMinimumHeight(36);
    m_searchEdit->setClearButtonEnabled(true);

    m_sortCombo = new QComboBox(topCard);
    m_sortCombo->setMinimumHeight(36);
    m_sortCombo->addItem(QStringLiteral("Nom A-Z"), QStringLiteral("name_asc"));
    m_sortCombo->addItem(QStringLiteral("Prix croissant"), QStringLiteral("price_asc"));
    m_sortCombo->addItem(QStringLiteral("Prix décroissant"), QStringLiteral("price_desc"));

    m_resultLabel = new QLabel(QStringLiteral("0 produit"), topCard);
    m_resultLabel->setObjectName(QStringLiteral("resultLabel"));

    topLay->addWidget(m_searchEdit, 1);
    topLay->addWidget(m_sortCombo, 0);
    topLay->addWidget(m_resultLabel, 0);
    root->addWidget(topCard);

    auto *bodyLay = new QHBoxLayout();
    bodyLay->setSpacing(12);

    auto *categoryCard = new QFrame(this);
    categoryCard->setObjectName(QStringLiteral("catalogueCategoryCard"));
    categoryCard->setMinimumWidth(220);
    categoryCard->setMaximumWidth(260);
    auto *catLay = new QVBoxLayout(categoryCard);
    catLay->setContentsMargins(10, 10, 10, 10);
    catLay->setSpacing(6);

    auto *catTitle = new QLabel(QStringLiteral("Catégories"), categoryCard);
    catTitle->setStyleSheet(QStringLiteral("color:#5d2e06; font-size:14px; font-weight:700;"));
    catLay->addWidget(catTitle);

    m_categoryList = new QListWidget(categoryCard);
    m_categoryList->setObjectName(QStringLiteral("categoryList"));
    m_categoryList->setIconSize(QSize(24, 24));

    const auto makeCategoryIcon = [](const QString &glyph, const QColor &bg, const QString &imagePath) -> QIcon {
        QPixmap pm(22, 22);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing, true);

        const QRect iconRect = pm.rect().adjusted(1, 1, -1, -1);
        QPainterPath clipPath;
        clipPath.addRoundedRect(iconRect, 6, 6);
        p.setClipPath(clipPath);

        QPixmap categoryImage;
        if (!imagePath.isEmpty())
            categoryImage.load(imagePath);
        if (!categoryImage.isNull()) {
            const QPixmap scaled = categoryImage.scaled(pm.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            const int sx = (scaled.width() - pm.width()) / 2;
            const int sy = (scaled.height() - pm.height()) / 2;
            p.drawPixmap(-sx, -sy, scaled);

            // Legere couche pour garder un rendu homogene
            p.setClipping(false);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 42));
            p.drawRoundedRect(iconRect, 6, 6);
        } else {
            p.setClipping(false);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(iconRect, 6, 6);
            QFont f(QStringLiteral("Segoe UI Symbol"));
            f.setBold(true);
            f.setPointSize(10);
            p.setFont(f);
            p.setPen(QColor(QStringLiteral("#ffffff")));
            p.drawText(pm.rect(), Qt::AlignCenter, glyph);
        }

        return QIcon(pm);
    };

    struct CategoryEntry {
        QString name;
        QString glyph;
        QColor color;
        QString folder;
    };
    const QVector<CategoryEntry> categories = {
        { QStringLiteral("Tous"), QStringLiteral("•"), QColor(QStringLiteral("#5d2e06")), QString() },
        { QStringLiteral("Sacs"), QStringLiteral("👜"), QColor(QStringLiteral("#8b5e3c")), QStringLiteral("sac") },
        { QStringLiteral("Chaussures"), QStringLiteral("👞"), QColor(QStringLiteral("#6f4e37")), QStringLiteral("chaussures") },
        { QStringLiteral("Vestes"), QStringLiteral("🧥"), QColor(QStringLiteral("#7a4d2b")), QStringLiteral("vestes") },
        { QStringLiteral("Ceintures"), QStringLiteral("⌁"), QColor(QStringLiteral("#9b6a3d")), QStringLiteral("ceintures") },
        { QStringLiteral("Accessoires"), QStringLiteral("✦"), QColor(QStringLiteral("#b07d4f")), QStringLiteral("accessoires") },
    };
    for (const CategoryEntry &entry : categories) {
        const QString imagePath = entry.folder.isEmpty() ? QString() : firstImageInResourceFolder(entry.folder);
        auto *item = new QListWidgetItem(makeCategoryIcon(entry.glyph, entry.color, imagePath), entry.name);
        item->setData(Qt::UserRole, entry.name);
        m_categoryList->addItem(item);
    }
    m_categoryList->setCurrentRow(0);

    catLay->addWidget(m_categoryList, 1);
    bodyLay->addWidget(categoryCard, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; }"));

    m_gridContainer = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
    m_gridLayout->setSpacing(12);
    m_scrollArea->setWidget(m_gridContainer);

    bodyLay->addWidget(m_scrollArea, 1);
    root->addLayout(bodyLay, 1);
}

void CatalogueProduitsWidget::setupConnections()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { applyFilters(); });
    connect(m_sortCombo, &QComboBox::currentIndexChanged, this, [this](int) { applyFilters(); });
    connect(m_categoryList, &QListWidget::currentRowChanged, this, [this](int) { applyFilters(); });
}

void CatalogueProduitsWidget::loadMockProducts()
{
    m_allProducts.clear();
    m_allProducts = {
        { 1, QStringLiteral("Veste Rider Homme Noire"), QStringLiteral("Vestes"), 690.0, QStringLiteral(":/images/catalogue/vestes/BONBON_050032_0004_356e5b24-9f11-4d12-846e-56862f082f97.webp"), 6, QStringLiteral("Veste cuir style biker, finition premium.") },
        { 2, QStringLiteral("Blouson Cuir Femme Noir"), QStringLiteral("Vestes"), 640.0, QStringLiteral(":/images/catalogue/vestes/cg23femmelcw8600noir23__028224300_1531_29092022.jpg"), 5, QStringLiteral("Coupe ajustee, zip frontal et doublure douce.") },
        { 3, QStringLiteral("Veste Bomber Gris"), QStringLiteral("Vestes"), 610.0, QStringLiteral(":/images/catalogue/vestes/show_17032440935691703244098.webp"), 4, QStringLiteral("Bomber moderne en cuir souple.") },
        { 4, QStringLiteral("Perfecto Femme Rouge"), QStringLiteral("Vestes"), 670.0, QStringLiteral(":/images/catalogue/vestes/veste-cuir-femme-veste-moto-cuir-veritable-rouge-ancre-Kbc.webp"), 3, QStringLiteral("Modele iconique pour look urbain.") },

        { 5, QStringLiteral("Sac Cabas Camel"), QStringLiteral("Sacs"), 320.0, QStringLiteral(":/images/catalogue/sac/sac-cabas-cuir-souple-femme-grand-format-et-couleurs-tendance-camel-cuir-veritable-1182174735.webp"), 9, QStringLiteral("Grand format ideal pour le quotidien.") },
        { 6, QStringLiteral("Sac A Main Elegant"), QStringLiteral("Sacs"), 280.0, QStringLiteral(":/images/catalogue/sac/sac-a-main-femme-alberto-ricci-gb3681.webp"), 7, QStringLiteral("Design chic avec finitions metal.") },
        { 7, QStringLiteral("Sac Croco Bordeaux"), QStringLiteral("Sacs"), 355.0, QStringLiteral(":/images/catalogue/sac/246319_RD_1yf.webp"), 5, QStringLiteral("Texture croco avec anse renforcee.") },
        { 8, QStringLiteral("Sac City Beige"), QStringLiteral("Sacs"), 295.0, QStringLiteral(":/images/catalogue/sac/WB01940_BX3542_1007_TCO00_M1.jpg"), 8, QStringLiteral("Modele leger et pratique.") },

        { 9, QStringLiteral("Derby Homme Marron"), QStringLiteral("Chaussures"), 240.0, QStringLiteral(":/images/catalogue/chaussures/chaussure-cuir-pour-homme-tunisie-a-bas-prix-vente-flash-scaled.jpg"), 11, QStringLiteral("Derby classique en cuir veritable.") },
        { 10, QStringLiteral("Mocassins Chaine Noirs"), QStringLiteral("Chaussures"), 210.0, QStringLiteral(":/images/catalogue/chaussures/J71A8029.webp"), 10, QStringLiteral("Mocassins tendance semelle epaisse.") },
        { 11, QStringLiteral("Chaussures Ville Brun"), QStringLiteral("Chaussures"), 225.0, QStringLiteral(":/images/catalogue/chaussures/J71A9435-min.webp"), 8, QStringLiteral("Confort quotidien et ligne elegante.") },
        { 12, QStringLiteral("Escarpins Nude Vernis"), QStringLiteral("Chaussures"), 180.0, QStringLiteral(":/images/catalogue/chaussures/mary-jane-a-brides-margot.webp"), 6, QStringLiteral("Talon stable et style raffine.") },

        { 13, QStringLiteral("Ceinture Cuir Noir"), QStringLiteral("Ceintures"), 95.0, QStringLiteral(":/images/catalogue/ceintures/J71A0212.jpg"), 15, QStringLiteral("Boucle metal finition brossee.") },
        { 14, QStringLiteral("Ceinture Cuir Marron"), QStringLiteral("Ceintures"), 89.0, QStringLiteral(":/images/catalogue/ceintures/ceinture-100-cuir-marron-1100808-1-product_1.jpg"), 13, QStringLiteral("Classique intemporel pour costume.") },
        { 15, QStringLiteral("Ceinture Reversible"), QStringLiteral("Ceintures"), 110.0, QStringLiteral(":/images/catalogue/ceintures/ceinture-reversible-38-mm-en-cuir-noir-et-gris-avec-boucle-pantheon-brossee-plaquee-or_a3f52995-3d9e-4d73-8f8f-9fe59b6963d3.webp"), 7, QStringLiteral("Deux couleurs en une seule ceinture.") },
        { 16, QStringLiteral("Ceinture Heritage"), QStringLiteral("Ceintures"), 99.0, QStringLiteral(":/images/catalogue/ceintures/10425_0414_9029_NM_FA_F1XX.webp"), 10, QStringLiteral("Ceinture pleine fleur et boucle argentee.") },
        { 17, QStringLiteral("Ceinture Femme Rouge"), QStringLiteral("Ceintures"), 84.0, QStringLiteral(":/images/catalogue/ceintures/a0yz1x-248.jpg"), 12, QStringLiteral("Modele fin pour looks habilles.") },
        { 18, QStringLiteral("Ceinture Classique Homme"), QStringLiteral("Ceintures"), 92.0, QStringLiteral(":/images/catalogue/ceintures/dj5uca-25.110.jpg"), 9, QStringLiteral("Ligne sobre pour tenue business.") },

        { 19, QStringLiteral("Porte-Cartes Cuir"), QStringLiteral("Accessoires"), 55.0, QStringLiteral(":/images/catalogue/accessoires/ccgf.jpg"), 18, QStringLiteral("Compact et resistant pour cartes.") },
        { 20, QStringLiteral("Trousse Cuir Noir"), QStringLiteral("Accessoires"), 65.0, QStringLiteral(":/images/catalogue/accessoires/trousse-cuir-noir.jpg"), 12, QStringLiteral("Trousse premium multi-usage.") },
        { 21, QStringLiteral("Pochette Atelier"), QStringLiteral("Accessoires"), 72.0, QStringLiteral(":/images/catalogue/accessoires/nn-400x457.jpg"), 9, QStringLiteral("Pochette minimaliste en cuir souple.") },
        { 22, QStringLiteral("Coffret Cuir Personnalisable"), QStringLiteral("Accessoires"), 135.0, QStringLiteral(":/images/catalogue/accessoires/touch-coffret-cuir-personnalisable-tunisie.jpg"), 6, QStringLiteral("Coffret cadeau en cuir haut de gamme.") },
        { 23, QStringLiteral("Trousse Stylos Luxe"), QStringLiteral("Accessoires"), 78.0, QStringLiteral(":/images/catalogue/accessoires/Trousse-stylos-sac-main-cuir-ofilducuir-lyon-france-819x1024.jpg"), 8, QStringLiteral("Etui elegant pour stylos et accessoires.") },
        { 24, QStringLiteral("Mini Accessoire Signature"), QStringLiteral("Accessoires"), 49.0, QStringLiteral(":/images/catalogue/accessoires/50222325-2d79-47cc-b767-f7ea95be79d5-Grande.webp"), 14, QStringLiteral("Petit format pratique pour le quotidien.") },
    };
}

void CatalogueProduitsWidget::applyFilters()
{
    m_visibleProducts = filteredProducts();

    const QString sortKey = m_sortCombo->currentData().toString();
    if (sortKey == QLatin1String("price_asc")) {
        std::sort(m_visibleProducts.begin(), m_visibleProducts.end(), [](const Catalogue::Produit &a, const Catalogue::Produit &b) {
            return a.prix < b.prix;
        });
    } else if (sortKey == QLatin1String("price_desc")) {
        std::sort(m_visibleProducts.begin(), m_visibleProducts.end(), [](const Catalogue::Produit &a, const Catalogue::Produit &b) {
            return a.prix > b.prix;
        });
    } else {
        std::sort(m_visibleProducts.begin(), m_visibleProducts.end(), [](const Catalogue::Produit &a, const Catalogue::Produit &b) {
            return QString::localeAwareCompare(a.nom, b.nom) < 0;
        });
    }

    const int count = m_visibleProducts.size();
    m_resultLabel->setText(count > 1 ? QStringLiteral("%1 produits").arg(count) : QStringLiteral("%1 produit").arg(count));
    rebuildProductGrid();
}

void CatalogueProduitsWidget::rebuildProductGrid()
{
    while (QLayoutItem *item = m_gridLayout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    const int columns = computeColumnCount();
    for (int i = 0; i < m_visibleProducts.size(); ++i) {
        const Catalogue::Produit &produit = m_visibleProducts.at(i);
        auto *card = new ProduitCardWidget(produit, m_gridContainer);
        connect(card, &ProduitCardWidget::detailRequested, this, [this](const Catalogue::Produit &p) {
            ProduitDetailDialog dlg(p, this);
            dlg.exec();
        });
        connect(card, &ProduitCardWidget::orderRequested, this, [this](const Catalogue::Produit &p) {
            QMessageBox::information(this,
                                     QStringLiteral("Commande"),
                                     QStringLiteral("Produit \"%1\" ajouté au panier.").arg(p.nom));
        });

        const int row = i / columns;
        const int col = i % columns;
        m_gridLayout->addWidget(card, row, col);
    }

    for (int c = 0; c < columns; ++c)
        m_gridLayout->setColumnStretch(c, 1);
    m_gridLayout->setRowStretch((m_visibleProducts.size() / columns) + 1, 1);
}

int CatalogueProduitsWidget::computeColumnCount() const
{
    if (!m_scrollArea || !m_scrollArea->viewport())
        return 1;
    const int width = m_scrollArea->viewport()->width();
    if (width >= 1200)
        return 4;
    if (width >= 900)
        return 3;
    if (width >= 620)
        return 2;
    return 1;
}

QVector<Catalogue::Produit> CatalogueProduitsWidget::filteredProducts() const
{
    QVector<Catalogue::Produit> out;
    const QString selectedCategory =
        m_categoryList->currentItem() ? normalizeCategory(m_categoryList->currentItem()->data(Qt::UserRole).toString()) : QStringLiteral("Tous");
    const QString searchTerm = m_searchEdit->text().trimmed();

    for (const Catalogue::Produit &p : m_allProducts) {
        if (selectedCategory != QLatin1String("Tous")
            && normalizeCategory(p.categorie).compare(selectedCategory, Qt::CaseInsensitive) != 0) {
            continue;
        }
        if (!searchTerm.isEmpty() && !p.nom.contains(searchTerm, Qt::CaseInsensitive))
            continue;
        out.push_back(p);
    }
    return out;
}

QString CatalogueProduitsWidget::normalizeCategory(const QString &category)
{
    QString value = category.trimmed();
    value.replace(QStringLiteral("é"), QStringLiteral("e"), Qt::CaseInsensitive);
    return value;
}
