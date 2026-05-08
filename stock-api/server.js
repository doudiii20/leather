require("dotenv").config();

const express = require("express");
const cors = require("cors");
const odbc = require("odbc");

const app = express();
app.use(cors());
app.use(express.json());

const PORT = Number.parseInt(process.env.PORT || "3000", 10);
const LOW_STOCK_THRESHOLD = Number.parseInt(process.env.LOW_STOCK_THRESHOLD || "10", 10);
const CRITICAL_STOCK_THRESHOLD = Number.parseInt(process.env.CRITICAL_STOCK_THRESHOLD || "5", 10);
const STOCK_DAILY_CONSUMPTION = Number.parseFloat(process.env.STOCK_DAILY_CONSUMPTION || "2");

let pool;

/** Entier sûr pour interpolation SQL (seuils uniquement). */
function safeSqlInt(value, fallback) {
  const n = Number.parseInt(String(value), 10);
  if (!Number.isFinite(n) || n < 0 || n > 10000000)
    return fallback;
  return n;
}

function formatOdbcErr(err) {
  const out = { error: err.message };
  if (err.state)
    out.state = err.state;
  if (Array.isArray(err.odbcErrors) && err.odbcErrors.length)
    out.odbcErrors = err.odbcErrors;
  return out;
}

/**
 * ODBC renvoie souvent des noms de colonnes en majuscules selon le driver.
 */
function normRow(row) {
  if (!row || typeof row !== "object")
    return row;
  const out = {};
  for (const [k, v] of Object.entries(row))
    out[k.toUpperCase()] = v;
  return out;
}

function buildOdbcConnectionString() {
  const full = process.env.ODBC_CONNECTION_STRING?.trim();
  if (full)
    return full;

  const dsn = (process.env.ODBC_DSN || "projet_cuir").trim();
  const u = process.env.ORACLE_USER?.trim();
  const p = process.env.ORACLE_PASSWORD ?? "";

  if (u)
    return `DSN=${dsn};UID=${u};PWD=${p}`;
  return `DSN=${dsn}`;
}

function buildPredictionFromRow(row) {
  const r = normRow(row);
  const stock = Number(r.RESERVE ?? 0);
  const consommation = Number.isFinite(STOCK_DAILY_CONSUMPTION) && STOCK_DAILY_CONSUMPTION > 0
    ? STOCK_DAILY_CONSUMPTION
    : 1;
  const joursRestants = Math.max(0, Math.floor(stock / consommation));
  return {
    produit: r.NOM_CUIR || "Produit",
    stock,
    consommation,
    joursRestants,
  };
}

function normalizeUid(uid) {
  return String(uid ?? "")
    .replace(/\s+/g, "")
    .toUpperCase()
    .trim();
}

async function queryOne(sql, params = []) {
  const result = await pool.query(sql, params);
  const rows = Array.isArray(result) ? result : [];
  return rows.length > 0 ? normRow(rows[0]) : null;
}

async function queryMany(sql, params = []) {
  const result = await pool.query(sql, params);
  const rows = Array.isArray(result) ? result : [];
  return rows.map(normRow);
}

app.get("/health", async (_req, res) => {
  try {
    await queryOne("SELECT 1 AS OK FROM DUAL");
    res.json({ status: "ok" });
  } catch (err) {
    res.status(500).json({ status: "error", message: err.message });
  }
});

app.post("/verifier-carte", async (req, res) => {
  try {
    const uidRaw = req.body?.uid;
    if (typeof uidRaw !== "string" || !uidRaw.trim()) {
      return res.status(400).json({ acces: false, erreur: "UID manquant ou invalide" });
    }

    const uid = normalizeUid(uidRaw);
    const table = (process.env.ORACLE_TABLE || "EMPLOYES").trim();
    const sql = `SELECT 1 AS OK FROM ${table} WHERE UPPER(REPLACE(ID_CARTE, ' ', '')) = ? AND ROWNUM <= 1`;
    const row = await queryOne(sql, [uid]);

    return res.json({ acces: Boolean(row) });
  } catch (err) {
    console.error("[/verifier-carte]", err);
    return res.status(500).json({ acces: false, message: "Erreur /verifier-carte", ...formatOdbcErr(err) });
  }
});

