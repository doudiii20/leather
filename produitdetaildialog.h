#ifndef PRODUITDETAILDIALOG_H
#define PRODUITDETAILDIALOG_H

#include "catalogueproduitmodel.h"

#include <QDialog>
#include <QPixmap>

class QLabel;
class QPushButton;
class QSize;

class ProduitDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProduitDetailDialog(const Catalogue::Produit &produit, QWidget *parent = nullptr);

private:
    static QPixmap generateQRCode(const QString &data);
    void showQRCodeDialog(const Catalogue::Produit &produit);
    static QString stockStatusText(int stock);
    static QString stockStatusStyle(int stock);
    static QPixmap buildPreviewPixmap(const Catalogue::Produit &produit, const QSize &size);
};

#endif // PRODUITDETAILDIALOG_H
