#include "matierepremiere.h"

MatierePremiere::MatierePremiere()
    : m_id(0), m_epaisseur(0.0), m_reserve(0)
{}

MatierePremiere::MatierePremiere(int id, const QString &ref, const QString &nomCuir,
                                 const QString &typeCuir, const QString &gamme,
                                 const QString &couleur, const QString &statut,
                                 double epaisseur, const QString &origine, int reserve)
    : m_id(id), m_reference(ref), m_nomCuir(nomCuir), m_typeCuir(typeCuir),
    m_gamme(gamme), m_couleur(couleur), m_statut(statut),
    m_epaisseur(epaisseur), m_origine(origine), m_reserve(reserve)
{}
