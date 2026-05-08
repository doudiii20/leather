# Stock API (Oracle via ODBC)

API backend reel pour la page matieres premieres, **sans Oracle Instant Client** : elle utilise le **DSN ODBC Windows** comme ton application Qt.

## Prerequis

- Pilote ODBC Oracle installe (souvent deja present si Qt se connecte).
- DSN configure dans **Outils d administration Windows > Sources de donnees ODBC** (ex. `projet_cuir`).

## 1) Installation

```powershell
cd "C:\Users\MSI\OneDrive\Desktop\leather\leather\stock-api"
copy .env.example .env
npm install
```

## 2) Configuration `.env`

Choisis une des deux options :

**Option A — DSN + identifiants**

```env
ODBC_DSN=projet_cuir
ORACLE_USER=ton_user
ORACLE_PASSWORD=ton_mot_de_passe
```

**Option B — Chaine ODBC complete**

```env
ODBC_CONNECTION_STRING=DSN=projet_cuir;UID=ton_user;PWD=ton_mot_de_passe
```

Tu peux laisser `ORACLE_USER` / `ORACLE_PASSWORD` vides si ton DSN enregistre deja l utilisateur (Option A avec seulement `ODBC_DSN=...`).

## 3) Demarrage

```powershell
npm.cmd start
```

Message attendu : `Stock API running on http://localhost:3000`

## 4) Tests

- `http://localhost:3000/health`
- `http://localhost:3000/stats`
- `http://localhost:3000/stock/faible` (matieres premieres)
- `http://localhost:3000/produits/stock/faible` (produits finis)
- `http://localhost:3000/prediction`

## 5) App Qt

Par defaut l app utilise `http://localhost:3000`. Sinon :

```powershell
$env:LEATHER_STOCK_API_BASE="http://localhost:3000"
```

## Depannage

- **Module odbc introuvable / erreur native** : installe les outils de build Node sur Windows (`npm install --global windows-build-tools` ou Visual Studio Build Tools) si `npm install odbc` echoue.
- **Connexion ODBC refusee** : verifie le DSN dans l administrateur ODBC et les memes identifiants que dans Qt.
