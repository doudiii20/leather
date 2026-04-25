# Site produit (option 3)

Ce dossier contient un mini site statique : après scan du QR, le téléphone ouvre `produit.html` avec les paramètres `id`, `nom`, `prix`.

**Dépôt GitHub `doudiii20/leather` :** la copie servie par GitHub Pages se trouve dans **`docs/`** à la racine du dépôt (même contenu). Activez Pages sur la branche `main`, dossier **`/docs`** — voir `docs/README.md`.

## Fichiers

- `index.html` : page d'accueil
- `produit.html` : fiche article (paramètres URL)
- `styles.css` : mise en page

## Lier le site à l'application Qt

Dans `leather/produitdetaildialog.cpp`, modifiez la constante `kProductPageUrl` pour qu'elle pointe vers **votre** `produit.html` en ligne, puis recompilez.

Exemple (dépôt projet sur GitHub Pages) :

`https://mon-compte.github.io/mon-depot-web/produit.html`

## Publication avec GitHub Pages (environ 5 minutes)

### 1. Créer un dépôt GitHub pour le site seulement

Sur [https://github.com/new](https://github.com/new) :

- Nom du dépôt : par exemple `leather-catalogue-web` (un nom simple).
- Public.
- Ne cochez pas de README si vous préférez pousser depuis votre machine.

### 2. Mettre les fichiers du dossier `public-site` à la racine du dépôt

Le dépôt doit contenir à la racine (pas dans un sous-dossier) :

- `index.html`
- `produit.html`
- `styles.css`

Vous pouvez copier-coller le contenu de ce dossier, ou en ligne de commande (depuis le dossier `public-site` de ce projet) :

```bash
git init
git add index.html produit.html styles.css README.md
git commit -m "Initial site catalogue produits"
git branch -M main
git remote add origin https://github.com/VOTRE_COMPTE/VOTRE_DEPOT.git
git push -u origin main
```

(Remplacez l'URL `remote` par celle de votre dépôt.)

### 3. Activer GitHub Pages

Sur GitHub : **Settings** → **Pages** :

- **Source** : **Deploy from a branch**
- **Branch** : `main` et dossier **`/ (root)`**
- Enregistrez.

Après une ou deux minutes, le site est disponible à une URL du type :

`https://VOTRE_COMPTE.github.io/VOTRE_DEPOT/`

La fiche produit sera donc :

`https://VOTRE_COMPTE.github.io/VOTRE_DEPOT/produit.html`

### 4. Cas particulier : dépôt `username.github.io`

Si le dépôt s'appelle exactement `votre-compte.github.io`, l'URL racine est :

`https://votre-compte.github.io/`

et la fiche :

`https://votre-compte.github.io/produit.html`

### 5. Tester dans le navigateur

Ouvrez par exemple :

`https://VOTRE_COMPTE.github.io/VOTRE_DEPOT/produit.html?id=1&nom=Test&prix=99.00`

Vous devez voir le nom, le prix et la référence.

### 6. Finaliser l'app Qt

Copiez l'URL complète de `produit.html` dans `kProductPageUrl` dans `produitdetaildialog.cpp`, recompilez, puis testez **Afficher QR Code** et scannez avec le téléphone.

## Alternative : Netlify ou Cloudflare Pages

Même principe : hébergez le contenu de ce dossier, récupérez l'URL HTTPS publique de `produit.html`, et collez-la dans `kProductPageUrl`.
