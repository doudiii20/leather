#ifndef CATALOGUEPRODUITMODEL_H
#define CATALOGUEPRODUITMODEL_H

#include <QString>

namespace Catalogue {

class Produit
{
public:
    int id = 0;
    QString nom;
    QString categorie;
    double prix = 0.0;
    QString imagePath;
    int stock = 0;
    QString description;
};

} // namespace Catalogue

#endif // CATALOGUEPRODUITMODEL_H
