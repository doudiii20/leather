# Site catalogue (GitHub Pages)

Ce dossier est servi par **GitHub Pages** depuis la branche `main`, dossier **`/docs`**.

URL publique du dépôt **doudiii20/leather** :

- Accueil : `https://doudiii20.github.io/leather/`
- Fiche produit (utilisée par le QR de l'app Qt) : `https://doudiii20.github.io/leather/produit.html`

## Activer Pages (une fois)

Sur GitHub : **Settings** → **Pages** → **Build and deployment** :

- **Source** : **Deploy from a branch**
- **Branch** : `main` / **`/docs`**
- Enregistrer, attendre 1 à 2 minutes.

## Images catalogue

Les fichiers du dossier `images/catalogue/` reprennent les memes photos que `Resources/images/catalogue/` (pour affichage sur la fiche web apres scan QR).

L'application Qt ajoute le parametre `img` dans l'URL du QR, par exemple :

`images/catalogue/sac/1773322456.jpg`

## Tester

Ouvrir dans le navigateur :

`https://doudiii20.github.io/leather/produit.html?id=1&nom=Test&prix=99`

Avec image (exemple) :

`https://doudiii20.github.io/leather/produit.html?id=1&nom=Test&prix=99&img=images/catalogue/sac/1773322456.jpg`
