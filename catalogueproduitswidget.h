#ifndef CATALOGUEPRODUITSWIDGET_H
#define CATALOGUEPRODUITSWIDGET_H

#include "catalogueproduitmodel.h"

#include <QVector>
#include <QWidget>

class QListWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QScrollArea;
class QGridLayout;
class QResizeEvent;

class CatalogueProduitsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CatalogueProduitsWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void setupConnections();
    bool loadProductsFromDatabase();
    void loadMockProducts();
    void applyFilters();
    void rebuildProductGrid();
    int computeColumnCount() const;
    QVector<Catalogue::Produit> filteredProducts() const;
    static QString normalizeCategory(const QString &category);

    QListWidget *m_categoryList = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_sortCombo = nullptr;
    QLabel *m_resultLabel = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_gridContainer = nullptr;
    QGridLayout *m_gridLayout = nullptr;

    QVector<Catalogue::Produit> m_allProducts;
    QVector<Catalogue::Produit> m_visibleProducts;
};

#endif // CATALOGUEPRODUITSWIDGET_H