app.get("/stats", async (_req, res) => {
  try {
    const low = safeSqlInt(LOW_STOCK_THRESHOLD, 10);
    // Pas de "?" ici : certains pilotes Oracle ODBC echouent sur les binds dans SUM(CASE...).
    const row = await queryOne(
      `SELECT
         COUNT(*) AS TOTAL_PRODUITS,
         SUM(CASE WHEN NVL(RESERVE,0) = 0 THEN 1 ELSE 0 END) AS PRODUITS_RUPTURE,
         SUM(CASE WHEN NVL(RESERVE,0) <= ${low} THEN 1 ELSE 0 END) AS STOCK_FAIBLE
       FROM MATIERES_PREMIERES`
    );

    res.json({
      totalProduits: Number(row?.TOTAL_PRODUITS ?? 0),
      produitsRupture: Number(row?.PRODUITS_RUPTURE ?? 0),
      stockFaible: Number(row?.STOCK_FAIBLE ?? 0),
    });
  } catch (err) {
    console.error("[/stats]", err);
    res.status(500).json({ message: "Erreur /stats", ...formatOdbcErr(err) });
  }
});

app.get("/stock/faible", async (_req, res) => {
  try {
    const crit = safeSqlInt(CRITICAL_STOCK_THRESHOLD, 5);
    const rows = await queryMany(
      `SELECT
         ID,
         NOM_CUIR AS NOM,
         NVL(RESERVE,0) AS STOCK,
         NVL(STATUT,'N/A') AS STATUT
       FROM MATIERES_PREMIERES
       WHERE NVL(RESERVE,0) <= ${crit}
       ORDER BY NVL(RESERVE,0) ASC, ID ASC`
    );

    res.json(
      rows.map((r) => ({
        id: Number(r.ID),
        nom: r.NOM,
        stock: Number(r.STOCK),
        statut: r.STATUT,
      }))
    );
  } catch (err) {
    console.error("[/stock/faible]", err);
    res.status(500).json({ message: "Erreur /stock/faible", ...formatOdbcErr(err) });
  }
});

/** Produits finis (PRODUITS + STOCK) — distinct de /stock/faible (matieres premieres). */
app.get("/produits/stock/faible", async (_req, res) => {
  try {
    const crit = safeSqlInt(CRITICAL_STOCK_THRESHOLD, 5);
    const rows = await queryMany(
      `SELECT
         P.ID,
         NVL(P.NOM, 'Produit') AS NOM,
         NVL(S.QTE_DISPONIBLE, 0) AS STOCK
       FROM PRODUITS P
       LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID
       WHERE NVL(S.QTE_DISPONIBLE, 0) <= ${crit}
       ORDER BY NVL(S.QTE_DISPONIBLE, 0) ASC, P.ID ASC`
    );

    res.json(
      rows.map((r) => ({
        id: Number(r.ID),
        nom: r.NOM,
        stock: Number(r.STOCK),
      }))
    );
  } catch (err) {
    console.error("[/produits/stock/faible]", err);
    res.status(500).json({ message: "Erreur /produits/stock/faible", ...formatOdbcErr(err) });
  }
});

app.get("/prediction", async (_req, res) => {
  try {
    // ROWNUM : compatible vieux Oracle ; FETCH FIRST peut echouer selon la version.
    const row = await queryOne(
      `SELECT ID, NOM_CUIR, RESERVE FROM (
         SELECT ID, NOM_CUIR, NVL(RESERVE,0) AS RESERVE
         FROM MATIERES_PREMIERES
         ORDER BY NVL(RESERVE,0) ASC, ID ASC
       ) t
       WHERE ROWNUM <= 1`
    );

    if (!row) {
      return res.status(404).json({ message: "Aucune matière première trouvée." });
    }

    return res.json(buildPredictionFromRow(row));
  } catch (err) {
    console.error("[/prediction]", err);
    return res.status(500).json({ message: "Erreur /prediction", ...formatOdbcErr(err) });
  }
});

app.use((err, _req, res, _next) => {
  res.status(500).json({ message: "Erreur serveur", error: err.message });
});

async function start() {
  const cs = buildOdbcConnectionString();
  if (!cs)
    throw new Error("Configure ODBC_CONNECTION_STRING ou ODBC_DSN (+ ORACLE_USER / ORACLE_PASSWORD si besoin).");

  console.log("[ODBC] Connexion via DSN / chaine ODBC (sans Oracle Instant Client).");

  pool = await odbc.pool({
    connectionString: cs,
    initialSize: 5,
    reuseConnections: true,
  });

  app.listen(PORT, () => {
    console.log(`Stock API running on http://localhost:${PORT}`);
  });
}

start().catch((err) => {
  console.error("Failed to start stock API:", err.message);
  process.exit(1);
});
