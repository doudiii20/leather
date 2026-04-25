#ifndef PRODUITCARDWIDGET_H
#define PRODUITCARDWIDGET_H

#include "catalogueproduitmodel.h"

#include <QFrame>
#include <QPixmap>

class QLabel;
class QPushButton;
class QGraphicsDropShadowEffect;
class QVariantAnimation;
class QEnterEvent;
class QSize;

class ProduitCardWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ProduitCardWidget(const Catalogue::Produit &produit, QWidget *parent = nullptr);
    const Catalogue::Produit &produit() const;

signals:
    void detailRequested(const Catalogue::Produit &produit);
    void orderRequested(const Catalogue::Produit &produit);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    static QString stockStatusText(int stock);
    static QString stockStatusStyle(int stock);
    static QPixmap buildPreviewPixmap(const Catalogue::Produit &produit, const QSize &size);
    void animateElevation(bool elevated);

    Catalogue::Produit m_produit;
    QGraphicsDropShadowEffect *m_shadow = nullptr;
    QVariantAnimation *m_shadowAnimation = nullptr;
    QVariantAnimation *m_paddingAnimation = nullptr;
};

#endif // PRODUITCARDWIDGET_H
