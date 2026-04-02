#ifndef MATIEREPREMIERE_H
#define MATIEREPREMIERE_H

#include <QString>
#include <QDate>

class MatierePremiere
{
public:
    MatierePremiere();
    MatierePremiere(int id, const QString &ref, const QString &nomCuir,
                    const QString &typeCuir, const QString &gamme,
                    const QString &couleur, const QString &statut,
                    double epaisseur, const QString &origine, int reserve);

    // Getters
    int     getId()           const { return m_id; }
    QString getReference()    const { return m_reference; }
    QString getNomCuir()      const { return m_nomCuir; }
    QString getTypeCuir()     const { return m_typeCuir; }
    QString getGamme()       const { return m_gamme; }
    QString getCouleur()      const { return m_couleur; }
    QString getStatut()       const { return m_statut; }
    double  getEpaisseur()    const { return m_epaisseur; }
    QString getOrigine()      const { return m_origine; }
    int     getReserve()      const { return m_reserve; }

    // Setters
    void setId(int id)                      { m_id = id; }
    void setReference(const QString &v)     { m_reference = v; }
    void setNomCuir(const QString &v)       { m_nomCuir = v; }
    void setTypeCuir(const QString &v)      { m_typeCuir = v; }
    void setGamme(const QString &v)         { m_gamme = v; }
    void setCouleur(const QString &v)       { m_couleur = v; }
    void setStatut(const QString &v)        { m_statut = v; }
    void setEpaisseur(double v)             { m_epaisseur = v; }
    void setOrigine(const QString &v)       { m_origine = v; }
    void setReserve(int v)                  { m_reserve = v; }

    // Seuil critique (stock bas)
    bool isSeuilCritique(int seuil = 10) const { return m_reserve <= seuil; }

private:
    int     m_id;
    QString m_reference;
    QString m_nomCuir;
    QString m_typeCuir;
    QString m_gamme;
    QString m_couleur;
    QString m_statut;
    double  m_epaisseur;
    QString m_origine;
    int     m_reserve;
};

#endif // MATIEREPREMIERE_H
