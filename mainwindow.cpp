#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "commercestore.h"
#include "recommendationservice.h"
#include "chatbotservice.h"
#include "fournisseurapiservice.h"
#include "clientnotificationservice.h"
#include "whatsappbusinessservice.h"
#include "fournisseur.h"
#include "produit.h"
#include "matierepremiere.h"
#include "assistantwindow.h"
#include "chatbotwindow.h"
#include "catalogueproduitswidget.h"
#include "facelogindialog.h"
#include "settingswindow.h"
#include "forgotpassworddialog.h"
#include "busyseasoncalendardialog.h"
#include "smtpsender.h"
#include "qrcodegen.hpp"
#include <QAbstractItemView>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QSqlDatabase>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollArea>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <cmath>
#include <QFrame>
#include <QSizePolicy>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QInputDialog>
#include <QCryptographicHash>
#include <QSettings>
#include <QDesktopServices>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QImage>
#include <QUuid>
#include <QStringConverter>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QDialog>
#include <QCalendarWidget>
#include <QHash>
#include <QLocale>
#include <QTextCharFormat>
#include <QStackedWidget>
#include <QGridLayout>
#include <QFormLayout>
#include <QSizePolicy>
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QTranslator>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QVariantAnimation>
#include <functional>
#include <QStyle>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QLinearGradient>
#include <QColor>
#include <QTextBrowser>
#include <QToolTip>
#include <QMouseEvent>
#include <QVector>
#ifdef LEATHER_HAVE_LOCATION_MAP
#include <QQuickWidget>
#include <QQmlContext>
#include <QQuickItem>
#endif
#ifdef LEATHER_HAVE_WEBENGINE
#include <QtWebEngineWidgets/QWebEngineView>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEnginePage>
#else
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEnginePage>
#endif
#endif
#include <QEventLoop>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QRadioButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QColor>
#include <QMap>
#include <memory>
#include <algorithm>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QPieSeries>
#include <QPieSlice>
#include <QAbstractAxis>
#include <QAbstractSeries>
#else
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QAbstractSeries>
using namespace QtCharts;
#endif
using namespace qrcodegen;

namespace {

/// Logo page d’accueil : `Resources/images/logo.png` si présent dans le .qrc, sinon pictogramme « RLH ».
static QPixmap leatherAppLogoPixmap(int side)
{
    if (side <= 0)
        side = 120;
    const QPixmap fromRes(QStringLiteral(":/images/logo.png"));
    if (!fromRes.isNull() && qMax(fromRes.width(), fromRes.height()) >= 64)
        return fromRes.scaled(side, side, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap pm(side, side);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRect r(0, 0, side, side);
    const int margin = qMax(3, side / 14);
    const QRect box = r.adjusted(margin, margin, -margin, -margin);
    const int radius = qMax(8, side / 4);
    const int dy = qMax(1, side / 48);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 38));
    p.drawRoundedRect(box.translated(0, dy), radius, radius);

    QLinearGradient g(box.topLeft(), box.bottomRight());
    g.setColorAt(0.0, QColor(150, 88, 42));
    g.setColorAt(0.55, QColor(93, 46, 6));
    g.setColorAt(1.0, QColor(52, 28, 10));
    p.setBrush(g);
    p.drawRoundedRect(box, radius, radius);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(232, 206, 168, 220), qMax(1, side / 72)));
    p.drawRoundedRect(box, radius, radius);

    QFont f(QStringLiteral("Segoe UI"));
    f.setPixelSize(qMax(10, int(side / 3.35)));
    f.setBold(true);
    f.setLetterSpacing(QFont::PercentageSpacing, side >= 96 ? 108.0 : 104.0);
    p.setFont(f);

    const QString mark = QStringLiteral("RLH");
    p.setPen(QColor(18, 10, 4, 100));
    p.drawText(box.translated(0, dy), Qt::AlignCenter, mark);
    p.setPen(QColor(255, 251, 244));
    p.drawText(box, Qt::AlignCenter, mark);
    return pm;
}

constexpr int kDashboardPageIndex = 6;
/// Hauteur des QTextEdit suivie du contenu : pas de mini-défilement interne, le QScrollArea du panneau défile tout.
static void wireTextEditHeightForOuterScroll(QTextEdit *te, int minH, int maxH)
{
    if (!te)
        return;
    te->setLineWrapMode(QTextEdit::WidgetWidth);
    te->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    te->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const auto bump = [te, minH, maxH]() {
        QTextDocument *const doc = te->document();
        const int vw = qMax(40, te->viewport()->width());
        doc->setTextWidth(static_cast<qreal>(vw));
        const int docH = static_cast<int>(std::ceil(static_cast<double>(doc->size().height())));
        const int pad = te->frameWidth() * 2 + 10;
        te->setFixedHeight(qBound(minH, docH + pad, maxH));
    };

    QObject::connect(te->document(), &QTextDocument::contentsChanged, te, bump);
    if (QAbstractTextDocumentLayout *dl = te->document()->documentLayout())
        QObject::connect(dl, &QAbstractTextDocumentLayout::documentSizeChanged, te, bump);
    bump();
}

static bool mwColumnExists(const QString &table, const QString &column)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME=UPPER(:t) AND COLUMN_NAME=UPPER(:c)");
    q.bindValue(":t", table);
    q.bindValue(":c", column);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

static QString pickFournisseurCol(const QStringList &candidates, const QString &fallback)
{
    for (const QString &n : candidates) {
        if (mwColumnExists(QStringLiteral("FOURNISSEURS"), n))
            return n;
    }
    return fallback;
}

static QString pickExistingColumn(const QString &table, const QStringList &candidates, const QString &fallback = QString())
{
    for (const QString &n : candidates) {
        if (mwColumnExists(table, n))
            return n;
    }
    return fallback;
}

static QString unifiedSurfaceButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "  background: #f3f5f8;"
        "  border: 1px solid #c7d1de;"
        "  border-radius: 10px;"
        "  color: #000000;"
        "  font-weight: 600;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; }"
        "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; }");
}

static void applyUnifiedButtonStyle(QPushButton *button)
{
    if (!button)
        return;
    button->setStyleSheet(unifiedSurfaceButtonStyle());
}

static void applyUnifiedButtonStyleRecursively(QWidget *root)
{
    if (!root)
        return;
    const QList<QPushButton *> buttons = root->findChildren<QPushButton *>();
    for (QPushButton *button : buttons)
        applyUnifiedButtonStyle(button);
}

static void applyUnifiedAddButtonStyle(QPushButton *button)
{
    if (!button)
        return;
    button->setMinimumSize(180, 38);
    button->setCursor(Qt::PointingHandCursor);
    button->setIcon(button->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    button->setStyleSheet(unifiedSurfaceButtonStyle());
}

static void applyUnifiedAddButtonStyleRecursively(QWidget *root)
{
    if (!root)
        return;
    const QList<QPushButton *> buttons = root->findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text().trimmed().compare(QStringLiteral("Ajouter"), Qt::CaseInsensitive) == 0)
            applyUnifiedAddButtonStyle(button);
    }
}

static void applyTopColumnFilter(QTableWidget *table, const QString &needleRaw, int column)
{
    if (!table)
        return;
    const QString needle = needleRaw.trimmed();
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->isRowHidden(row))
            continue;
        if (needle.isEmpty())
            continue;

        bool match = false;
        if (column >= 0 && column < table->columnCount()) {
            if (QTableWidgetItem *it = table->item(row, column))
                match = it->text().contains(needle, Qt::CaseInsensitive);
        } else {
            for (int c = 0; c < table->columnCount(); ++c) {
                // Ignore action columns rendered with widgets.
                if (table->cellWidget(row, c))
                    continue;
                if (QTableWidgetItem *it = table->item(row, c)) {
                    if (it->text().contains(needle, Qt::CaseInsensitive)) {
                        match = true;
                        break;
                    }
                }
            }
        }
        if (!match)
            table->setRowHidden(row, true);
    }
}

struct DashboardCounts {
    int employes = 0;
    int fournisseurs = 0;
    int produits = 0;
    int clients = 0;
    int matieres = 0;
    int commandes = 0;
};

struct CatalogueDashboardStats {
    int totalProduits = 0;
    int stockDisponible = 0;
    int stockFaible = 0;
    int stockRupture = 0;
    QVector<QPair<QString, int>> topCategories;
};

static int dashScalar(QSqlDatabase &db, const QString &sql)
{
    if (!db.isOpen())
        return 0;
    QSqlQuery q(db);
    if (!q.exec(sql) || !q.next())
        return 0;
    return q.value(0).toInt();
}

static DashboardCounts fetchDashboardCounts(QSqlDatabase &db)
{
    DashboardCounts c;
    c.employes = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM EMPLOYES"));
    c.fournisseurs = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM FOURNISSEURS"));
    c.produits = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM PRODUITS"));
    c.clients = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM CLIENT"));
    c.matieres = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM MATIERES_PREMIERES"));
    c.commandes = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM COMMANDES"));
    return c;
}

static CatalogueDashboardStats fetchCatalogueDashboardStats(QSqlDatabase &db)
{
    CatalogueDashboardStats s;
    if (!db.isOpen())
        return s;

    s.totalProduits = dashScalar(db, QStringLiteral("SELECT COUNT(*) FROM PRODUITS"));
    s.stockDisponible = dashScalar(db,
                                   QStringLiteral("SELECT COUNT(*) FROM STOCK WHERE NVL(QTE_DISPONIBLE, 0) > 5"));
    s.stockFaible = dashScalar(db,
                               QStringLiteral("SELECT COUNT(*) FROM STOCK WHERE NVL(QTE_DISPONIBLE, 0) BETWEEN 1 AND 5"));
    s.stockRupture = dashScalar(db,
                                QStringLiteral("SELECT COUNT(*) FROM STOCK WHERE NVL(QTE_DISPONIBLE, 0) <= 0"));

    QSqlQuery q(db);
    const QString sql = QStringLiteral(
        "SELECT NVL(TRIM(CATEGORIE), 'N/A') AS CAT, COUNT(*) AS NB "
        "FROM PRODUITS "
        "GROUP BY NVL(TRIM(CATEGORIE), 'N/A') "
        "ORDER BY NB DESC");
    if (q.exec(sql)) {
        while (q.next()) {
            s.topCategories.append({q.value(0).toString(), q.value(1).toInt()});
            if (s.topCategories.size() >= 4)
                break;
        }
    }

    return s;
}

static QMap<QDate, int> fetchOrderCountsByDay(QString *errorMessage = nullptr)
{
    QMap<QDate, int> counts;
    QSqlQuery q;
    if (!q.exec(QStringLiteral(
            "SELECT TO_CHAR(TRUNC(DATE_COMMANDE), 'YYYY-MM-DD') AS JOUR, COUNT(*) "
            "FROM COMMANDES GROUP BY TRUNC(DATE_COMMANDE) ORDER BY TRUNC(DATE_COMMANDE)"))) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return counts;
    }
    while (q.next()) {
        const QDate d = QDate::fromString(q.value(0).toString(), QStringLiteral("yyyy-MM-dd"));
        if (!d.isValid())
            continue;
        counts.insert(d, q.value(1).toInt());
    }
    return counts;
}

static QVector<QPair<QString, int>> fetchClientCategorySlices(QSqlDatabase &db)
{
    QVector<QPair<QString, int>> out;
    if (!db.isOpen())
        return out;
    QSqlQuery q(db);
    const QString sql = QStringLiteral(
        "SELECT NVL(TRIM(CATEGORIE), 'N/A'), COUNT(*) FROM CLIENT GROUP BY NVL(TRIM(CATEGORIE), 'N/A') ORDER BY 2 DESC");
    if (q.exec(sql)) {
        while (q.next())
            out.append({q.value(0).toString(), q.value(1).toInt()});
    }
    return out;
}

static QVector<QPair<QDate, int>> fetchOrdersByDate(QSqlDatabase &db)
{
    QVector<QPair<QDate, int>> out;
    if (!db.isOpen())
        return out;

    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT TRUNC(DATE_COMMANDE), COUNT(*) FROM COMMANDES "
            "GROUP BY TRUNC(DATE_COMMANDE) ORDER BY TRUNC(DATE_COMMANDE) ASC"))) {
        return out;
    }
    while (q.next()) {
        const QDate d = q.value(0).toDate();
        if (!d.isValid())
            continue;
        out.append({d, q.value(1).toInt()});
    }
    while (out.size() > 30)
        out.removeFirst();
    return out;
}

static QVector<QPair<QString, int>> fetchProductsByType(QSqlDatabase &db)
{
    QVector<QPair<QString, int>> out;
    if (!db.isOpen())
        return out;
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT NVL(TRIM(TYPE_CUIR), 'N/A'), COUNT(*) FROM PRODUITS "
            "GROUP BY NVL(TRIM(TYPE_CUIR), 'N/A') ORDER BY 2 DESC"))) {
        return out;
    }
    while (q.next())
        out.append({q.value(0).toString(), q.value(1).toInt()});
    while (out.size() > 8)
        out.removeLast();
    return out;
}

static void clearChartSeriesAndAxes(QChart *chart)
{
    if (!chart)
        return;
    const QList<QAbstractSeries *> seriesList = chart->series();
    for (QAbstractSeries *s : seriesList) {
        chart->removeSeries(s);
        delete s;
    }
    const QList<QAbstractAxis *> axes = chart->axes();
    for (QAbstractAxis *ax : axes) {
        chart->removeAxis(ax);
        delete ax;
    }
}

static double calculDistance(double lat1, double lon1, double lat2, double lon2)
{
    static const double kEarthRadiusKm = 6371.0;
    static const double kPi = 3.14159265358979323846;
    const auto degToRad = [](double deg) { return deg * kPi / 180.0; };
    const double dLat = degToRad(lat2 - lat1);
    const double dLon = degToRad(lon2 - lon1);
    const double a = std::pow(std::sin(dLat / 2.0), 2)
                     + std::cos(degToRad(lat1)) * std::cos(degToRad(lat2))
                           * std::pow(std::sin(dLon / 2.0), 2);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return kEarthRadiusKm * c;
}

}

static QFrame *makeKpiCard(QWidget *parent, const QString &emoji, const QString &title, QLabel **valueLabel)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("dashKpiCard"));
    card->setMinimumHeight(104);
    card->setStyleSheet(QStringLiteral(
        "QFrame#dashKpiCard {"
        "  background-color: #ffffff;"
        "  border: 1px solid #e0d4c4;"
        "  border-radius: 14px;"
        "}"));
    auto *outer = new QVBoxLayout(card);
    outer->setContentsMargins(16, 14, 16, 14);
    outer->setSpacing(4);
    auto *head = new QHBoxLayout();
    auto *em = new QLabel(emoji, card);
    em->setStyleSheet(QStringLiteral("font-size: 22px; border: none; background: transparent;"));
    auto *tl = new QLabel(title, card);
    tl->setObjectName(QStringLiteral("dashKpiTitle"));
    tl->setStyleSheet(QStringLiteral(
        "font-size: 11px; color: #6b5345; border: none; background: transparent; font-weight: 600;"));
    head->addWidget(em);
    head->addWidget(tl, 1, Qt::AlignLeft | Qt::AlignVCenter);
    outer->addLayout(head);
    auto *val = new QLabel(QStringLiteral("0"), card);
    val->setObjectName(QStringLiteral("dashKpiValue"));
    val->setStyleSheet(QStringLiteral(
        "font-size: 28px; font-weight: 700; color: rgb(88, 41, 0); border: none; background: transparent;"));
    outer->addWidget(val);
    *valueLabel = val;
    return card;
}

static QFrame *makeDashboardChartCard(QWidget *parent, const QString &titleText, QWidget *content, int minContentHeight = 260)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("dashChartCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#dashChartCard { background: #ffffff; border: 1px solid #e0d4c4; border-radius: 16px; }"));
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(22);
    shadow->setOffset(0, 7);
    shadow->setColor(QColor(40, 25, 10, 30));
    card->setGraphicsEffect(shadow);
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(10);
    auto *tl = new QLabel(titleText, card);
    tl->setObjectName(QStringLiteral("dashCardTitle"));
    tl->setStyleSheet(QStringLiteral(
        "font-size: 14px; font-weight: 700; color: #5a3a22; border: none; background: transparent;"));
    content->setMinimumHeight(minContentHeight);
    lay->addWidget(tl);
    lay->addWidget(content, 1);
    return card;
}

/*
 * ---------------------------------------------------------------------------
 * Client Scoring System (table Clients : colonnes SCORE et STATUS)
 * ---------------------------------------------------------------------------
 * Declenchement : bouton "Analyze Clients" dans la barre du haut de la page Clients.
 *
 * SCORE (entier 0 a 100) :
 *   +40  dernier paiement enregistre (CLIENT_PAIEMENTS) il y a <= 3 mois
 *   +20  dernier paiement entre 3 et 12 mois
 *   +30  numero de telephone valide selon l API Numverify (cle : NUMVERIFY_API_KEY ou NUMVERIFY-KEY).
 *        Entre deux appels, un ecart minimal (defaut 1200 ms) limite le HTTP 429 ; surcharge via
 *        NUMVERIFY_MIN_INTERVAL_MS. En cas de 429, quelques reprises avec attente sont tentees.
 *   -100 si le champ STATUTCLIENT contient "block" (insensible a la casse), puis score borne a [0,100]
 *
 * STATUS (libelle affiche) : derive du score, sauf si bloque
 *   VIP      score 80 a 100
 *   Active   score 50 a 79
 *   Medium   score 20 a 49
 *   Inactive score < 20
 *   Blocked  client bloque (ignore les seuils ci-dessus)
 *
 * Couleurs de ligne (apres analyse, voir applyClientScoringResults) :
 *   VIP = violet pastel, Active = vert, Medium = jaune, Inactive = rouge, Blocked = gris
 *
 * Sans analyse : SCORE affiche "-" et STATUS "Not analyzed". Relancer "Analyze Clients" pour recalculer.
 * ---------------------------------------------------------------------------
 */

namespace {

/// Evite d afficher la cle API si QNetworkReply inclut l URL dans errorString().
static QString numverifyRedactSecrets(const QString &msg)
{
    QString m(msg);
    static const QRegularExpression accessKeyRe(
        QStringLiteral(R"(access_key=[^&\s#\"']*)"),
        QRegularExpression::CaseInsensitiveOption);
    m.replace(accessKeyRe, QStringLiteral("access_key=***"));
    return m;
}

static int numverifyMinIntervalMs()
{
    bool ok = false;
    const int v = qEnvironmentVariableIntValue("NUMVERIFY_MIN_INTERVAL_MS", &ok);
    if (ok && v >= 0 && v <= 60000)
        return v;
    return 1200;
}

static QMutex g_numverifyThrottleMutex;
static qint64 g_numverifyLastRequestEndMs = 0;

static void numverifyThrottleBeforeNextRequest()
{
    QMutexLocker locker(&g_numverifyThrottleMutex);
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 waitMs = g_numverifyLastRequestEndMs + static_cast<qint64>(numverifyMinIntervalMs()) - now;
    if (waitMs > 0)
        QThread::msleep(static_cast<unsigned long>(qMin(waitMs, qint64(60000))));
}

static void numverifyMarkRequestEnded()
{
    QMutexLocker locker(&g_numverifyThrottleMutex);
    g_numverifyLastRequestEndMs = QDateTime::currentMSecsSinceEpoch();
}

} // namespace

/// Numverify attend en general un numero international (ex. +216XXXXXXXX pour la Tunisie).
static QString normalizePhoneForNumverify(const QString &raw)
{
    QString s = raw.trimmed();
    s.remove(QLatin1Char(' '));
    s.remove(QLatin1Char('-'));
    s.remove(QLatin1Char('.'));
    if (s.isEmpty())
        return QString();
    if (s.startsWith(QStringLiteral("00")))
        s = QLatin1Char('+') + s.mid(2);
    if (s.startsWith(QLatin1Char('+')))
        return s;

    QString digitsOnly = raw;
    digitsOnly.remove(QRegularExpression(QStringLiteral("[^0-9]")));
    if (digitsOnly.isEmpty())
        return QString();
    if (digitsOnly.length() == 8)
        return QStringLiteral("+216") + digitsOnly;
    if (digitsOnly.length() == 9 && digitsOnly.startsWith(QLatin1Char('0')))
        return QStringLiteral("+216") + digitsOnly.mid(1);
    if (digitsOnly.length() == 9 && digitsOnly.startsWith(QLatin1Char('2')))
        return QStringLiteral("+216") + digitsOnly;
    return QLatin1Char('+') + digitsOnly;
}

struct ClientScoringRow
{
    int id = 0;
    QString phone;
    QString statutClient;
    QDate lastPurchaseDate;
};

static bool fetchClientScoringRowsFromDb(QList<ClientScoringRow> &rows, QString *errorMessage)
{
    rows.clear();
    QSqlQuery query;
    if (!query.exec(QStringLiteral(
            "SELECT C.ID, C.TELEPHONE, C.STATUTCLIENT, MAX(P.DATE_PAIEMENT) "
            "FROM CLIENT C "
            "LEFT JOIN CLIENT_PAIEMENTS P ON P.CLIENT_ID = C.ID "
            "GROUP BY C.ID, C.TELEPHONE, C.STATUTCLIENT "
            "ORDER BY C.ID"))) {
        if (errorMessage)
            *errorMessage = query.lastError().text();
        return false;
    }

    while (query.next()) {
        ClientScoringRow row;
        row.id = query.value(0).toInt();
        row.phone = query.value(1).toString().trimmed();
        row.statutClient = query.value(2).toString().trimmed();
        row.lastPurchaseDate = query.value(3).toDate();
        rows.push_back(row);
    }
    return true;
}

/// Points du score Client Scoring (voir bloc de documentation "Client Scoring System" plus haut).
static int computeClientScoreValue(const QDate &lastPurchaseDate, bool isPhoneValid, bool isBlocked)
{
    int score = 0;
    if (lastPurchaseDate.isValid()) {
        const int monthsAgo = lastPurchaseDate.daysTo(QDate::currentDate()) / 30;
        if (monthsAgo <= 3)
            score += 40;
        else if (monthsAgo <= 12)
            score += 20;
    }
    if (isPhoneValid)
        score += 30;
    if (isBlocked)
        score -= 100;
    return qBound(0, score, 100);
}

/// Libelle STATUS affiche dans la table ; seuils alignes avec la spec Client Scoring System.
static QString computeClientStatusLabel(int score, bool isBlocked)
{
    if (isBlocked)
        return QStringLiteral("Blocked"); // prioritaire : client bloque
    if (score >= 80)
        return QStringLiteral("VIP"); // 80 - 100
    if (score >= 50)
        return QStringLiteral("Active"); // 50 - 79
    if (score >= 20)
        return QStringLiteral("Medium"); // 20 - 49
    return QStringLiteral("Inactive"); // < 20
}

// ------------------- Constructeur -------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupHubAndBodyStack();
    setupLoginPageChrome();
    installProduitsPageResponsiveLayout();
    installMatieresPageResponsiveLayout();
    m_networkAccessManager = new QNetworkAccessManager(this);
    m_googleCalendarService = new GoogleCalendarService(this);
    connect(m_googleCalendarService, &GoogleCalendarService::authenticationFinished, this,
            [this](bool ok, const QString &message) {
                if (ok) {
                    QMessageBox::information(this, QStringLiteral("Google Calendar"), message);
                    /// Charger tout de suite les événements dans le calendrier Qt (liste + grille).
                    QTimer::singleShot(0, this, [this]() {
                        if (m_googleCalendarService)
                            m_googleCalendarService->syncCalendar();
                    });
                } else {
                    QMessageBox::warning(this, QStringLiteral("Google Calendar"), message);
                }
            });
    connect(m_googleCalendarService, &GoogleCalendarService::errorOccurred, this, [this](const QString &message) {
        QMessageBox::warning(this, QStringLiteral("Google Calendar"), message);
    });
    connect(m_googleCalendarService, &GoogleCalendarService::eventsReady, this, &MainWindow::onGoogleCalendarEventsReady);
    connect(m_googleCalendarService, &GoogleCalendarService::eventCreated, this, &MainWindow::onGoogleEventCreated);
    m_fournisseurApiService = new FournisseurApiService(m_networkAccessManager, this);
    m_chatbotService = new ChatbotService(m_networkAccessManager, this);
    m_whatsappBusinessService = new WhatsAppBusinessService(m_networkAccessManager, this);
    connect(m_whatsappBusinessService, &WhatsAppBusinessService::sendFinished, this,
            [this](bool ok, const QString &detail) {
                if (ok) {
                    QString msg = detail;
                    if (!m_lastWhatsAppDestinationDisplay.isEmpty())
                        msg += QStringLiteral("\n\nDestinataire utilisé : %1").arg(m_lastWhatsAppDestinationDisplay);
                    msg += QStringLiteral(
                        "\n\nSi vous ne voyez rien sur le téléphone : vérifiez le numéro WhatsApp, le sandbox Twilio "
                        "(joindre le code « join … » au numéro sandbox si besoin), et le journal des messages dans la "
                        "console Twilio.");
                    QMessageBox::information(this, QStringLiteral("WhatsApp"), msg);
                } else {
                    QMessageBox::warning(this, QStringLiteral("WhatsApp"), detail);
                }
            });
    connect(m_chatbotService, &ChatbotService::replyReady, this, [this](const QString &t) {
        if (m_chatbotSink == ChatbotSink::Produit && ui->textEdit_3)
            ui->textEdit_3->append(QStringLiteral("Assistant :\n%1\n").arg(t));
        else if (m_chatbotSink == ChatbotSink::Fournisseur && ui->chatDisplay_2)
            ui->chatDisplay_2->append(QStringLiteral("Assistant :\n%1\n").arg(t));
    });
    connect(m_fournisseurApiService, &FournisseurApiService::apiReplyReady, this,
            [this](const QString &apiName, const QString &content) {
                if (!ui->chatDisplay_2)
                    return;
                ui->chatDisplay_2->append(QStringLiteral("[%1]\n%2\n").arg(apiName, content));
            });
    // Hide default menubar / statusbar
    menuBar()->hide();
    statusBar()->hide();
    if (ui->topNavLogo)
        ui->topNavLogo->hide();

    // Stretch table columns to fill
    ui->employeeTable->horizontalHeader()->setStretchLastSection(true);
    ui->employeeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Ajustement de la table client
    ui->clientTable->horizontalHeader()->setStretchLastSection(true);
    ui->clientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->clientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->clientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->clientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->clientTable->setColumnCount(15);
    ui->clientTable->setHorizontalHeaderLabels({
        "ID", "NOM", "PRENOM", "EMAIL", "TELEPHONE", "ADRESSE", "STATUT",
        "CATEGORIE", "REMISE %", "TOTAL ACHATS", "SCORE", "STATUS",
        "SOLDE CREDIT", "SUPPR.", "MODIF."
    });
    // Infobulles sur les en-tetes : visibles au survol de "SCORE" / "STATUS" dans le tableau Clients.
    if (QTableWidgetItem *scoreHeader = ui->clientTable->horizontalHeaderItem(10)) {
        scoreHeader->setToolTip(
            QStringLiteral("Client Scoring — Score (0 a 100), apres le bouton Analyze Clients.\n"
                           "Composition : +40 dernier paiement <= 3 mois ; +20 entre 3 et 12 mois ; "
                           "+30 telephone valide (Numverify) ; -100 si client bloque.\n"
                           "\"-\" = pas encore analyse."));
    }
    if (QTableWidgetItem *statusHeader = ui->clientTable->horizontalHeaderItem(11)) {
        statusHeader->setToolTip(
            QStringLiteral("Client Scoring — Statut deduit du score :\n"
                           "VIP 80-100 ; Active 50-79 ; Medium 20-49 ; Inactive < 20 ; Blocked si bloque.\n"
                           "Couleurs des lignes : violet VIP, vert Active, jaune Medium, rouge Inactive, gris Blocked.\n"
                           "\"Not analyzed\" = pas encore analyse."));
    }

    connectSidebar();

    if (ui->topNavBar) {
        if (auto *navLay = qobject_cast<QHBoxLayout *>(ui->topNavBar->layout())) {
            int spacerIdx = -1;
            for (int i = 0; i < navLay->count(); ++i) {
                QLayoutItem *it = navLay->itemAt(i);
                if (it && it->spacerItem()) {
                    spacerIdx = i;
                    break;
                }
            }
            auto *btnGlobalChatbot = new QPushButton(QStringLiteral("Chatbot"), ui->topNavBar);
            btnGlobalChatbot->setObjectName(QStringLiteral("btnGlobalChatbot"));
            btnGlobalChatbot->setMinimumHeight(36);
            btnGlobalChatbot->setCursor(Qt::PointingHandCursor);
            btnGlobalChatbot->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: transparent;"
                "  color: rgb(130, 80, 45);"
                "  text-align: center;"
                "  padding: 8px 14px;"
                "  border: 1px solid rgba(93, 46, 6, 0.35);"
                "  border-radius: 8px;"
                "  font: 600 10pt \"Segoe UI\";"
                "}"
                "QPushButton:hover {"
                "  background-color: rgba(200, 90, 60, 0.12);"
                "  border-color: rgba(93, 46, 6, 0.55);"
                "}"
                "QPushButton:pressed {"
                "  background-color: rgba(200, 90, 60, 0.18);"
                "}"));
            navLay->insertWidget(spacerIdx >= 0 ? spacerIdx : qMax(0, navLay->count() - 2), btnGlobalChatbot);
            connect(btnGlobalChatbot, &QPushButton::clicked, this, &MainWindow::onGlobalChatbotClicked);

            // Cree le bouton Maps s'il n'existe pas dans le .ui.
            QPushButton *btnMaps = this->findChild<QPushButton *>(QStringLiteral("btnMaps"));
            if (!btnMaps) {
                btnMaps = new QPushButton(QStringLiteral("Maps"), ui->topNavBar);
                btnMaps->setObjectName(QStringLiteral("btnMaps"));
                btnMaps->setMinimumHeight(36);
                btnMaps->setCursor(Qt::PointingHandCursor);
                navLay->insertWidget(spacerIdx >= 0 ? spacerIdx : qMax(0, navLay->count() - 2), btnMaps);
            }
            // Meme style que Chatbot (sinon bouton du .ui garde le theme systeme bleu).
            btnMaps->setStyleSheet(btnGlobalChatbot->styleSheet());

            // Branche toujours le bouton Maps vers openMap.
            connect(btnMaps, &QPushButton::clicked, this, &MainWindow::openMap, Qt::UniqueConnection);
        }
    }

    applyAuthenticationUiState(false);
    if (ui->lineEdit_6)
        ui->lineEdit_6->setEchoMode(QLineEdit::Password);
    if (ui->lineEdit_5)
        ui->lineEdit_5->setFocus();
    if (ui->lineEdit_6)
        connect(ui->lineEdit_6, &QLineEdit::returnPressed, this, &MainWindow::on_btnAjouter_8_clicked);
    if (ui->btnForgotPassword)
        connect(ui->btnForgotPassword, &QPushButton::clicked, this, &MainWindow::onForgotPasswordRequested);
    if (ui->btnFaceLogin)
        connect(ui->btnFaceLogin, &QPushButton::clicked, this, &MainWindow::onFaceLoginRequested);
    installClientPageResponsiveLayout();
    setupClientValidators();
    setupClientFicheScrollAndHeader();
    setupClientUiEnhancements();
    connect(ui->clientTable, &QTableWidget::cellClicked, this, &MainWindow::on_clientTable_cellClicked);

    // Start on Accueil (page 0)
    ui->contentStack->setCurrentIndex(0);
    setActiveButton(ui->btnAccueil);

    // Connexion à la base de données
    if (connectToDatabase())
        qDebug() << "Connexion BD réussie";
    else
        qDebug() << "Connexion BD échouée";

    QString err;
    if (!Client::ensureSchema(&err)) {
        QMessageBox::critical(this, "Base de données", "Erreur de préparation du schéma CLIENT:\n" + err);
    }

    if (db.isOpen()) {
        QString cerr;
        if (!CommerceStore::ensureSchema(&cerr)) {
            QMessageBox::critical(this, "Base de données", "Erreur schéma commerce (produits/commandes):\n" + cerr);
        } else {
            CommerceStore::seedDemoCatalogIfEmpty(&cerr);
        }
    }

    if (db.isOpen()) {
        gestionFournisseur = new FournisseurManager(ui, this);
        setupFournisseurDashboardBlock();
        connect(gestionFournisseur, &FournisseurManager::fournisseursChanged, this, [this]() {
            refreshFournisseurDashboard();
            geocodePendingFournisseurs();
            injectFournisseursLeafletMarkers();
        });
        refreshFournisseurDashboard();
        geocodePendingFournisseurs();
    } else {
        QMessageBox::warning(this, "Fournisseurs", "Module fournisseurs desactive: connexion base de donnees fermee.");
    }

    loadClients();
    refreshClientRecommendations();

    setupProduitPage();
    setupMatierePage();
    setupEmployePage();
    setupAdministratorDashboard();
    installEmployesPageClientLikeLayout();
    installFournisseursPageClientLikeLayout();
    installMatieresPageClientLikeLayout();

    // Uniformisation visuelle: retirer les blocs annexes non presents sur la page Clients.
    const QList<QWidget *> nonClientBlocks = {
        static_cast<QWidget *>(ui->chatGroupBox),
        static_cast<QWidget *>(ui->statsGroupBox),
        static_cast<QWidget *>(ui->chatGroupBox_2),
        static_cast<QWidget *>(ui->chatGroupBox_5),
        static_cast<QWidget *>(ui->statsGroupBox_2),
    };
    for (QWidget *w : nonClientBlocks) {
        if (!w)
            continue;
        w->hide();
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

    auto applyClientLikePageTheme = [this](QWidget *page,
                                           QTableWidget *table,
                                           QGroupBox *formBox,
                                           QLineEdit *searchEdit,
                                           QPushButton *searchBtn) {
        if (!page)
            return;
        const QString moduleStyle = QStringLiteral(
            "QWidget { background: #ffffff; }"
            "#clientTopNavBar { background: #ffffff; border: 1px solid #e6eaf0; border-radius: 10px; }"
            "#clientFormCard, #clientTableZone {"
            "  background: #ffffff;"
            "  border: 1px solid #e7ecf2;"
            "  border-radius: 12px;"
            "}"
            "QGroupBox { background: #ffffff; border: 1px solid #e7ecf2; border-radius: 12px; font-weight: 700; color: #000000; }"
            "QLineEdit, QComboBox, QDateEdit, QDateTimeEdit {"
            "  min-height: 32px;"
            "  border: 1px solid #d5dbe5;"
            "  border-radius: 7px;"
            "  padding: 0 8px;"
            "  background: #ffffff;"
            "}"
            "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDateTimeEdit:focus { border-color: #b8c4d3; }"
            "QPushButton {"
            "  background: #f3f5f8;"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  color: #1f2937;"
            "  font-weight: 600;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; color: #000000; }");
        page->setStyleSheet(page->styleSheet() + moduleStyle);

        if (table) {
            table->setAlternatingRowColors(true);
            table->setShowGrid(true);
            table->verticalHeader()->setVisible(false);
            table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
            table->setStyleSheet(QStringLiteral(
                "QTableWidget {"
                "  border: 1px solid #dce3ec;"
                "  border-radius: 8px;"
                "  background-color: #ffffff;"
                "  gridline-color: #edf1f6;"
                "  selection-background-color: #ead7c3;"
                "  selection-color: #1f2937;"
                "  alternate-background-color: #ffffff;"
                "}"
                "QTableWidget::item { padding: 6px; }"
                "QHeaderView::section {"
                "  background-color: #582900;"
                "  color: #ffffff;"
                "  font-weight: 700;"
                "  border: none;"
                "  padding: 6px;"
                "}"));
        }

        if (formBox) {
            formBox->setMinimumWidth(300);
            formBox->setMaximumWidth(380);
            formBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        }
        if (searchEdit) {
            searchEdit->setMinimumWidth(220);
            searchEdit->setMaximumWidth(16777215);
            searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        }
        if (searchBtn) {
            searchBtn->setMinimumSize(116, 34);
            searchBtn->setMaximumWidth(116);
            searchBtn->setCursor(Qt::PointingHandCursor);
            searchBtn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  border: 1px solid transparent;"
                "  border-radius: 8px;"
                "  background: #f3f5f8;"
                "  color: #1f2937;"
                "  font-weight: 600;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; color: #000000; }"
                "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; color: #000000; }"));
        }
    };

    applyClientLikePageTheme(ui->pageEmployes, ui->employeeTable, ui->employeeFormBox, ui->lineEditSearch, ui->btnRechercher);
    applyClientLikePageTheme(ui->page, ui->employeeTable_2, ui->employeeFormBox_2, ui->lineEditSearch_2, ui->btnRechercher_6);
    applyClientLikePageTheme(ui->page_2, ui->employeeTable_4, ui->employeeFormBox_4, ui->lineEditSearch_5, ui->btnRechercher_4);
    applyClientLikePageTheme(ui->page_3, ui->employeeTable_5, ui->employeeFormBox_5, ui->lineEditSearch_6, ui->btnRechercher_5);

    if (ui->matieresRightPanel_mp) {
        ui->matieresRightPanel_mp->hide();
        ui->matieresRightPanel_mp->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

    // Uniformiser la couleur de fond de tous les boutons (comme les boutons Rechercher/Reset).
    applyUnifiedButtonStyleRecursively(this);
    // Uniformiser specifiquement tous les boutons "Ajouter".
    applyUnifiedAddButtonStyleRecursively(this);

    applyGlobalAppearanceFromSettings();
    installApplicationTranslators();
    if (ui->btnFaceLogin)
        refreshFaceLoginButtonVisibility();
}

void MainWindow::setupFournisseurDashboardBlock()
{
    if (!ui->chatGroupBox_2 || !ui->chatLayout_2) {
        return;
    }

    // Retouche 1: labels metier fournisseurs (affichage proche du projet ami).
    if (ui->labelCIN_2) ui->labelCIN_2->setText("Code partenaire");
    if (ui->labelNom_2) ui->labelNom_2->setText("Raison sociale");
    if (ui->labelPrenom_2) ui->labelPrenom_2->setText("Score fiabilite");
    if (ui->labelSexe_2) ui->labelSexe_2->setText("Email achats");
    if (ui->labelSalaire_2) ui->labelSalaire_2->setText("Zone logistique");
    if (ui->labelDateEmbauche_2) ui->labelDateEmbauche_2->setText("SLA livraison (jours)");
    if (ui->lineEditCIN_2) ui->lineEditCIN_2->setPlaceholderText("ex: FRN-001");
    if (ui->lineEditNom_2) ui->lineEditNom_2->setPlaceholderText("ex: Leather Supply Pro");
    if (ui->lineEditPrenom_2) ui->lineEditPrenom_2->setPlaceholderText("0 a 100");
    if (ui->lineEdit) ui->lineEdit->setPlaceholderText("contact@fournisseur.com");
    if (ui->lineEditEmail_2) ui->lineEditEmail_2->setPlaceholderText("ex: Maghreb / Europe");
    if (ui->lineEdit_2) ui->lineEdit_2->setPlaceholderText("ex: 5");

    if (ui->employesTitle_2) ui->employesTitle_2->setText("Gestion des fournisseurs");
    if (ui->employeeFormBox_2) ui->employeeFormBox_2->setTitle("Fiche Fournisseurs");
    if (ui->btnRechercher_6) ui->btnRechercher_6->setText("Rechercher");
    if (ui->lineEditSearch_2) {
        ui->lineEditSearch_2->setPlaceholderText("Rechercher...");
    }

    // Filtre rapide comme le projet ami
    if (ui->employesHeaderRow_2 && ui->page && !ui->page->findChild<QComboBox *>("comboFiltreFournisseur")) {
        auto *comboFiltre = new QComboBox(ui->page);
        comboFiltre->setObjectName("comboFiltreFournisseur");
        comboFiltre->setMinimumHeight(32);
        comboFiltre->setMinimumWidth(140);
        comboFiltre->addItems({"Tous", "Score >= 80", "SLA <= 7", "Risque eleve", "Risque faible"});
        auto *filterRow = new QHBoxLayout();
        filterRow->setSpacing(8);
        filterRow->setContentsMargins(0, 0, 0, 0);
        auto *filterLabel = new QLabel("Filtre", ui->page);
        filterLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 700;"));
        filterRow->addWidget(filterLabel, 0);
        filterRow->addWidget(comboFiltre, 0);
        ui->employesHeaderRow_2->insertLayout(2, filterRow);
    }

    if (ui->chatGroupBox_2) {
        ui->chatGroupBox_2->setTitle(QStringLiteral("Pilotage fournisseur (Notes)"));
        ui->chatGroupBox_2->setStyleSheet(QString());
    }
    if (ui->chatLayout_2) {
        ui->chatLayout_2->setContentsMargins(10, 8, 10, 8);
        ui->chatLayout_2->setSpacing(6);
    }
    // Bloc pilotage fournisseur (notes + IA + contexte + bouton).
    if (ui->labelAssistantMode_2) ui->labelAssistantMode_2->show();
    if (ui->comboAiProvider_2) ui->comboAiProvider_2->show();
    if (QWidget *modeRow = ui->chatGroupBox_2->findChild<QWidget *>(QStringLiteral("assistantModeRow_2")))
        modeRow->show();
    if (ui->chatDisplay_2) {
        ui->chatDisplay_2->show();
        ui->chatDisplay_2->setReadOnly(false);
        ui->chatDisplay_2->setPlaceholderText(
            QStringLiteral("Saisissez les notes metier: incidents, plans d'action, conditions commerciales..."));
        ui->chatDisplay_2->setMinimumHeight(72);
        ui->chatDisplay_2->setMaximumHeight(130);
    }
    if (ui->lineEditChat_2)
        ui->lineEditChat_2->hide();
    if (ui->btnSendChat_2)
        ui->btnSendChat_2->hide();

    if (ui->chatGroupBox_2->findChild<QWidget *>(QStringLiteral("fournisseurDashboardBlock"))) {
        setupSmartMapFournisseursUi();
        return;
    }

    auto *title = new QLabel(QStringLiteral("Statistiques & Dashboard"), ui->chatGroupBox_2);
    title->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 700;"));

    m_fournisseurStatsSummary = new QLabel(ui->chatGroupBox_2);
    m_fournisseurStatsSummary->setWordWrap(true);
    m_fournisseurStatsSummary->setStyleSheet(QStringLiteral("font-size: 11px;"));
    m_fournisseurStatsSummary->setText(QString());
    m_fournisseurStatsSummary->hide();

    m_fournisseurStatsChartsContainer = new QWidget(ui->chatGroupBox_2);
    auto *chartLay = new QVBoxLayout(m_fournisseurStatsChartsContainer);
    chartLay->setContentsMargins(0, 0, 0, 0);
    chartLay->setSpacing(6);
    m_fournisseurBarChart = new SupplierBarChartWidget(m_fournisseurStatsChartsContainer);
    m_fournisseurPieChart = new SupplierPieChartWidget(m_fournisseurStatsChartsContainer);
    chartLay->addWidget(m_fournisseurBarChart);
    chartLay->addWidget(m_fournisseurPieChart);

    auto *dashboardBlock = new QWidget(ui->chatGroupBox_2);
    dashboardBlock->setObjectName(QStringLiteral("fournisseurDashboardBlock"));
    dashboardBlock->setStyleSheet(QStringLiteral(
        "QWidget#fournisseurDashboardBlock { background-color: #f9f4e8; border: 1px solid #d9c8b0; "
        "border-radius: 6px; }"));
    dashboardBlock->setAttribute(Qt::WA_StyledBackground, true);
    auto *blockLay = new QVBoxLayout(dashboardBlock);
    blockLay->setContentsMargins(12, 12, 12, 12);
    blockLay->setSpacing(8);
    blockLay->addWidget(title);
    blockLay->addWidget(m_fournisseurStatsSummary);
    blockLay->addWidget(m_fournisseurStatsChartsContainer);
    ui->chatLayout_2->addWidget(dashboardBlock);
    setupSmartMapFournisseursUi();
}

void MainWindow::refreshFournisseurDashboard()
{
    if (!m_fournisseurStatsSummary) {
        return;
    }
    const QString cmdCol = mwColumnExists("FOURNISSEURS", "NB_COMMANDES") ? "NB_COMMANDES" : "COMMANDES";
    const QString codeCol = mwColumnExists("FOURNISSEURS", "CODE") ? "CODE" : (mwColumnExists("FOURNISSEURS", "ID") ? "ID" : "CIN");
    const QString nomCol = mwColumnExists("FOURNISSEURS", "RAISON_SOCIALE") ? "RAISON_SOCIALE" : "NOM";
    QSqlQuery q;
    if (!q.exec(QString("SELECT NVL(COUNT(*),0), NVL(SUM(NVL(%1,0)),0), "
                        "NVL(MAX(%1),0), NVL(MIN(%1),0) FROM FOURNISSEURS").arg(cmdCol))) {
        m_fournisseurStatsSummary->setText("Statistiques indisponibles.");
        return;
    }
    if (!q.next()) {
        m_fournisseurStatsSummary->setText("Aucune statistique.");
        return;
    }

    const int total = q.value(0).toInt();
    const int commandesTotal = q.value(1).toInt();
    const int maxCmd = q.value(2).toInt();
    const int minCmd = q.value(3).toInt();

    QString summaryBase = QStringLiteral("Fournisseurs : %1\nCommandes totales : %2")
                              .arg(total)
                              .arg(commandesTotal);

    if (!m_fournisseurStatsChartsContainer)
        return;

    QList<FournisseurStatsInput> rows;
    QSqlQuery qRows;
    if (!qRows.exec(QString("SELECT TO_CHAR(%1), TO_CHAR(%2), NVL(%3,0) FROM FOURNISSEURS ORDER BY NVL(%3,0) DESC")
                        .arg(codeCol, nomCol, cmdCol))) {
        return;
    }
    while (qRows.next()) {
        FournisseurStatsInput r;
        r.id = qRows.value(0).toString();
        r.nom = qRows.value(1).toString();
        r.commandes = qRows.value(2).toInt();
        rows.append(r);
    }
    const FournisseurStatsSnapshot snap = FournisseurStatsCalculator::compute(rows);

    const QString fiabCol = pickFournisseurCol({QStringLiteral("FIABILITE"),
                                                 QStringLiteral("PRENOM"),
                                                 QStringLiteral("TELEPHONE"),
                                                 QStringLiteral("CONTACT")},
                                                QStringLiteral("PRENOM"));
    const QString emailCol = pickFournisseurCol({QStringLiteral("EMAIL_ACHATS"),
                                                 QStringLiteral("EMAIL"),
                                                 QStringLiteral("MAIL")},
                                                QStringLiteral("EMAIL_ACHATS"));
    const QString zoneCol = pickFournisseurCol({QStringLiteral("PAYS"),
                                                QStringLiteral("ZONE_GEO"),
                                                QStringLiteral("ZONE")},
                                               QStringLiteral("PAYS"));
    const QString slaCol = pickFournisseurCol({QStringLiteral("SLA_JOURS"), QStringLiteral("SLA")},
                                              QStringLiteral("SLA_JOURS"));

    QList<FournisseurPilotageRow> pilotageRows;
    QSqlQuery qPilot;
    const QString pilotSql =
        QStringLiteral("SELECT TO_CHAR(%1), TO_CHAR(%2), TRIM(TO_CHAR(%3)), NVL(%4,0), NVL(%5,0), TRIM(TO_CHAR(%6)), "
                       "TRIM(TO_CHAR(%7)) FROM FOURNISSEURS")
            .arg(codeCol, nomCol, fiabCol, slaCol, cmdCol, emailCol, zoneCol);
    if (qPilot.exec(pilotSql)) {
        while (qPilot.next()) {
            FournisseurPilotageRow pr;
            pr.code = qPilot.value(0).toString().trimmed();
            pr.nom = qPilot.value(1).toString().trimmed();
            bool okF = false;
            pr.fiabilite = qPilot.value(2).toString().trimmed().toInt(&okF);
            if (!okF)
                pr.fiabilite = -1;
            pr.slaJours = qPilot.value(3).toInt();
            pr.commandes = qPilot.value(4).toInt();
            pr.email = qPilot.value(5).toString().trimmed();
            pr.zone = qPilot.value(6).toString().trimmed();
            pilotageRows.append(pr);
        }
    }
    Q_UNUSED(summaryBase)
    Q_UNUSED(pilotageRows)

    QVector<QPair<QString, int>> bars;
    int n = 0;
    for (const auto &r : snap.ranked) {
        if (n++ >= 8) break;
        QString shortNom = r.nom;
        if (shortNom.size() > 12) shortNom = shortNom.left(11) + QChar(0x2026);
        bars.append(qMakePair(shortNom, r.commandes));
    }
    if (bars.isEmpty()) bars.append(qMakePair(QString("-"), 0));
    m_fournisseurBarChart->setChartTitle("Commandes par fournisseur");
    m_fournisseurBarChart->setBars(bars);

    QVector<QPair<QString, int>> slices;
    for (const auto &r : snap.ranked) {
        if (r.commandes > 0) slices.append(qMakePair(r.nom, r.commandes));
        if (slices.size() >= 6) break;
    }
    if (slices.isEmpty()) slices.append(qMakePair(QString("-"), 0));
    m_fournisseurPieChart->setChartTitle("Repartition des commandes");
    m_fournisseurPieChart->setSlices(slices);
    refreshSmartMapFournisseurs();
    injectFournisseursLeafletMarkers();
    onStatisticsDataChanged();
}

void MainWindow::setupSmartMapFournisseursUi()
{
    if (!ui->chatLayout_2 || m_fournisseurSmartMapCanvas)
        return;

    auto *smartBlock = new QFrame(ui->chatGroupBox_2);
    smartBlock->setObjectName(QStringLiteral("smartMapBlock"));
    smartBlock->setStyleSheet(QStringLiteral(
        "QFrame#smartMapBlock { background:#fffdf8; border:1px solid #dccab4; border-radius:8px; }"));
    auto *smartLay = new QVBoxLayout(smartBlock);
    smartLay->setContentsMargins(10, 10, 10, 10);
    smartLay->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("Smart Map fournisseurs"), smartBlock);
    title->setStyleSheet(QStringLiteral("font-size:12px;font-weight:700;color:#5a3a22;"));
    smartLay->addWidget(title);

    m_fournisseurOpenSmartMapButton = new QPushButton(QStringLiteral("Ouvrir interface Smart Map"), smartBlock);
    m_fournisseurOpenSmartMapButton->setMinimumHeight(30);
    m_fournisseurOpenSmartMapButton->setCursor(Qt::PointingHandCursor);
    smartLay->addWidget(m_fournisseurOpenSmartMapButton, 0, Qt::AlignRight);

    auto *filterRow = new QHBoxLayout();
    filterRow->setContentsMargins(0, 0, 0, 0);
    filterRow->setSpacing(8);
    auto *filterLabel = new QLabel(QStringLiteral("Matiere"), smartBlock);
    filterLabel->setStyleSheet(QStringLiteral("font-size:11px;font-weight:600;"));
    m_fournisseurSmartFilter = new QComboBox(smartBlock);
    m_fournisseurSmartFilter->setMinimumHeight(28);
    filterRow->addWidget(filterLabel);
    filterRow->addWidget(m_fournisseurSmartFilter, 1);
    smartLay->addLayout(filterRow);

    m_fournisseurPlusProcheLabel = new QLabel(QStringLiteral("Plus proche : -"), smartBlock);
    m_fournisseurPlusRapideLabel = new QLabel(QStringLiteral("Plus rapide : -"), smartBlock);
    m_fournisseurPlusFiableLabel = new QLabel(QStringLiteral("Plus fiable : -"), smartBlock);
    m_fournisseurMeilleurLabel = new QLabel(QStringLiteral("Meilleur fournisseur : -"), smartBlock);
    m_fournisseurAlertesLabel = new QLabel(QStringLiteral("Alertes : -"), smartBlock);
    for (QLabel *lab : {m_fournisseurPlusProcheLabel, m_fournisseurPlusRapideLabel,
                        m_fournisseurPlusFiableLabel, m_fournisseurMeilleurLabel, m_fournisseurAlertesLabel}) {
        lab->setStyleSheet(QStringLiteral("font-size:11px;"));
        lab->setWordWrap(true);
        smartLay->addWidget(lab);
    }

    m_fournisseurSmartMapCanvas = new QLabel(smartBlock);
    m_fournisseurSmartMapCanvas->setObjectName(QStringLiteral("smartMapCanvas"));
    m_fournisseurSmartMapCanvas->setMinimumHeight(210);
    m_fournisseurSmartMapCanvas->setAlignment(Qt::AlignCenter);
    m_fournisseurSmartMapCanvas->setMouseTracking(true);
    m_fournisseurSmartMapCanvas->setStyleSheet(QStringLiteral(
        "QLabel#smartMapCanvas { background:#f8fafc; border:1px solid #d5dbe5; border-radius:6px; }"));
    m_fournisseurSmartMapCanvas->installEventFilter(this);
    smartLay->addWidget(m_fournisseurSmartMapCanvas, 1);

    auto *legend = new QLabel(QStringLiteral("Legende: vert=fiable, orange=moyen, rouge=faible, noir=magasin"), smartBlock);
    legend->setStyleSheet(QStringLiteral("font-size:10px;color:#64748b;"));
    smartLay->addWidget(legend);

    ui->chatLayout_2->addWidget(smartBlock);

    connect(m_fournisseurSmartFilter, &QComboBox::currentTextChanged, this, [this](const QString &) {
        if (m_fournisseurSmartFilterUpdating)
            return;
        refreshSmartMapFournisseurs();
    });
    connect(m_fournisseurOpenSmartMapButton, &QPushButton::clicked, this, &MainWindow::showSmartMapFournisseursDialog);
}

void MainWindow::analyserFournisseurs(QVector<Fournisseur> list)
{
    if (list.isEmpty()) {
        if (m_fournisseurPlusProcheLabel) m_fournisseurPlusProcheLabel->setText(QStringLiteral("Plus proche : -"));
        if (m_fournisseurPlusRapideLabel) m_fournisseurPlusRapideLabel->setText(QStringLiteral("Plus rapide : -"));
        if (m_fournisseurPlusFiableLabel) m_fournisseurPlusFiableLabel->setText(QStringLiteral("Plus fiable : -"));
        if (m_fournisseurMeilleurLabel) m_fournisseurMeilleurLabel->setText(QStringLiteral("Meilleur fournisseur : -"));
        if (m_fournisseurAlertesLabel) m_fournisseurAlertesLabel->setText(QStringLiteral("Alertes : -"));
        return;
    }

    const double latMagasin = 36.8065;
    const double lonMagasin = 10.1815;

    Fournisseur plusProche;
    Fournisseur plusRapide;
    Fournisseur plusFiable;
    Fournisseur meilleur;

    double minDistance = 999999.0;
    int minDelai = 999;
    int maxFiabilite = -1;

    struct CandidateScore {
        Fournisseur f;
        double distance = 0.0;
        double score = 0.0;
    };
    QVector<CandidateScore> candidates;
    candidates.reserve(list.size());

    double distMin = 1e9, distMax = 0.0;
    int delaiMin = 9999, delaiMax = 0;
    int cmdMin = 999999, cmdMax = 0;
    int fiabMin = 100, fiabMax = 0;

    for (const Fournisseur &f : list) {
        const double distance = (f.apiDistanceKm >= 0.0)
                                    ? f.apiDistanceKm
                                    : calculDistance(latMagasin, lonMagasin, f.latitude, f.longitude);
        CandidateScore c;
        c.f = f;
        c.distance = distance;
        candidates.push_back(c);

        distMin = qMin(distMin, distance);
        distMax = qMax(distMax, distance);
        delaiMin = qMin(delaiMin, f.delai);
        delaiMax = qMax(delaiMax, f.delai);
        cmdMin = qMin(cmdMin, f.nb_commandes);
        cmdMax = qMax(cmdMax, f.nb_commandes);
        fiabMin = qMin(fiabMin, f.fiabilite);
        fiabMax = qMax(fiabMax, f.fiabilite);

        if (distance < minDistance) {
            minDistance = distance;
            plusProche = f;
        }
        if (f.delai < minDelai) {
            minDelai = f.delai;
            plusRapide = f;
        }
        if (f.fiabilite > maxFiabilite) {
            maxFiabilite = f.fiabilite;
            plusFiable = f;
        }
    }

    const auto normalize = [](double value, double minV, double maxV) -> double {
        if (qFuzzyCompare(minV + 1.0, maxV + 1.0))
            return 0.0;
        return qBound(0.0, (value - minV) / (maxV - minV), 1.0);
    };

    double bestScore = 1e9;
    for (CandidateScore &c : candidates) {
        const double distNorm = normalize(c.distance, distMin, distMax);
        const double delaiNorm = normalize(double(c.f.delai), double(delaiMin), double(delaiMax));
        const double fiabPenalty = 1.0 - normalize(double(c.f.fiabilite), double(fiabMin), double(fiabMax));
        const double chargeNorm = normalize(double(c.f.nb_commandes), double(cmdMin), double(cmdMax));
        const double routePenalty = (c.f.apiDurationMin >= 0.0) ? qMin(1.0, c.f.apiDurationMin / 180.0) : 0.0;

        // Score intelligent: équilibre coût logistique, rapidité, qualité et risque de surcharge.
        c.score = (distNorm * 0.30) + (delaiNorm * 0.25) + (fiabPenalty * 0.22) + (chargeNorm * 0.13)
                  + (routePenalty * 0.10);

        // Pénalités métier (risques).
        if (c.f.delai > 10)
            c.score += 0.08;
        if (c.f.nb_commandes > 50)
            c.score += 0.07;
        if (c.f.fiabilite < 60)
            c.score += 0.10;

        if (c.score < bestScore) {
            bestScore = c.score;
            meilleur = c.f;
        }
    }

    if (m_fournisseurPlusProcheLabel)
        m_fournisseurPlusProcheLabel->setText(
            QStringLiteral("Plus proche : %1 (%2 km)").arg(plusProche.nom).arg(minDistance, 0, 'f', 2));
    if (m_fournisseurPlusRapideLabel)
        m_fournisseurPlusRapideLabel->setText(
            QStringLiteral("Plus rapide : %1 (%2 jours)").arg(plusRapide.nom).arg(plusRapide.delai));
    if (m_fournisseurPlusFiableLabel)
        m_fournisseurPlusFiableLabel->setText(
            QStringLiteral("Plus fiable : %1 (%2%)").arg(plusFiable.nom).arg(plusFiable.fiabilite));
    if (m_fournisseurMeilleurLabel)
        m_fournisseurMeilleurLabel->setText(
            QStringLiteral("Meilleur fournisseur : %1 (score %2)")
                .arg(meilleur.nom)
                .arg(bestScore, 0, 'f', 3));

    QStringList alerts;
    for (const Fournisseur &f : list) {
        if (f.nb_commandes > 50)
            alerts << QStringLiteral("%1: ⚠ surcharge").arg(f.nom);
        if (f.delai > 10)
            alerts << QStringLiteral("%1: ⚠ livraison lente").arg(f.nom);
    }
    alerts.removeDuplicates();
    if (m_fournisseurAlertesLabel) {
        QString recommendation;
        if (meilleur.delai <= 3 && meilleur.fiabilite >= 75)
            recommendation = QStringLiteral("Decision: mode urgent OK.");
        else if (meilleur.fiabilite >= 85)
            recommendation = QStringLiteral("Decision: mode qualite prioritaire.");
        else if (meilleur.nb_commandes > 50)
            recommendation = QStringLiteral("Decision: attention surcharge, prevoir plan B.");
        else
            recommendation = QStringLiteral("Decision: compromis equilibre.");

        QString baseText = alerts.isEmpty()
                               ? QStringLiteral("Alertes : aucune | %1").arg(recommendation)
                               : QStringLiteral("Alertes : %1 | %2").arg(alerts.join(QStringLiteral(" | ")), recommendation);
        if (!m_fournisseurSmartApiStatusMessage.isEmpty())
            baseText += QStringLiteral(" | %1").arg(m_fournisseurSmartApiStatusMessage);
        m_fournisseurAlertesLabel->setText(baseText);
    }
}

void MainWindow::renderSmartMapFournisseurs(const QVector<Fournisseur> &list)
{
    if (!m_fournisseurSmartMapCanvas)
        return;
    renderSmartMapOnCanvas(m_fournisseurSmartMapCanvas, list, &m_fournisseurSmartPointInfos);
}

void MainWindow::renderSmartMapOnCanvas(QLabel *canvas, const QVector<Fournisseur> &list, QVector<SmartMapPointInfo> *pointInfos)
{
    if (!canvas)
        return;
    if (pointInfos)
        pointInfos->clear();

    const int w = qMax(220, canvas->width());
    const int h = qMax(160, canvas->height());
    const QRect targetRect(0, 0, w, h);

    QPixmap baseMap;
    const QStringList imageCandidates = {
        QStringLiteral(":/images/tunisie.png"),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("tunisie.png")),
        QDir::current().filePath(QStringLiteral("tunisie.png")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("Resources/tunisie.png"))
    };
    for (const QString &path : imageCandidates) {
        QPixmap test(path);
        if (!test.isNull()) {
            baseMap = test;
            break;
        }
    }

    QPixmap rendered(w, h);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (!baseMap.isNull()) {
        const QPixmap scaledMap = baseMap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint topLeft((w - scaledMap.width()) / 2, (h - scaledMap.height()) / 2);
        painter.drawPixmap(topLeft, scaledMap);
    } else {
        painter.fillRect(targetRect, QColor(QStringLiteral("#f8fafc")));
        painter.setPen(QPen(QColor(QStringLiteral("#94a3b8"))));
        painter.drawText(targetRect.adjusted(10, 10, -10, -10),
                         Qt::AlignTop | Qt::AlignLeft,
                         QStringLiteral("Image tunisie.png introuvable"));
    }
    painter.fillRect(targetRect, QColor(255, 255, 255, 48));
    const auto drawPoint = [&](double lat, double lon, const QColor &color, int radius, const QString &tooltip) {
        const QPointF pt = smartMapCoordToPixel(lat, lon, w, h);
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(color);
        painter.drawEllipse(pt, radius, radius);
        if (pointInfos) {
            SmartMapPointInfo info;
            info.center = pt.toPoint();
            info.radius = radius + 3;
            info.tooltip = tooltip;
            pointInfos->push_back(info);
        }
    };

    const double latMagasin = 36.8065;
    const double lonMagasin = 10.1815;
    drawPoint(latMagasin,
              lonMagasin,
              QColor(QStringLiteral("#111827")),
              5,
              QStringLiteral("Magasin\nLat: %1\nLon: %2").arg(latMagasin, 0, 'f', 4).arg(lonMagasin, 0, 'f', 4));

    for (const Fournisseur &f : list) {
        QColor pointColor = QColor(QStringLiteral("#ef4444"));
        if (f.fiabilite >= 80)
            pointColor = QColor(QStringLiteral("#22c55e"));
        else if (f.fiabilite >= 60)
            pointColor = QColor(QStringLiteral("#f59e0b"));

        const double dist = (f.apiDistanceKm >= 0.0)
                                ? f.apiDistanceKm
                                : calculDistance(latMagasin, lonMagasin, f.latitude, f.longitude);
        const QString routeInfo = (f.apiDurationMin >= 0.0)
                                      ? QStringLiteral("\nTrajet API: %1 min").arg(f.apiDurationMin, 0, 'f', 1)
                                      : QString();
        const QString tip = QStringLiteral("%1\nDistance: %2 km\nDelai: %3 jours\nFiabilite: %4%\nNb commandes: %5%6")
                                .arg(f.nom)
                                .arg(dist, 0, 'f', 2)
                                .arg(f.delai)
                                .arg(f.fiabilite)
                                .arg(f.nb_commandes)
                                .arg(routeInfo);
        drawPoint(f.latitude, f.longitude, pointColor, 5, tip);
    }
    painter.end();
    canvas->setPixmap(rendered);
}

void MainWindow::refreshSmartMapFournisseurs()
{
    if (!m_fournisseurSmartFilter || !m_fournisseurSmartMapCanvas || !db.isOpen())
        return;

    setupSmartMapFournisseursUi();

    const QString nomCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("RAISON_SOCIALE"), QStringLiteral("NOM")},
                                              QStringLiteral("NOM"));
    const QString latCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LATITUDE"), QStringLiteral("LAT"), QStringLiteral("GPS_LAT")});
    const QString lonCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LONGITUDE"), QStringLiteral("LON"), QStringLiteral("LNG"), QStringLiteral("GPS_LON")});
    const QString delaiCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                                {QStringLiteral("SLA_JOURS"), QStringLiteral("SLA"), QStringLiteral("DELAI")});
    const QString fiabCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                               {QStringLiteral("FIABILITE"), QStringLiteral("SCORE_FIABILITE")});
    const QString nbCmdCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                                {QStringLiteral("NB_COMMANDES"), QStringLiteral("COMMANDES")});
    const QString matiereCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                                  {QStringLiteral("MATIERE"), QStringLiteral("MATIERE_PREMIERE"),
                                                   QStringLiteral("TYPE_MATIERE"), QStringLiteral("CATEGORIE"),
                                                   QStringLiteral("ZONE_GEO"), QStringLiteral("ZONE")});

    if (latCol.isEmpty() || lonCol.isEmpty()) {
        analyserFournisseurs({});
        renderSmartMapFournisseurs({});
        if (m_fournisseurMeilleurLabel) {
            m_fournisseurMeilleurLabel->setText(
                QStringLiteral("Meilleur fournisseur : coordonnees latitude/longitude indisponibles"));
        }
        return;
    }

    const QString matiereExpr = matiereCol.isEmpty()
                                    ? QStringLiteral("'Tous'")
                                    : QStringLiteral("NVL(TRIM(TO_CHAR(%1)),'Tous')").arg(matiereCol);
    const QString delaiExpr = delaiCol.isEmpty() ? QStringLiteral("999") : QStringLiteral("NVL(%1,999)").arg(delaiCol);
    const QString fiabExpr = fiabCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1,0)").arg(fiabCol);
    const QString nbCmdExpr = nbCmdCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1,0)").arg(nbCmdCol);
    QSqlQuery q(db);
    const QString sql = QStringLiteral(
        "SELECT TRIM(TO_CHAR(%1)), %2, %3, %4, %5, %6, %7 "
        "FROM FOURNISSEURS")
                            .arg(nomCol, latCol, lonCol, delaiExpr, fiabExpr, nbCmdExpr, matiereExpr);
    if (!q.exec(sql)) {
        analyserFournisseurs({});
        renderSmartMapFournisseurs({});
        return;
    }

    struct Row { Fournisseur f; QString matiere; };
    QVector<Row> rows;
    QStringList matieres;
    bool hasMissingCoords = false;
    while (q.next()) {
        Row r;
        r.f.nom = q.value(0).toString().trimmed();
        if (r.f.nom.isEmpty())
            r.f.nom = QStringLiteral("Fournisseur");
        const QVariant latV = q.value(1);
        const QVariant lonV = q.value(2);
        if (latV.isNull() || lonV.isNull()) {
            hasMissingCoords = true;
            continue;
        }
        bool latOk = false;
        bool lonOk = false;
        r.f.latitude = latV.toDouble(&latOk);
        r.f.longitude = lonV.toDouble(&lonOk);
        if (!latOk || !lonOk) {
            hasMissingCoords = true;
            continue;
        }
        r.f.delai = qMax(0, q.value(3).toInt());
        r.f.fiabilite = qBound(0, q.value(4).toInt(), 100);
        r.f.nb_commandes = qMax(0, q.value(5).toInt());
        r.matiere = q.value(6).toString().trimmed();
        if (r.matiere.isEmpty())
            r.matiere = QStringLiteral("Autre");
        rows.push_back(r);
        if (!matieres.contains(r.matiere))
            matieres << r.matiere;
    }

    m_fournisseurSmartFilterUpdating = true;
    const QString wanted = m_fournisseurSmartFilterSelection.isEmpty()
                               ? m_fournisseurSmartFilter->currentText()
                               : m_fournisseurSmartFilterSelection;
    m_fournisseurSmartFilter->clear();
    m_fournisseurSmartFilter->addItem(QStringLiteral("Toutes matieres"));
    std::sort(matieres.begin(), matieres.end(), [](const QString &a, const QString &b) {
        return a.localeAwareCompare(b) < 0;
    });
    for (const QString &m : matieres)
        m_fournisseurSmartFilter->addItem(m);
    int idx = m_fournisseurSmartFilter->findText(wanted);
    if (idx < 0)
        idx = 0;
    m_fournisseurSmartFilter->setCurrentIndex(idx);
    m_fournisseurSmartFilterUpdating = false;
    m_fournisseurSmartFilterSelection = m_fournisseurSmartFilter->currentText();
    m_fournisseurSmartMissingCoordsMessage = hasMissingCoords
                                                 ? QStringLiteral("Coordonnées manquantes pour certains fournisseurs")
                                                 : QString();

    QVector<Fournisseur> filtered;
    for (const Row &r : rows) {
        if (m_fournisseurSmartFilterSelection != QStringLiteral("Toutes matieres")
            && r.matiere != m_fournisseurSmartFilterSelection) {
            continue;
        }
        filtered.push_back(r.f);
    }

    applyRouteApiMetrics(filtered, &m_fournisseurSmartApiStatusMessage);
    m_fournisseurSmartMapPoints = filtered;
    analyserFournisseurs(filtered);
    if (!m_fournisseurSmartMissingCoordsMessage.isEmpty() && m_fournisseurAlertesLabel) {
        const QString prev = m_fournisseurAlertesLabel->text();
        if (prev.contains(QStringLiteral("Alertes"))) {
            m_fournisseurAlertesLabel->setText(prev + QStringLiteral(" | ") + m_fournisseurSmartMissingCoordsMessage);
        }
    }
    renderSmartMapFournisseurs(filtered);
}

void MainWindow::applyRouteApiMetrics(QVector<Fournisseur> &list, QString *statusMessage)
{
    if (statusMessage)
        statusMessage->clear();
    if (list.isEmpty())
        return;
    if (!m_networkAccessManager) {
        if (statusMessage)
            *statusMessage = QStringLiteral("API inactive (network manager indisponible)");
        return;
    }

    QString apiKey = QString::fromUtf8(qgetenv("SMARTMAP_API_KEY")).trimmed();
    if (apiKey.isEmpty())
        apiKey = QString::fromUtf8(qgetenv("ORS_API_KEY")).trimmed();
    if (apiKey.isEmpty()) {
        if (statusMessage)
            *statusMessage = QStringLiteral("API inactive (SMARTMAP_API_KEY absente)");
        return;
    }

    const double latMagasin = 36.8065;
    const double lonMagasin = 10.1815;
    QJsonArray locations;
    locations.append(QJsonArray{lonMagasin, latMagasin});
    QJsonArray destinations;
    for (int i = 0; i < list.size(); ++i) {
        locations.append(QJsonArray{list[i].longitude, list[i].latitude});
        destinations.append(i + 1);
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("locations"), locations);
    payload.insert(QStringLiteral("sources"), QJsonArray{0});
    payload.insert(QStringLiteral("destinations"), destinations);
    payload.insert(QStringLiteral("metrics"), QJsonArray{QStringLiteral("distance"), QStringLiteral("duration")});
    payload.insert(QStringLiteral("units"), QStringLiteral("km"));

    QNetworkRequest req(QUrl(QStringLiteral("https://api.openrouteservice.org/v2/matrix/driving-car")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", apiKey.toUtf8());
    QNetworkReply *reply = m_networkAccessManager->post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        if (statusMessage)
            *statusMessage = QStringLiteral("API ORS indisponible (fallback local)");
        reply->deleteLater();
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    if (!doc.isObject()) {
        if (statusMessage)
            *statusMessage = QStringLiteral("API ORS reponse invalide (fallback local)");
        return;
    }
    const QJsonObject obj = doc.object();
    const QJsonArray distMatrix = obj.value(QStringLiteral("distances")).toArray();
    const QJsonArray durMatrix = obj.value(QStringLiteral("durations")).toArray();
    if (distMatrix.isEmpty() || durMatrix.isEmpty()) {
        if (statusMessage)
            *statusMessage = QStringLiteral("API ORS incomplete (fallback local)");
        return;
    }
    const QJsonArray distRow = distMatrix.first().toArray();
    const QJsonArray durRow = durMatrix.first().toArray();
    const int count = qMin(list.size(), qMin(distRow.size(), durRow.size()));
    int applied = 0;
    for (int i = 0; i < count; ++i) {
        const QJsonValue d = distRow.at(i);
        const QJsonValue t = durRow.at(i);
        if (!d.isDouble() || !t.isDouble())
            continue;
        list[i].apiDistanceKm = d.toDouble();
        list[i].apiDurationMin = t.toDouble() / 60.0; // ORS renvoie en secondes.
        ++applied;
    }
    if (statusMessage)
        *statusMessage = QStringLiteral("API ORS active (%1 trajets routes)").arg(applied);
}

void MainWindow::geocodeAdresse(QString adresse, int fournisseurId)
{
    adresse = adresse.trimmed();
    if (adresse.isEmpty() || fournisseurId <= 0 || !m_networkAccessManager) {
        qDebug() << "[Geocode] Parametres invalides" << fournisseurId << adresse;
        return;
    }

    QUrl url(QStringLiteral("https://nominatim.openstreetmap.org/search"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), adresse + QStringLiteral(", Tunisia"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("LeatherERP/1.0 (SmartMap Geocoding)"));
    req.setRawHeader("Accept-Language", "fr,en;q=0.9");
    qDebug() << "[Geocode] Request:" << url.toString();

    QNetworkReply *reply = m_networkAccessManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, fournisseurId, adresse]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[Geocode] Erreur reseau fournisseur" << fournisseurId << ":" << reply->errorString();
            QTimer::singleShot(1200, this, [this]() { geocodePendingFournisseurs(); });
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isArray() || doc.array().isEmpty()) {
            qDebug() << "[Geocode] Adresse introuvable pour fournisseur" << fournisseurId << ":" << adresse;
            QTimer::singleShot(1200, this, [this]() { geocodePendingFournisseurs(); });
            return;
        }
        const QJsonObject first = doc.array().first().toObject();
        bool okLat = false;
        bool okLon = false;
        const double lat = first.value(QStringLiteral("lat")).toString().toDouble(&okLat);
        const double lon = first.value(QStringLiteral("lon")).toString().toDouble(&okLon);
        if (!okLat || !okLon) {
            qDebug() << "[Geocode] Coordonnees invalides pour fournisseur" << fournisseurId;
            QTimer::singleShot(1200, this, [this]() { geocodePendingFournisseurs(); });
            return;
        }

        const QString codeCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                                   {QStringLiteral("ID"), QStringLiteral("CODE"), QStringLiteral("CIN")},
                                                   QStringLiteral("ID"));
        QSqlQuery q(db);
        q.prepare(QStringLiteral("UPDATE FOURNISSEURS SET LATITUDE=:la, LONGITUDE=:lo WHERE TO_CHAR(%1)=:id").arg(codeCol));
        q.bindValue(QStringLiteral(":la"), lat);
        q.bindValue(QStringLiteral(":lo"), lon);
        q.bindValue(QStringLiteral(":id"), QString::number(fournisseurId));
        if (!q.exec()) {
            qDebug() << "[Geocode] Echec update SQL fournisseur" << fournisseurId << ":" << q.lastError().text();
        } else {
            qDebug() << "[Geocode] OK fournisseur" << fournisseurId << "->" << lat << lon;
            refreshSmartMapFournisseurs();
            injectFournisseursLeafletMarkers();
        }
        QTimer::singleShot(1200, this, [this]() { geocodePendingFournisseurs(); });
    });
}

void MainWindow::geocodePendingFournisseurs()
{
    if (!db.isOpen())
        return;

    const QString codeCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                               {QStringLiteral("ID"), QStringLiteral("CODE"), QStringLiteral("CIN")},
                                               QStringLiteral("ID"));
    const QString addrCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                               {QStringLiteral("ADRESSE"), QStringLiteral("ZONE_GEO"), QStringLiteral("ZONE"), QStringLiteral("PAYS")});
    const QString latCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LATITUDE"), QStringLiteral("LAT"), QStringLiteral("GPS_LAT")},
                                              QStringLiteral("LATITUDE"));
    const QString lonCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LONGITUDE"), QStringLiteral("LON"), QStringLiteral("LNG"), QStringLiteral("GPS_LON")},
                                              QStringLiteral("LONGITUDE"));
    if (addrCol.isEmpty() || codeCol.isEmpty() || latCol.isEmpty() || lonCol.isEmpty())
        return;

    QSqlQuery q(db);
    const QString sql = QStringLiteral(
        "SELECT TO_CHAR(%1), TRIM(TO_CHAR(%2)) FROM FOURNISSEURS "
        "WHERE TRIM(TO_CHAR(%2)) IS NOT NULL AND (%3 IS NULL OR %4 IS NULL)")
                            .arg(codeCol, addrCol, latCol, lonCol);
    if (!q.exec(sql) || !q.next())
        return;

    bool okId = false;
    const int fournisseurId = q.value(0).toString().trimmed().toInt(&okId);
    const QString adresse = q.value(1).toString().trimmed();
    if (!okId || adresse.isEmpty()) {
        qDebug() << "[Geocode] Ligne ignorée (id/adresse invalides)";
        return;
    }
    geocodeAdresse(adresse, fournisseurId);
}

void MainWindow::loadFournisseursLeafletMap()
{
    // Conserve la compatibilite avec le code existant.
    openMap();
}

QList<QVariantMap> MainWindow::getFournisseurs() const
{
    QList<QVariantMap> fournisseurs;
    if (!db.isOpen()) {
        qDebug() << "[Map] Base de donnees fermee: impossible de charger les fournisseurs.";
        return fournisseurs;
    }

    const QString nomCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("NOM"), QStringLiteral("RAISON_SOCIALE"), QStringLiteral("RAISON")},
                                              QStringLiteral("NOM"));
    const QString latCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LATITUDE"), QStringLiteral("LAT"), QStringLiteral("GPS_LAT")},
                                              QStringLiteral("LATITUDE"));
    const QString lonCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("LONGITUDE"), QStringLiteral("LON"), QStringLiteral("LNG"), QStringLiteral("GPS_LON")},
                                              QStringLiteral("LONGITUDE"));

    if (latCol.isEmpty() || lonCol.isEmpty()) {
        qDebug() << "[Map] Colonnes latitude/longitude introuvables.";
        return fournisseurs;
    }

    QSqlQuery q(db);
    const QString sql = QStringLiteral("SELECT TO_CHAR(%1), TO_CHAR(%2), TO_CHAR(%3) FROM FOURNISSEURS")
                            .arg(nomCol, latCol, lonCol);
    if (!q.exec(sql)) {
        qDebug() << "[Map] Echec requete fournisseurs:" << q.lastError().text();
        return fournisseurs;
    }

    while (q.next()) {
        const QString nom = q.value(0).toString().trimmed();
        const QString rawLat = q.value(1).toString().trimmed();
        const QString rawLon = q.value(2).toString().trimmed();

        // Accepte les formats "36.123" et "36,123".
        bool okLat = false;
        bool okLon = false;
        double lat = rawLat.toDouble(&okLat);
        double lon = rawLon.toDouble(&okLon);
        if (!okLat) {
            QString normalizedLat = rawLat;
            lat = normalizedLat.replace(',', '.').toDouble(&okLat);
        }
        if (!okLon) {
            QString normalizedLon = rawLon;
            lon = normalizedLon.replace(',', '.').toDouble(&okLon);
        }

        // Ignore les lignes sans coordonnees valides.
        if (!okLat || !okLon) {
            qDebug() << "[Map] Fournisseur ignore (coordonnees invalides):"
                     << nom << "| lat brute =" << rawLat << "| lon brute =" << rawLon;
            continue;
        }

        QVariantMap row;
        row.insert(QStringLiteral("nom"), nom);
        row.insert(QStringLiteral("latitude"), lat);
        row.insert(QStringLiteral("longitude"), lon);
        fournisseurs.append(row);
    }

    qDebug() << "[Map] Fournisseurs valides charges:" << fournisseurs.size();
    return fournisseurs;
}

QList<QVariantMap> MainWindow::getFournisseursNomAdresse() const
{
    QList<QVariantMap> rows;
    if (!db.isOpen()) {
        qDebug() << "[Map] Base fermee : getFournisseursNomAdresse ignore.";
        return rows;
    }

    const QString nomCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                              {QStringLiteral("NOM"), QStringLiteral("RAISON_SOCIALE"), QStringLiteral("RAISON")},
                                              QStringLiteral("NOM"));
    const QString addrCol = pickExistingColumn(QStringLiteral("FOURNISSEURS"),
                                               {QStringLiteral("ADRESSE"), QStringLiteral("ZONE_GEO"), QStringLiteral("ZONE"),
                                                QStringLiteral("PAYS")},
                                               QStringLiteral("ADRESSE"));
    if (nomCol.isEmpty() || addrCol.isEmpty()) {
        qDebug() << "[Map] Colonnes nom/adresse introuvables pour la carte.";
        return rows;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral("SELECT TO_CHAR(%1), TO_CHAR(%2) FROM FOURNISSEURS").arg(nomCol, addrCol);
    if (!query.exec(sql)) {
        qDebug() << "[Map] Echec SELECT nom,adresse:" << query.lastError().text();
        return rows;
    }

    while (query.next()) {
        QVariantMap row;
        row.insert(QStringLiteral("nom"), query.value(0).toString().trimmed());
        row.insert(QStringLiteral("adresse"), query.value(1).toString().trimmed());
        rows.append(row);
    }
    qDebug() << "[Map] Lignes fournisseurs (nom+adresse):" << rows.size();
    return rows;
}

void MainWindow::openMap()
{
#ifdef LEATHER_HAVE_WEBENGINE
    const QList<QVariantMap> nomAddrRows = getFournisseursNomAdresse();
    bool hasAdresse = false;
    for (const QVariantMap &r : nomAddrRows) {
        if (!r.value(QStringLiteral("adresse")).toString().trimmed().isEmpty()) {
            hasAdresse = true;
            break;
        }
    }
    if (!hasAdresse) {
        qDebug() << "[Map] Aucune adresse fournisseur pour geocodage Nominatim.";
        QMessageBox::information(this,
                                 QStringLiteral("Carte fournisseurs"),
                                 QStringLiteral("Aucune adresse fournisseur renseignee pour afficher la carte."));
        return;
    }
#else
    const QList<QVariantMap> fournisseurs = getFournisseurs();

    if (fournisseurs.isEmpty()) {
        qDebug() << "[Map] Aucun fournisseur avec coordonnees valides.";
        QMessageBox::information(this,
                                 QStringLiteral("Carte fournisseurs"),
                                 QStringLiteral("Aucun fournisseur avec latitude/longitude valide."));
        return;
    }
#endif

#ifdef LEATHER_HAVE_WEBENGINE
    // Carte integree type « capture » : tuiles OpenStreetMap + marqueurs rouges (Leaflet dans QWebEngineView).
    if (!m_fournisseurMapDialog) {
        m_fournisseurMapDialog = new QDialog(this);
        m_fournisseurMapDialog->setAttribute(Qt::WA_DeleteOnClose, false);
        m_fournisseurMapDialog->setWindowTitle(QStringLiteral("Carte fournisseurs — OpenStreetMap"));
        m_fournisseurMapDialog->resize(1100, 760);
        auto *lay = new QVBoxLayout(m_fournisseurMapDialog);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
        m_fournisseurMapView = new QWebEngineView(m_fournisseurMapDialog);
        // Nominatim exige un User-Agent identifiable ; en JS fetch() on ne peut pas forcer User-Agent (navigateur).
        m_fournisseurMapView->page()->profile()->setHttpUserAgent(
            QStringLiteral("RoyalLeatherHouse/1.0 (Qt Fournisseurs map; contact: app@royalleatherhouse.local)"));
        lay->addWidget(m_fournisseurMapView, 1);
        connect(m_fournisseurMapView, &QWebEngineView::urlChanged, this, [](const QUrl &u) {
            qDebug() << "[Map][WebEngine] URL chargée:" << u;
        });
        connect(m_fournisseurMapView, &QWebEngineView::loadFinished, this, [this](bool ok) {
            qDebug() << "[Map][WebEngine] loadFinished ok =" << ok;
            m_fournisseurMapHtmlLoaded = ok;
            if (ok) {
                injectFournisseursLeafletMarkers();
            } else {
                qDebug() << "[Map][WebEngine] Échec chargement qrc:/map.html (écran blanc possible).";
            }
        });
        connect(m_fournisseurMapView->page(), &QWebEnginePage::renderProcessTerminated, this,
                [](QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
                    qDebug() << "[Map][WebEngine] renderProcessTerminated status =" << status
                             << "exitCode =" << exitCode;
                });
        connect(m_fournisseurMapView->page(), &QWebEnginePage::loadFinished, this, [](bool ok) {
            qDebug() << "[Map][WebEngine][Page] loadFinished ok =" << ok;
        });
    }

    if (!m_fournisseurMapHtmlLoaded && m_fournisseurMapView)
        m_fournisseurMapView->setUrl(QUrl(QStringLiteral("qrc:/map.html")));
    else
        injectFournisseursLeafletMarkers();

    m_fournisseurMapDialog->show();
    m_fournisseurMapDialog->raise();
    m_fournisseurMapDialog->activateWindow();
#else
    // Sans WebEngine : ouverture Google Maps dans le navigateur (meme logique qu'avant).
    auto coordText = [](const QVariantMap &f) -> QString {
        return QString::number(f.value(QStringLiteral("latitude")).toDouble(), 'f', 6)
               + QLatin1Char(',')
               + QString::number(f.value(QStringLiteral("longitude")).toDouble(), 'f', 6);
    };

    constexpr int kMaxPoints = 24;
    QStringList coords;
    coords.reserve(qMin(fournisseurs.size(), kMaxPoints));
    for (int i = 0; i < fournisseurs.size() && coords.size() < kMaxPoints; ++i)
        coords.append(coordText(fournisseurs.at(i)));

    QUrl url;
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("api"), QStringLiteral("1"));

    if (coords.size() == 1) {
        url = QUrl(QStringLiteral("https://www.google.com/maps/search/"));
        query.addQueryItem(QStringLiteral("query"), coords.first());
    } else {
        url = QUrl(QStringLiteral("https://www.google.com/maps/dir/"));
        query.addQueryItem(QStringLiteral("destination"), coords.first());
        const QStringList rest = coords.mid(1);
        if (!rest.isEmpty())
            query.addQueryItem(QStringLiteral("waypoints"), rest.join(QLatin1Char('|')));
    }
    url.setQuery(query);

    qDebug() << "[Map] WebEngine absent : ouverture Google Maps, points:" << coords.size();
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this,
                             QStringLiteral("Carte fournisseurs"),
                             QStringLiteral("Impossible d'ouvrir Google Maps.\n"
                                            "Activez le module Qt WebEngine pour une carte integree (OpenStreetMap)."));
    }
#endif
}

void MainWindow::injectFournisseursLeafletMarkers()
{
#ifdef LEATHER_HAVE_WEBENGINE
    if (!m_fournisseurMapView || !m_fournisseurMapHtmlLoaded || !db.isOpen())
        return;

    // Geocodage cote navigateur (Nominatim) a partir de nom + adresse (voir map.html).
    const QList<QVariantMap> rows = getFournisseursNomAdresse();
    QVariantList variantList;
    for (const QVariantMap &item : rows)
        variantList.append(item);
    const QString json = QString::fromUtf8(QJsonDocument::fromVariant(variantList).toJson(QJsonDocument::Compact));
    const QString js = QStringLiteral(
                           "if (typeof geocodeAndAddMarkersFromRows === 'function') {"
                           "  geocodeAndAddMarkersFromRows(__FOURNISSEURS_JSON__);"
                           "} else {"
                           "  console.error('geocodeAndAddMarkersFromRows absent dans map.html');"
                           "}")
                           .replace(QStringLiteral("__FOURNISSEURS_JSON__"), json);
    m_fournisseurMapView->page()->runJavaScript(js);
#endif
}

QPointF MainWindow::smartMapCoordToPixel(double latitude, double longitude, int width, int height) const
{
    const double minLat = 30.0;
    const double maxLat = 37.5;
    const double minLon = 7.0;
    const double maxLon = 12.0;
    const double x = (longitude - minLon) / (maxLon - minLon) * double(width);
    const double y = double(height) - (latitude - minLat) / (maxLat - minLat) * double(height);
    return QPointF(qBound(0.0, x, double(width - 1)), qBound(0.0, y, double(height - 1)));
}

void MainWindow::showSmartMapFournisseursDialog()
{
    refreshSmartMapFournisseurs();

    auto *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("Smart Map fournisseurs"));
    dlg->resize(1180, 760);

    auto *root = new QVBoxLayout(dlg);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto *head = new QLabel(QStringLiteral("Interface Smart Map - Fournisseurs"), dlg);
    head->setStyleSheet(QStringLiteral("font-size:18px;font-weight:700;color:#5a3a22;"));
    root->addWidget(head);

    auto *filterRow = new QHBoxLayout();
    auto *filterLab = new QLabel(QStringLiteral("Matiere"), dlg);
    auto *filter = new QComboBox(dlg);
    filter->setMinimumHeight(30);
    if (m_fournisseurSmartFilter) {
        for (int i = 0; i < m_fournisseurSmartFilter->count(); ++i)
            filter->addItem(m_fournisseurSmartFilter->itemText(i));
        filter->setCurrentText(m_fournisseurSmartFilter->currentText());
    }
    filterRow->addWidget(filterLab);
    filterRow->addWidget(filter, 1);
    root->addLayout(filterRow);

    auto *closest = new QLabel(dlg);
    auto *fastest = new QLabel(dlg);
    auto *reliable = new QLabel(dlg);
    auto *best = new QLabel(dlg);
    auto *alerts = new QLabel(dlg);
    for (QLabel *lab : {closest, fastest, reliable, best, alerts}) {
        lab->setWordWrap(true);
        lab->setStyleSheet(QStringLiteral("font-size:12px;"));
        root->addWidget(lab);
    }

    QWidget *mapWidget = nullptr;
#ifdef LEATHER_HAVE_LOCATION_MAP
    auto *map = new QQuickWidget(dlg);
    map->setResizeMode(QQuickWidget::SizeRootObjectToView);
    map->setMinimumHeight(470);
    map->setMinimumWidth(760);
    map->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    map->setStyleSheet(QStringLiteral("QQuickWidget { background:#f8fafc; border:1px solid #d5dbe5; border-radius:8px; }"));
    map->setSource(QUrl(QStringLiteral("qrc:/qml/SmartMapView.qml")));
    mapWidget = map;
#else
    auto *map = new QLabel(dlg);
    map->setMinimumHeight(470);
    map->setMinimumWidth(760);
    map->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    map->setAlignment(Qt::AlignCenter);
    map->setStyleSheet(QStringLiteral("QLabel { background:#f8fafc; border:1px solid #d5dbe5; border-radius:8px; }"));
    m_fournisseurSmartMapDialogCanvas = map;
    m_fournisseurSmartMapDialogCanvas->installEventFilter(this);
    mapWidget = map;
#endif
    root->addWidget(mapWidget, 1);

    const auto syncFromMainSmartMap = [this, closest, fastest, reliable, best, alerts, mapWidget]() {
        if (m_fournisseurPlusProcheLabel) closest->setText(m_fournisseurPlusProcheLabel->text());
        if (m_fournisseurPlusRapideLabel) fastest->setText(m_fournisseurPlusRapideLabel->text());
        if (m_fournisseurPlusFiableLabel) reliable->setText(m_fournisseurPlusFiableLabel->text());
        if (m_fournisseurMeilleurLabel) best->setText(m_fournisseurMeilleurLabel->text());
        if (m_fournisseurAlertesLabel) alerts->setText(m_fournisseurAlertesLabel->text());
#ifdef LEATHER_HAVE_LOCATION_MAP
        if (auto *quickMap = qobject_cast<QQuickWidget *>(mapWidget)) {
            QVariantList suppliers;
            const double latMagasin = 36.8065;
            const double lonMagasin = 10.1815;
            for (const Fournisseur &f : m_fournisseurSmartMapPoints) {
                const double dist = (f.apiDistanceKm >= 0.0)
                                        ? f.apiDistanceKm
                                        : calculDistance(latMagasin, lonMagasin, f.latitude, f.longitude);
                QString color = QStringLiteral("#ef4444");
                if (f.fiabilite >= 80)
                    color = QStringLiteral("#22c55e");
                else if (f.fiabilite >= 60)
                    color = QStringLiteral("#f59e0b");

                QVariantMap item;
                item.insert(QStringLiteral("name"), f.nom);
                item.insert(QStringLiteral("latitude"), f.latitude);
                item.insert(QStringLiteral("longitude"), f.longitude);
                item.insert(QStringLiteral("color"), color);
                item.insert(QStringLiteral("tooltip"),
                            QStringLiteral("%1\nDistance: %2 km\nDelai: %3 jours\nFiabilite: %4%\nNb commandes: %5")
                                .arg(f.nom)
                                .arg(dist, 0, 'f', 2)
                                .arg(f.delai)
                                .arg(f.fiabilite)
                                .arg(f.nb_commandes));
                suppliers.push_back(item);
            }
            auto *rootObj = quickMap->rootObject();
            if (rootObj) {
                rootObj->setProperty("suppliers", suppliers);
                rootObj->setProperty("storeLat", 36.8065);
                rootObj->setProperty("storeLon", 10.1815);
            }
        }
#else
        QVector<SmartMapPointInfo> tmpInfos;
        if (auto *labelMap = qobject_cast<QLabel *>(mapWidget))
            renderSmartMapOnCanvas(labelMap, m_fournisseurSmartMapPoints, &tmpInfos);
#endif
    };

    connect(filter, &QComboBox::currentTextChanged, dlg, [this, filter, syncFromMainSmartMap](const QString &text) {
        if (!m_fournisseurSmartFilter)
            return;
        m_fournisseurSmartFilter->setCurrentText(text);
        refreshSmartMapFournisseurs();
        syncFromMainSmartMap();
    });

    syncFromMainSmartMap();
    QTimer::singleShot(0, dlg, [syncFromMainSmartMap]() { syncFromMainSmartMap(); });
    connect(dlg, &QDialog::destroyed, this, [this]() { m_fournisseurSmartMapDialogCanvas = nullptr; });
    dlg->show();
}

QString MainWindow::buildFournisseurAiContext() const
{
    QStringList lines;
    lines << QStringLiteral("Table fournisseurs (extrait):");
    if (!ui->employeeTable_2) {
        lines << QStringLiteral("Aucun tableau fournisseur disponible.");
        return lines.join('\n');
    }

    const int rowCount = ui->employeeTable_2->rowCount();
    const int colCount = ui->employeeTable_2->columnCount();
    const int maxRows = qMin(rowCount, 12);
    const int maxCols = qMin(colCount, 6);

    QStringList headers;
    for (int c = 0; c < maxCols; ++c) {
        const QTableWidgetItem *h = ui->employeeTable_2->horizontalHeaderItem(c);
        headers << (h ? h->text().trimmed() : QStringLiteral("Col%1").arg(c + 1));
    }
    if (!headers.isEmpty())
        lines << headers.join(QStringLiteral(" | "));

    for (int r = 0; r < maxRows; ++r) {
        QStringList rowValues;
        for (int c = 0; c < maxCols; ++c) {
            const QTableWidgetItem *it = ui->employeeTable_2->item(r, c);
            rowValues << (it ? it->text().trimmed() : QStringLiteral("-"));
        }
        lines << rowValues.join(QStringLiteral(" | "));
    }
    if (rowCount > maxRows)
        lines << QStringLiteral("... %1 autres fournisseurs non affiches").arg(rowCount - maxRows);

    if (ui->lineEditChat_2) {
        const QString userExtra = ui->lineEditChat_2->text().trimmed();
        if (!userExtra.isEmpty())
            lines << QStringLiteral("Contexte ajoute par utilisateur: %1").arg(userExtra);
    }
    return lines.join('\n');
}

void MainWindow::onFournisseurAiRequested()
{
    if (!m_fournisseurApiService || !ui->chatDisplay_2)
        return;

    const QString context = buildFournisseurAiContext();
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm"));
    ui->chatDisplay_2->append(QStringLiteral("[%1] Lancement de 2 APIs fournisseurs: Performance + Risque...")
                                  .arg(stamp));

    m_fournisseurApiService->requestPerformanceAnalystApi(context);
    m_fournisseurApiService->requestRiskContinuityApi(context);

    if (ui->lineEditChat_2)
        ui->lineEditChat_2->clear();
}

void MainWindow::onFournisseurChatbotRequested()
{
    if (!m_chatbotService || !ui->chatDisplay_2 || !ui->lineEditChat_2)
        return;

    const QString msg = ui->lineEditChat_2->text().trimmed();
    if (msg.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Chatbot"), QStringLiteral("Champ vide."));
        return;
    }

    ui->lineEditChat_2->clear();
    ui->chatDisplay_2->append(QStringLiteral("Vous :\n%1\n").arg(msg));
    m_chatbotSink = ChatbotSink::Fournisseur;
    m_chatbotService->ask(msg, buildFournisseurAiContext(), true);
}

// ------------------- Connexion BD -------------------

bool MainWindow::connectToDatabase(bool showErrorDialog)
{
    // Connexion nommée = connexion par défaut de Qt (utilisée par Client, Produit, CommerceStore).
    const QString connName = QStringLiteral("qt_sql_default_connection");
    if (QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::database(connName);
    } else {
        db = QSqlDatabase::addDatabase(QStringLiteral("QODBC"), connName);
    }

    const QString envDsn = QString::fromUtf8(qgetenv("LEATHER_ODBC_DSN")).trimmed();
    const QString envUser = QString::fromUtf8(qgetenv("LEATHER_DB_USER")).trimmed();
    const QString envPass = QString::fromUtf8(qgetenv("LEATHER_DB_PASSWORD"));
    const QString envConnStr = QString::fromUtf8(qgetenv("LEATHER_ODBC_CONNECT_STRING")).trimmed();

    const QString dsn = envDsn.isEmpty() ? QStringLiteral("projet_cuir") : envDsn;
    const QString user = envUser.isEmpty() ? QStringLiteral("dorra") : envUser;
    const QString pass = envPass.isEmpty() ? QStringLiteral("2012") : envPass;

    db.setUserName(user);
    db.setPassword(pass);

    if (db.isOpen())
        return true;

    QString attemptedDbName;
    auto tryOpen = [&](const QString &dbName) -> bool {
        attemptedDbName = dbName;
        db.setDatabaseName(dbName);
        return db.open();
    };

    bool ok = false;
    if (!envConnStr.isEmpty()) {
        ok = tryOpen(envConnStr);
    } else {
        // Certains pilotes attendent "DSN=nom;", d'autres acceptent directement le nom.
        ok = tryOpen(QStringLiteral("DSN=%1;").arg(dsn));
        if (!ok)
            ok = tryOpen(dsn);
    }

    if (ok) {
        qDebug() << "Connexion Oracle via ODBC réussie";
        qDebug() << "ODBC databaseName utilisé :" << attemptedDbName;
        return true;
    }

    const QString openError = db.lastError().text();
    const QString availableDrivers = QSqlDatabase::drivers().join(", ");
    qDebug() << "Erreur connexion ODBC :" << openError;
    qDebug() << "Drivers Qt disponibles :" << availableDrivers;

    if (showErrorDialog) {
        QString oraHint;
        if (openError.contains(QStringLiteral("12545"), Qt::CaseInsensitive)) {
            oraHint = QStringLiteral(
                "\n\n--- ORA-12545 ---\n"
                "La cible (hôte ou service Oracle) n’existe pas ou n’est pas joignable.\n"
                "Vérifiez dans le DSN ODBC / tnsnames.ora : nom d’hôte, port, SERVICE_NAME ou SID.\n"
                "Testez la même connexion dans « Gestionnaire ODBC Windows » (64 bits).\n"
                "Vous pouvez forcer une chaîne complète avec la variable d’environnement\n"
                "LEATHER_ODBC_CONNECT_STRING (ex. DSN=... ou format Easy Connect si le pilote le permet).\n");
        }
        QMessageBox::critical(
            this,
            "Connexion base de données",
            "Connexion Oracle impossible via ODBC.\n\n"
            "Vérifiez que la source de données ODBC 64 bits existe bien,\n"
            "et que le DSN/utilisateur/mot de passe sont corrects.\n\n"
            "DSN utilise: " + dsn + "\n"
            "Utilisateur: " + user + "\n"
            "DatabaseName ODBC tente: " + attemptedDbName + "\n"
            "Pour override via environnement:\n"
            "- LEATHER_ODBC_DSN\n"
            "- LEATHER_DB_USER\n"
            "- LEATHER_DB_PASSWORD\n"
            "- LEATHER_ODBC_CONNECT_STRING (optionnel)\n\n"
            "Erreur: " + openError + "\n"
            "Drivers Qt: " + availableDrivers + oraHint
        );
    }
    return false;

}

bool MainWindow::ensureDbOpenForProduits()
{
    if (db.isOpen())
        return true;
    return connectToDatabase(false);
}

// ------------------- Destructeur -------------------

MainWindow::~MainWindow()
{
    if (m_appTranslator) {
        qApp->removeTranslator(m_appTranslator);
        delete m_appTranslator;
        m_appTranslator = nullptr;
    }
    delete ui;
}

// ------------------- Sidebar -------------------

void MainWindow::connectSidebar()
{
    connect(ui->btnAccueil, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(0);
        setActiveButton(ui->btnAccueil);
        updateShellChromeVisibility();
    });

    connect(ui->btnDashboard, &QPushButton::clicked, this, [this]() {
        if (ui->contentStack && ui->pageDashboard)
            ui->contentStack->setCurrentWidget(ui->pageDashboard);
        setActiveButton(ui->btnDashboard);
    });

    connect(ui->btnEmployes, &QPushButton::clicked, this, &MainWindow::openEmployesModule);

    connect(ui->btnclients, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(2);
        setActiveButton(ui->btnclients);
    });

    connect(ui->btnFournisseurs, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(3);
        setActiveButton(ui->btnFournisseurs);
    });

    connect(ui->btnMpremieres, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(5);
        setActiveButton(ui->btnMpremieres);
        if (!ensureDbOpenForProduits()) {
            const QString detail = db.lastError().text().trimmed();
            QMessageBox::warning(this, QStringLiteral("Matieres premieres"),
                                  QStringLiteral("Connexion Oracle impossible. Verifiez le DSN ODBC (projet_cuir), "
                                                 "l'utilisateur et le mot de passe.")
                                      + (detail.isEmpty() ? QString() : QStringLiteral("\n\n") + detail));
            return;
        }
        QString merr;
        if (!MatierePremiere::ensureSchema(&merr)) {
            QMessageBox::critical(this, QStringLiteral("Matieres premieres"),
                                  QStringLiteral("Preparation du schema MATIERES_PREMIERES :\n%1").arg(merr));
            return;
        }
        MatierePremiere::seedDemoIfEmpty(&merr);
        refreshMatieresTable();
    });

    connect(ui->btnProduits, &QPushButton::clicked, this, [this]() {
        ui->contentStack->setCurrentIndex(4);
        setActiveButton(ui->btnProduits);
        if (!ensureDbOpenForProduits()) {
            const QString detail = db.lastError().text().trimmed();
            QMessageBox::warning(this, QStringLiteral("Produits"),
                                  QStringLiteral("Connexion Oracle impossible. Verifiez le DSN ODBC (projet_cuir), "
                                                 "l'utilisateur et le mot de passe.")
                                      + (detail.isEmpty() ? QString() : QStringLiteral("\n\n") + detail));
            return;
        }
        QString perr;
        if (!CommerceStore::ensureSchema(&perr)) {
            QMessageBox::critical(this, QStringLiteral("Produits"),
                                  QStringLiteral("Schema commerce (PRODUITS / STOCK) :\n%1").arg(perr));
            return;
        }
        CommerceStore::seedDemoCatalogIfEmpty(&perr);
        refreshProduitsTable();
    });

}

void MainWindow::setActiveButton(QPushButton *active)
{
    // Styles inline : priorité sur #topNavBar du .ui (hover / actif).
    const QString defaultStyle = QStringLiteral(
        "QPushButton {"
        "  background: transparent;"
        "  color: rgb(130, 80, 45);"
        "  text-align: center;"
        "  padding: 8px 14px;"
        "  border: 1px solid rgba(93, 46, 6, 0.35);"
        "  border-radius: 8px;"
        "  font: 600 10pt \"Segoe UI\";"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(200, 90, 60, 0.12);"
        "  border-color: rgba(93, 46, 6, 0.55);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(200, 90, 60, 0.18);"
        "}");

    const QString activeStyle = QStringLiteral(
        "QPushButton {"
        "  background-color: rgba(200, 90, 60, 0.14);"
        "  color: rgb(110, 62, 28);"
        "  text-align: center;"
        "  padding: 8px 14px;"
        "  border: 1px solid rgba(93, 46, 6, 0.55);"
        "  border-radius: 8px;"
        "  font: 700 10pt \"Segoe UI\";"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(200, 90, 60, 0.18);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(200, 90, 60, 0.24);"
        "}");

    const QString logoutStyle = QStringLiteral(
        "QPushButton {"
        "  background: transparent;"
        "  color: rgb(130, 80, 45);"
        "  text-align: center;"
        "  padding: 8px 14px;"
        "  border: 1px solid rgba(93, 46, 6, 0.35);"
        "  border-radius: 8px;"
        "  font: 600 10pt \"Segoe UI\";"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(200, 90, 60, 0.12);"
        "  border-color: rgba(93, 46, 6, 0.55);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(200, 90, 60, 0.18);"
        "}");

    QList<QPushButton *> allBtns = {
        ui->btnAccueil, ui->btnDashboard, ui->btnEmployes, ui->btnFournisseurs,
        ui->btnProduits, ui->btnclients, ui->btnMpremieres
    };

    for (QPushButton *btn : allBtns) {
        if (btn)
            btn->setStyleSheet(btn == active ? activeStyle : defaultStyle);
    }

    if (ui->btnSettings)
        ui->btnSettings->setStyleSheet(logoutStyle);
    if (ui->btnMpremieres_2)
        ui->btnMpremieres_2->setStyleSheet(logoutStyle);
    if (QPushButton *btnGlobalChatbot = this->findChild<QPushButton *>(QStringLiteral("btnGlobalChatbot")))
        btnGlobalChatbot->setStyleSheet(logoutStyle);
    if (QPushButton *btnMaps = this->findChild<QPushButton *>(QStringLiteral("btnMaps")))
        btnMaps->setStyleSheet(logoutStyle);
}

void MainWindow::applyAuthenticationUiState(bool authenticated)
{
    m_isAuthenticated = authenticated;
    const QList<QPushButton *> protectedButtons = {
        ui->btnDashboard, ui->btnEmployes, ui->btnFournisseurs, ui->btnProduits, ui->btnclients, ui->btnMpremieres,
        ui->btnSettings
    };

    for (QPushButton *btn : protectedButtons) {
        if (btn)
            btn->setEnabled(authenticated);
    }

    updateShellChromeVisibility();
}

void MainWindow::updateShellChromeVisibility()
{
    const bool authed = m_isAuthenticated;

    // Barre du haut visible dès qu'un utilisateur est connecté (y compris sur le hub),
    // pour permettre l'accès à Paramètres / Déconnexion sans quitter l'accueil.
    if (ui->topNavBar)
        ui->topNavBar->setVisible(authed);
    if (ui->appHeader)
        ui->appHeader->setVisible(authed);
    if (ui->appFooter)
        ui->appFooter->setVisible(authed);
}

void MainWindow::setupLoginPageChrome()
{
    if (QWidget *loginPage = ui->pageAccueil) {
        loginPage->setStyleSheet(QStringLiteral(
            "#pageAccueil {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "    stop:0 #f8f5ef, stop:0.55 #f3ecdf, stop:1 #efe6d5);"
            "}"
            "#loginBrandTagline { font-size: 14px; font-weight: 600; color: #7a563a; }"
            "#loginBrandBody { font-size: 12px; color: #6f5d4d; line-height: 1.35em; }"
            "#loginBrandLogo { background: transparent; border: none; }"
            "#loginCard {"
            "  background: #ffffff;"
            "  border: 1px solid #eadfce;"
            "  border-radius: 18px;"
            "}"
            "#loginTitle { font-size: 24px; font-weight: 800; color: #5d2e06; }"
            "#loginSubtitle { font-size: 12px; color: #7a6b5a; }"
            "QFrame#loginFieldUser, QFrame#loginFieldPass {"
            "  background: #fbf8f3;"
            "  border: 1px solid #ded2c1;"
            "  border-radius: 12px;"
            "}"
            "QFrame#loginFieldUser[loginFocus=\"true\"], QFrame#loginFieldPass[loginFocus=\"true\"] {"
            "  border: 1px solid #c08a5b;"
            "  background: #fffdf8;"
            "}"
            "QLineEdit#lineEdit_5, QLineEdit#lineEdit_6 {"
            "  border: none;"
            "  background: transparent;"
            "  color: #2c2017;"
            "  padding: 8px 4px;"
            "  font-size: 12px;"
            "}"
            "QLineEdit#lineEdit_5:focus, QLineEdit#lineEdit_6:focus { color: #1f140b; }"
            "QPushButton#btnAjouter_8 {"
            "  min-height: 40px;"
            "  border: none;"
            "  border-radius: 10px;"
            "  padding: 8px 14px;"
            "  background: #5d2e06;"
            "  color: #ffffff;"
            "  font-weight: 700;"
            "}"
            "QPushButton#btnAjouter_8:hover { background: #6f3708; }"
            "QPushButton#btnAjouter_8:pressed { background: #4d2504; }"
            "QPushButton#btnSignUp, QPushButton#btnForgotPassword, QPushButton#btnFaceLogin {"
            "  min-height: 34px;"
            "  border-radius: 9px;"
            "  font-weight: 600;"
            "}"));
    }

    if (QFrame *card = findChild<QFrame *>(QStringLiteral("loginCard"))) {
        auto *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(42);
        shadow->setOffset(0, 16);
        shadow->setColor(QColor(48, 28, 10, 60));
        card->setGraphicsEffect(shadow);
    }
    if (QLabel *logo = findChild<QLabel *>(QStringLiteral("loginBrandLogo"))) {
        logo->setAutoFillBackground(false);
        logo->setPixmap(leatherAppLogoPixmap(300));
    }
    if (QLabel *iu = findChild<QLabel *>(QStringLiteral("loginIconUser")))
        iu->setText(QStringLiteral("👤"));
    if (QLabel *ip = findChild<QLabel *>(QStringLiteral("loginIconPass")))
        ip->setText(QStringLiteral("🔒"));

    m_loginUserFieldBox = findChild<QFrame *>(QStringLiteral("loginFieldUser"));
    m_loginPassFieldBox = findChild<QFrame *>(QStringLiteral("loginFieldPass"));

    if (ui->lineEdit_5)
        ui->lineEdit_5->installEventFilter(this);
    if (ui->lineEdit_6)
        ui->lineEdit_6->installEventFilter(this);

    refreshLoginFieldFocusHighlight();
}

void MainWindow::refreshLoginFieldFocusHighlight()
{
    const QWidget *fw = QApplication::focusWidget();
    const bool u = (fw == ui->lineEdit_5);
    const bool p = (fw == ui->lineEdit_6);

    const auto apply = [](QFrame *box, bool on) {
        if (!box)
            return;
        box->setProperty("loginFocus", on);
        if (box->style()) {
            box->style()->unpolish(box);
            box->style()->polish(box);
        }
    };

    apply(m_loginUserFieldBox, u);
    apply(m_loginPassFieldBox, p);
}

void MainWindow::applyGlobalAppearanceFromSettings()
{
    QSettings s;
    s.beginGroup(QStringLiteral("LeatherApp"));
    const QString accent = s.value(QStringLiteral("appearance/accent"), QStringLiteral("#5d2e06")).toString();
    const int fontPt = s.value(QStringLiteral("appearance/fontPointSize"), 11).toInt();
    // Application forcee en mode clair.
    s.setValue(QStringLiteral("appearance/theme"), QStringLiteral("light"));
    s.endGroup();

    QFont appFont(QStringLiteral("Segoe UI"), qBound(8, fontPt, 20));
    QApplication::setFont(appFont);

    QColor accentColor(accent);
    if (!accentColor.isValid())
        accentColor = QColor(QStringLiteral("#5d2e06"));

    qApp->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette pal;
    pal.setColor(QPalette::Window, QColor(0xf8, 0xf5, 0xef));
    pal.setColor(QPalette::WindowText, QColor(0x2c, 0x20, 0x17));
    pal.setColor(QPalette::Base, QColor(0xff, 0xfd, 0xf8));
    pal.setColor(QPalette::AlternateBase, QColor(0xfa, 0xf5, 0xeb));
    pal.setColor(QPalette::Text, QColor(0x2c, 0x20, 0x17));
    pal.setColor(QPalette::Button, QColor(0xfc, 0xf9, 0xf3));
    pal.setColor(QPalette::ButtonText, QColor(0x5d, 0x2e, 0x06));
    pal.setColor(QPalette::Link, accentColor);
    pal.setColor(QPalette::Highlight, accentColor.lighter(120));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(pal);

    if (ui->headerUserLabel) {
        ui->headerUserLabel->setStyleSheet(
            QStringLiteral("color: %1; font-weight: 600; font-size: 12px;").arg(accentColor.name()));
    }
}

void MainWindow::installApplicationTranslators()
{
    if (m_appTranslator) {
        qApp->removeTranslator(m_appTranslator);
        delete m_appTranslator;
        m_appTranslator = nullptr;
    }

    QSettings s;
    s.beginGroup(QStringLiteral("LeatherApp"));
    const QString code = s.value(QStringLiteral("language"), QStringLiteral("fr")).toString();
    s.endGroup();

    const QString file = QStringLiteral("leather_%1.qm").arg(code);
    const QString p1 = QApplication::applicationDirPath() + QStringLiteral("/translations/") + file;

    auto *tr = new QTranslator(this);
    if (QFileInfo::exists(p1) && tr->load(p1)) {
        m_appTranslator = tr;
        qApp->installTranslator(m_appTranslator);
    } else if (tr->load(QStringLiteral(":/translations/") + file)) {
        m_appTranslator = tr;
        qApp->installTranslator(m_appTranslator);
    } else {
        delete tr;
    }

    if (code.compare(QStringLiteral("ar"), Qt::CaseInsensitive) == 0)
        QApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QApplication::setLayoutDirection(Qt::LeftToRight);

    QLocale::setDefault(QLocale(code));

    if (ui)
        ui->retranslateUi(this);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange && ui)
        ui->retranslateUi(this);
    QMainWindow::changeEvent(event);
}

void MainWindow::refreshFaceLoginButtonVisibility()
{
    if (!ui->btnFaceLogin)
        return;
    QSettings s;
    s.beginGroup(QStringLiteral("LeatherApp"));
    const bool want = s.value(QStringLiteral("security/faceRecognition"), true).toBool();
    s.endGroup();
    // Toujours afficher si l utilisateur l a demande dans Parametres : sans OpenCV au build, le clic
    // ouvre FaceLoginDialog avec un message expliquant comment activer OpenCV.
    ui->btnFaceLogin->setVisible(want);
}

void MainWindow::applyProfileDisplayFromSettings()
{
    QSettings s;
    s.beginGroup(QStringLiteral("LeatherApp"));
    const QString name = s.value(QStringLiteral("profile/displayName")).toString().trimmed();
    const QString email = s.value(QStringLiteral("profile/email")).toString().trimmed();
    s.endGroup();
    if (!name.isEmpty() && ui->headerUserLabel)
        ui->headerUserLabel->setText(name);

    // Synchronise l’e-mail du profil Paramètres avec APP_USERS pour « mot de passe oublie ».
    if (!m_isAuthenticated || !db.isOpen() || !ui->lineEdit_5)
        return;
    const QString u = ui->lineEdit_5->text().trimmed();
    if (u.isEmpty() || email.isEmpty())
        return;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE APP_USERS SET EMAIL=:e WHERE UPPER(USERNAME)=UPPER(:u)"));
    q.bindValue(QStringLiteral(":e"), email);
    q.bindValue(QStringLiteral(":u"), u);
    q.exec();
}

void MainWindow::onPasswordChangeFromSettings(const QString &username, const QString &oldPassword,
                                              const QString &newPassword)
{
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Paramètres"),
                             QStringLiteral("Connexion Oracle indisponible. Impossible de modifier le mot de passe."));
        return;
    }
    QString authErr;
    if (!ensureAuthSchema(&authErr)) {
        QMessageBox::critical(this, QStringLiteral("Paramètres"),
                              QStringLiteral("Impossible de preparer APP_USERS:\n%1").arg(authErr));
        return;
    }
    if (!validateLoginCredentials(username, oldPassword)) {
        QMessageBox::warning(this, QStringLiteral("Paramètres"),
                             QStringLiteral("Mot de passe actuel incorrect."));
        return;
    }
    QSqlQuery upd(db);
    upd.prepare(QStringLiteral("UPDATE APP_USERS SET PASSWORD=:p WHERE UPPER(USERNAME)=UPPER(:u)"));
    upd.bindValue(QStringLiteral(":p"), hashPassword(newPassword));
    upd.bindValue(QStringLiteral(":u"), username);
    if (!upd.exec()) {
        QMessageBox::critical(this, QStringLiteral("Paramètres"),
                              QStringLiteral("Mise a jour impossible:\n%1").arg(upd.lastError().text().trimmed()));
        return;
    }
    if (auto *sw = qobject_cast<SettingsWindow *>(sender()))
        sw->clearPasswordFields();
    QMessageBox::information(this, QStringLiteral("Paramètres"), QStringLiteral("Mot de passe mis a jour."));
}

void MainWindow::on_btnSettings_clicked()
{
    SettingsWindow dlg(this);
    QString u = ui->lineEdit_5 ? ui->lineEdit_5->text().trimmed() : QString();
    if (u.isEmpty() && ui->headerUserLabel)
        u = ui->headerUserLabel->text().trimmed();
    dlg.setSessionUsername(u);
    connect(&dlg, &SettingsWindow::appearanceChanged, this, &MainWindow::applyGlobalAppearanceFromSettings);
    connect(&dlg, &SettingsWindow::languageChanged, this, &MainWindow::installApplicationTranslators);
    connect(&dlg, &SettingsWindow::passwordChangeRequested, this, &MainWindow::onPasswordChangeFromSettings);
    connect(&dlg, &SettingsWindow::profileChanged, this, &MainWindow::applyProfileDisplayFromSettings);
    connect(&dlg, &SettingsWindow::securitySettingsChanged, this, &MainWindow::refreshFaceLoginButtonVisibility);
    dlg.exec();
    applyGlobalAppearanceFromSettings();
    refreshFaceLoginButtonVisibility();
    installApplicationTranslators();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (QPushButton *hubBtn = qobject_cast<QPushButton *>(watched)) {
        if (hubBtn->property("hubInteractiveCard").toBool()) {
            auto animate = [hubBtn](bool hover) {
                auto *effect = qobject_cast<QGraphicsDropShadowEffect *>(hubBtn->graphicsEffect());
                if (!effect)
                    return;
                const qreal startBlur = effect->blurRadius();
                const qreal endBlur = hover ? 32.0 : 20.0;
                const QPointF startOffset = effect->offset();
                const QPointF endOffset = hover ? QPointF(0, 10) : QPointF(0, 6);
                auto *anim = new QVariantAnimation(hubBtn);
                anim->setDuration(160);
                anim->setStartValue(startBlur);
                anim->setEndValue(endBlur);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                QObject::connect(anim, &QVariantAnimation::valueChanged, hubBtn,
                                 [effect, startOffset, endOffset, anim](const QVariant &v) {
                    const qreal t = anim->currentTime() / qreal(qMax(1, anim->duration()));
                    effect->setBlurRadius(v.toReal());
                    effect->setOffset(startOffset.x() + (endOffset.x() - startOffset.x()) * t,
                                      startOffset.y() + (endOffset.y() - startOffset.y()) * t);
                });
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            };
            if (event->type() == QEvent::Enter)
                animate(true);
            else if (event->type() == QEvent::Leave)
                animate(false);
        }
    }

    if (watched == ui->lineEdit_5 || watched == ui->lineEdit_6) {
        switch (event->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            QTimer::singleShot(0, this, [this]() { refreshLoginFieldFocusHighlight(); });
            break;
        default:
            break;
        }
    }
    if (watched == m_fournisseurSmartMapCanvas) {
        if (event->type() == QEvent::Resize) {
            renderSmartMapFournisseurs(m_fournisseurSmartMapPoints);
        } else if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            QString tip;
            for (const SmartMapPointInfo &info : m_fournisseurSmartPointInfos) {
                const int dx = me->pos().x() - info.center.x();
                const int dy = me->pos().y() - info.center.y();
                if ((dx * dx + dy * dy) <= (info.radius * info.radius)) {
                    tip = info.tooltip;
                    break;
                }
            }
            if (!tip.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QToolTip::showText(me->globalPosition().toPoint(), tip, m_fournisseurSmartMapCanvas);
#else
                QToolTip::showText(me->globalPos(), tip, m_fournisseurSmartMapCanvas);
#endif
            }
            else
                QToolTip::hideText();
        } else if (event->type() == QEvent::Leave) {
            QToolTip::hideText();
        }
    } else if (watched == m_fournisseurSmartMapDialogCanvas) {
        if (event->type() == QEvent::Resize) {
            QVector<SmartMapPointInfo> tmpInfos;
            renderSmartMapOnCanvas(m_fournisseurSmartMapDialogCanvas, m_fournisseurSmartMapPoints, &tmpInfos);
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupHubAndBodyStack()
{
    if (m_bodyStack)
        return;

    QVBoxLayout *const bodyLay = qobject_cast<QVBoxLayout *>(ui->bodyWidget->layout());
    if (!bodyLay || bodyLay->count() < 2)
        return;

    QLayoutItem *const itNav = bodyLay->takeAt(0);
    QLayoutItem *const itContent = bodyLay->takeAt(0);
    QWidget *const nav = itNav ? itNav->widget() : nullptr;
    QWidget *const content = itContent ? itContent->widget() : nullptr;
    delete itNav;
    delete itContent;
    if (!nav || !content || nav != ui->topNavBar || content != ui->contentStack)
        return;

    m_bodyStack = new QStackedWidget(ui->bodyWidget);
    m_bodyStack->setObjectName(QStringLiteral("bodyStack"));

    auto *hubPage = new QWidget(m_bodyStack);
    hubPage->setObjectName(QStringLiteral("hubPage"));
    hubPage->setStyleSheet(QStringLiteral(
        "#hubPage {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #f8f5ef, stop:0.65 #f3ecdf, stop:1 #eee3d1);"
        "}"));

    auto *hubOuter = new QVBoxLayout(hubPage);
    hubOuter->setContentsMargins(56, 46, 56, 46);
    hubOuter->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("ROYAL LEATHER HOUSE"), hubPage);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral(
        "font-size: 32px; font-weight: 800; color: rgb(88, 41, 0); letter-spacing: 1px;"));

    auto *subtitle = new QLabel(QStringLiteral("Menu principal — choisissez un module"), hubPage);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(QStringLiteral("font-size: 14px; color: rgb(90, 70, 55);"));

    const QString hubBtnStyle = QStringLiteral(
        "QPushButton {"
        "  background: #ffffff;"
        "  color: rgb(88, 41, 0);"
        "  border: 1px solid #dfd2c2;"
        "  border-radius: 16px;"
        "  padding: 14px 18px;"
        "  font: 700 12pt \"Segoe UI\";"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: #fdf7ef;"
        "  border-color: #c08a5b;"
        "}"
        "QPushButton:pressed {"
        "  background: #f4e9d8;"
        "}");
    const QString hubLogoutStyle = QStringLiteral(
        "QPushButton {"
        "  background: #fff7f4;"
        "  color: #7a2f17;"
        "  border: 1px solid #ddb7a9;"
        "  border-radius: 12px;"
        "  padding: 10px 20px;"
        "  font: 700 11pt \"Segoe UI\";"
        "}"
        "QPushButton:hover {"
        "  background: #ffece5;"
        "  border-color: #c78672;"
        "}"
        "QPushButton:pressed {"
        "  background: #f5ddd4;"
        "}");

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(20);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    int row = 0;
    int col = 0;
    auto placeBtn = [&](QPushButton *b, const QString &icon, const QString &titleText) {
        b->setText(QStringLiteral("%1  %2").arg(icon, titleText));
        b->setMinimumHeight(92);
        b->setMinimumWidth(300);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(hubBtnStyle);
        b->setProperty("hubInteractiveCard", true);
        b->installEventFilter(this);
        auto *shadow = new QGraphicsDropShadowEffect(b);
        shadow->setBlurRadius(20);
        shadow->setOffset(0, 6);
        shadow->setColor(QColor(60, 35, 10, 26));
        b->setGraphicsEffect(shadow);
        grid->addWidget(b, row, col, Qt::AlignCenter);
        ++col;
        if (col >= 2) {
            col = 0;
            ++row;
        }
    };

    auto *btnDash = new QPushButton(hubPage);
    connect(btnDash, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        if (ui->contentStack)
            ui->contentStack->setCurrentIndex(kDashboardPageIndex);
        setActiveButton(ui->btnAccueil);
        updateShellChromeVisibility();
    });
    placeBtn(btnDash, QStringLiteral("📊"), QStringLiteral("Tableau de bord"));

    auto *btnEmp = new QPushButton(hubPage);
    connect(btnEmp, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        updateShellChromeVisibility();
        openEmployesModule();
    });
    placeBtn(btnEmp, QStringLiteral("👥"), QStringLiteral("Employés"));

    auto *btnFour = new QPushButton(hubPage);
    connect(btnFour, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        updateShellChromeVisibility();
        if (ui->contentStack)
            ui->contentStack->setCurrentIndex(3);
        setActiveButton(ui->btnFournisseurs);
    });
    placeBtn(btnFour, QStringLiteral("🏭"), QStringLiteral("Fournisseurs"));

    auto *btnProd = new QPushButton(hubPage);
    connect(btnProd, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        updateShellChromeVisibility();
        if (ui->btnProduits)
            ui->btnProduits->click();
    });
    placeBtn(btnProd, QStringLiteral("📦"), QStringLiteral("Produits"));

    auto *btnCli = new QPushButton(hubPage);
    connect(btnCli, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        updateShellChromeVisibility();
        if (ui->contentStack)
            ui->contentStack->setCurrentIndex(2);
        setActiveButton(ui->btnclients);
    });
    placeBtn(btnCli, QStringLiteral("🧑"), QStringLiteral("Clients"));

    auto *btnMp = new QPushButton(hubPage);
    connect(btnMp, &QPushButton::clicked, this, [this]() {
        if (m_bodyStack)
            m_bodyStack->setCurrentIndex(1);
        updateShellChromeVisibility();
        if (ui->btnMpremieres)
            ui->btnMpremieres->click();
    });
    placeBtn(btnMp, QStringLiteral("🧵"), QStringLiteral("Matière première"));

    auto *btnHubLogout = new QPushButton(QStringLiteral("⎋  Déconnexion"), hubPage);
    btnHubLogout->setCursor(Qt::PointingHandCursor);
    btnHubLogout->setMinimumHeight(44);
    btnHubLogout->setMinimumWidth(220);
    btnHubLogout->setStyleSheet(hubLogoutStyle);
    connect(btnHubLogout, &QPushButton::clicked, this, &MainWindow::on_btnMpremieres_2_clicked);

    hubOuter->addStretch(1);
    hubOuter->addWidget(title);
    hubOuter->addWidget(subtitle);
    hubOuter->addSpacing(28);
    hubOuter->addLayout(grid, 0);
    hubOuter->addSpacing(16);
    hubOuter->addWidget(btnHubLogout, 0, Qt::AlignHCenter);
    hubOuter->addStretch(2);

    auto *mainShell = new QWidget(m_bodyStack);
    auto *shellLay = new QVBoxLayout(mainShell);
    shellLay->setContentsMargins(0, 0, 0, 0);
    shellLay->setSpacing(0);
    shellLay->addWidget(ui->topNavBar, 0);
    shellLay->addWidget(ui->contentStack, 1);

    m_bodyStack->addWidget(hubPage);
    m_bodyStack->addWidget(mainShell);

    bodyLay->addWidget(m_bodyStack, 1);

    connect(m_bodyStack, &QStackedWidget::currentChanged, this, &MainWindow::updateShellChromeVisibility);

    if (ui->topNavLogo) {
        const QPixmap pm(QStringLiteral(":/images/logo.png"));
        if (!pm.isNull())
            ui->topNavLogo->setPixmap(pm.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Non connecté : zone login (contentStack page 0) dans l’interface principale sans hub.
    m_bodyStack->setCurrentIndex(1);
}

void MainWindow::setupAdministratorDashboard()
{
    if (m_adminDashboardSetup || !ui->pageDashboard || !ui->dashboardLayout)
        return;
    m_adminDashboardSetup = true;

    while (QLayoutItem *it = ui->dashboardLayout->takeAt(0)) {
        if (it->widget())
            delete it->widget();
        delete it;
    }

    auto *scroll = new QScrollArea(ui->pageDashboard);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"));

    m_dashContentRoot = new QWidget();
    auto *mainCol = new QVBoxLayout(m_dashContentRoot);
    mainCol->setContentsMargins(8, 8, 12, 16);
    mainCol->setSpacing(18);

    m_dashPageTitle = new QLabel(QStringLiteral("Tableau de bord analytique"), m_dashContentRoot);
    m_dashPageTitle->setStyleSheet(QStringLiteral(
        "font-size: 26px; font-weight: 700; color: rgb(88, 41, 0); border: none; background: transparent;"));
    mainCol->addWidget(m_dashPageTitle);

    m_dashPageSubtitle = new QLabel(
        QStringLiteral(
            "Indicateurs et graphiques alimentés par la base — rafraîchis à l’ouverture, après chaque modification, "
            "et toutes les 15 secondes tant que cette page est visible."),
        m_dashContentRoot);
    m_dashPageSubtitle->setWordWrap(true);
    m_dashPageSubtitle->setStyleSheet(QStringLiteral("font-size: 12px; color: #5a4a3a; border: none; background: transparent;"));
    mainCol->addWidget(m_dashPageSubtitle);

    auto *toolbar = new QFrame(m_dashContentRoot);
    toolbar->setObjectName(QStringLiteral("dashToolbar"));
    toolbar->setStyleSheet(QStringLiteral(
        "QFrame#dashToolbar { background: #ffffff; border: 1px solid #e0d4c4; border-radius: 14px; }"
        "QLabel { color: #5a4a3a; font-weight: 600; border: none; background: transparent; }"));
    auto *tbLay = new QHBoxLayout(toolbar);
    tbLay->setContentsMargins(14, 10, 14, 10);
    tbLay->setSpacing(16);
    tbLay->addWidget(new QLabel(QStringLiteral("Thème :"), toolbar));
    m_dashThemeCombo = new QComboBox(toolbar);
    m_dashThemeCombo->addItems({QStringLiteral("Clair"), QStringLiteral("Sombre")});
    m_dashThemeCombo->setMinimumWidth(120);
    tbLay->addWidget(m_dashThemeCombo);
    tbLay->addSpacing(8);
    tbLay->addWidget(new QLabel(QStringLiteral("Animations :"), toolbar));
    m_dashAnimCheck = new QCheckBox(QStringLiteral("Activer"), toolbar);
    m_dashAnimCheck->setChecked(true);
    tbLay->addWidget(m_dashAnimCheck);
    tbLay->addSpacing(8);
    tbLay->addWidget(new QLabel(QStringLiteral("Légende :"), toolbar));
    m_dashLegendCombo = new QComboBox(toolbar);
    m_dashLegendCombo->addItems({QStringLiteral("Bas"), QStringLiteral("Haut"), QStringLiteral("Gauche"),
                                 QStringLiteral("Droite"), QStringLiteral("Masquée")});
    m_dashLegendCombo->setMinimumWidth(130);
    tbLay->addWidget(m_dashLegendCombo);
    tbLay->addStretch(1);
    mainCol->addWidget(toolbar);

    connect(m_dashThemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        applyDashboardPresentation();
    });
    connect(m_dashAnimCheck, &QCheckBox::toggled, this, [this](bool) { applyDashboardPresentation(); });
    connect(m_dashLegendCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        applyDashboardPresentation();
    });

    auto *kpiGrid = new QGridLayout();
    kpiGrid->setHorizontalSpacing(14);
    kpiGrid->setVerticalSpacing(14);
    kpiGrid->addWidget(makeKpiCard(m_dashContentRoot, QStringLiteral("👥"), QStringLiteral("EMPLOYÉS"), &m_dashKpiEmployes), 0, 0);
    kpiGrid->addWidget(makeKpiCard(m_dashContentRoot, QStringLiteral("🏭"), QStringLiteral("FOURNISSEURS"), &m_dashKpiFournisseurs), 0, 1);
    kpiGrid->addWidget(makeKpiCard(m_dashContentRoot, QStringLiteral("📦"), QStringLiteral("PRODUITS"), &m_dashKpiProduits), 0, 2);
    kpiGrid->addWidget(makeKpiCard(m_dashContentRoot, QStringLiteral("👤"), QStringLiteral("CLIENTS"), &m_dashKpiClients), 0, 3);
    kpiGrid->addWidget(makeKpiCard(m_dashContentRoot, QStringLiteral("🧵"), QStringLiteral("MATIÈRES PREMIÈRES"), &m_dashKpiMatieres), 0, 4);
    for (int c = 0; c < 5; ++c)
        kpiGrid->setColumnStretch(c, 1);
    mainCol->addLayout(kpiGrid);

    auto *catalogueCard = new QFrame(m_dashContentRoot);
    catalogueCard->setObjectName(QStringLiteral("dashCatalogueCard"));
    catalogueCard->setStyleSheet(QStringLiteral(
        "QFrame#dashCatalogueCard { background:#ffffff; border:1px solid #e0d4c4; border-radius:14px; }"
        "QLabel#dashCatalogueTitle { color:#5a3a22; font-size:14px; font-weight:700; border:none; background:transparent; }"
        "QLabel#dashCatalogueStats { color:#4a4036; font-size:12px; border:none; background:transparent; }"
        "QLabel#dashCatalogueTop { color:#6b5a49; font-size:11px; border:none; background:transparent; }"
        "QProgressBar { border:1px solid #e4d8ca; border-radius:8px; background:#f8f2ea; text-align:center; min-height:22px; }"
        "QProgressBar::chunk { border-radius:7px; }"
        "QProgressBar#dashCatalogueAvailableBar::chunk { background:#2e7d32; }"
        "QProgressBar#dashCatalogueLowBar::chunk { background:#f39c12; }"
        "QProgressBar#dashCatalogueOutBar::chunk { background:#c0392b; }"));
    auto *catLay = new QVBoxLayout(catalogueCard);
    catLay->setContentsMargins(14, 12, 14, 12);
    catLay->setSpacing(8);
    auto *catTitle = new QLabel(QStringLiteral("Statistiques catalogue produits"), catalogueCard);
    catTitle->setObjectName(QStringLiteral("dashCatalogueTitle"));
    m_dashCatalogueStats = new QLabel(catalogueCard);
    m_dashCatalogueStats->setObjectName(QStringLiteral("dashCatalogueStats"));
    m_dashCatalogueStats->setWordWrap(true);
    auto makeStockBar = [&](const QString &objName) -> QProgressBar * {
        auto *bar = new QProgressBar(catalogueCard);
        bar->setObjectName(objName);
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setTextVisible(true);
        return bar;
    };
    m_dashCatalogueAvailableBar = makeStockBar(QStringLiteral("dashCatalogueAvailableBar"));
    m_dashCatalogueLowBar = makeStockBar(QStringLiteral("dashCatalogueLowBar"));
    m_dashCatalogueOutBar = makeStockBar(QStringLiteral("dashCatalogueOutBar"));
    m_dashCatalogueTopCategories = new QLabel(catalogueCard);
    m_dashCatalogueTopCategories->setObjectName(QStringLiteral("dashCatalogueTop"));
    m_dashCatalogueTopCategories->setWordWrap(true);
    catLay->addWidget(catTitle);
    catLay->addWidget(m_dashCatalogueStats);
    catLay->addWidget(m_dashCatalogueAvailableBar);
    catLay->addWidget(m_dashCatalogueLowBar);
    catLay->addWidget(m_dashCatalogueOutBar);
    catLay->addWidget(m_dashCatalogueTopCategories);
    mainCol->addWidget(catalogueCard);

    m_dashBarChart = new QChart();
    m_dashBarChart->setTitle(QStringLiteral("Produits par type"));
    m_dashBarChart->setMargins(QMargins(12, 8, 12, 12));
    m_dashBarChart->legend()->setVisible(true);
    m_dashBarChartView = new QChartView(m_dashBarChart, m_dashContentRoot);
    m_dashBarChartView->setRenderHint(QPainter::Antialiasing);

    m_dashPieSeries = new QPieSeries();
    m_dashPieSeries->append(QStringLiteral("—"), 1);
    m_dashPieSeries->setLabelsVisible(true);
    m_dashPieChart = new QChart();
    m_dashPieChart->addSeries(m_dashPieSeries);
    m_dashPieChart->setTitle(QStringLiteral("Répartition des clients par catégorie"));
    m_dashPieChart->legend()->setVisible(true);
    m_dashPieChart->setMargins(QMargins(8, 8, 8, 8));
    m_dashPieChartView = new QChartView(m_dashPieChart, m_dashContentRoot);
    m_dashPieChartView->setRenderHint(QPainter::Antialiasing);

    m_dashOrdersBarChart = new QChart();
    m_dashOrdersBarChart->setTitle(QStringLiteral("Évolution des commandes par date"));
    m_dashOrdersBarChart->legend()->setVisible(true);
    m_dashOrdersBarChart->setMargins(QMargins(12, 8, 12, 12));
    m_dashOrdersBarChartView = new QChartView(m_dashOrdersBarChart, m_dashContentRoot);
    m_dashOrdersBarChartView->setRenderHint(QPainter::Antialiasing);

    auto *chartsGrid = new QGridLayout();
    chartsGrid->setHorizontalSpacing(18);
    chartsGrid->setVerticalSpacing(18);
    chartsGrid->setColumnStretch(0, 1);
    chartsGrid->setColumnStretch(1, 1);

    m_dashCardBar = makeDashboardChartCard(
        m_dashContentRoot,
        QStringLiteral("Histogramme — volume par type de produit"),
        m_dashBarChartView,
        272);
    m_dashCardPie = makeDashboardChartCard(
        m_dashContentRoot,
        QStringLiteral("Secteurs — clients par catégorie"),
        m_dashPieChartView,
        272);
    chartsGrid->addWidget(m_dashCardBar, 0, 0);
    chartsGrid->addWidget(m_dashCardPie, 0, 1);

    m_dashCardLine = makeDashboardChartCard(
        m_dashContentRoot,
        QStringLiteral("Courbe — commandes dans le temps"),
        m_dashOrdersBarChartView,
        288);
    chartsGrid->addWidget(m_dashCardLine, 1, 0, 1, 2);
    mainCol->addLayout(chartsGrid);

    scroll->setWidget(m_dashContentRoot);
    ui->dashboardLayout->addWidget(scroll);

    connect(ui->contentStack, &QStackedWidget::currentChanged, this, [this](int idx) {
        if (idx == kDashboardPageIndex)
            refreshAdministratorDashboard();
    });

    if (!m_statsAutoRefreshTimer) {
        m_statsAutoRefreshTimer = new QTimer(this);
        m_statsAutoRefreshTimer->setInterval(15000);
        connect(m_statsAutoRefreshTimer, &QTimer::timeout, this, [this]() {
            if (ui->contentStack && ui->contentStack->currentIndex() == kDashboardPageIndex)
                onStatisticsDataChanged();
        });
        m_statsAutoRefreshTimer->start();
    }

    refreshAdministratorDashboard();
}

void MainWindow::refreshAdministratorDashboard()
{
    if (!m_dashKpiEmployes)
        return;

    const DashboardCounts c = fetchDashboardCounts(db);
    m_dashKpiEmployes->setText(QString::number(c.employes));
    m_dashKpiFournisseurs->setText(QString::number(c.fournisseurs));
    m_dashKpiProduits->setText(QString::number(c.produits));
    m_dashKpiClients->setText(QString::number(c.clients));
    m_dashKpiMatieres->setText(QString::number(c.matieres));

    if (m_dashCatalogueStats) {
        const CatalogueDashboardStats cs = fetchCatalogueDashboardStats(db);
        const int total = qMax(1, cs.totalProduits);
        const int pctAvailable = qBound(0, qRound((100.0 * cs.stockDisponible) / total), 100);
        const int pctLow = qBound(0, qRound((100.0 * cs.stockFaible) / total), 100);
        const int pctOut = qBound(0, qRound((100.0 * cs.stockRupture) / total), 100);

        QString topCats = QStringLiteral("Top catégories : N/A");
        if (!cs.topCategories.isEmpty()) {
            QStringList lines;
            for (const auto &it : cs.topCategories)
                lines << QStringLiteral("%1 (%2)").arg(it.first).arg(it.second);
            topCats = QStringLiteral("Top catégories : %1").arg(lines.join(QStringLiteral(" • ")));
        }
        m_dashCatalogueStats->setText(
            QStringLiteral("Total produits : %1  |  Disponible : %2  |  Stock faible : %3  |  Rupture : %4")
                .arg(cs.totalProduits)
                .arg(cs.stockDisponible)
                .arg(cs.stockFaible)
                .arg(cs.stockRupture));
        if (m_dashCatalogueAvailableBar) {
            m_dashCatalogueAvailableBar->setValue(pctAvailable);
            m_dashCatalogueAvailableBar->setFormat(QStringLiteral("Disponible (>5): %1 (%2%)").arg(cs.stockDisponible).arg(pctAvailable));
        }
        if (m_dashCatalogueLowBar) {
            m_dashCatalogueLowBar->setValue(pctLow);
            m_dashCatalogueLowBar->setFormat(QStringLiteral("Stock faible (1-5): %1 (%2%)").arg(cs.stockFaible).arg(pctLow));
        }
        if (m_dashCatalogueOutBar) {
            m_dashCatalogueOutBar->setValue(pctOut);
            m_dashCatalogueOutBar->setFormat(QStringLiteral("Rupture (0): %1 (%2%)").arg(cs.stockRupture).arg(pctOut));
        }
        if (m_dashCatalogueTopCategories)
            m_dashCatalogueTopCategories->setText(topCats);
    }

    loadBarChart();
    loadPieChart();
    loadLineChart();
    applyDashboardPresentation();
}

Qt::Alignment MainWindow::dashboardLegendAlignment() const
{
    if (!m_dashLegendCombo)
        return Qt::AlignBottom;
    switch (m_dashLegendCombo->currentIndex()) {
    case 1:
        return Qt::AlignTop;
    case 2:
        return Qt::AlignLeft;
    case 3:
        return Qt::AlignRight;
    case 4:
        return Qt::AlignBottom;
    default:
        return Qt::AlignBottom;
    }
}

void MainWindow::applyDashboardChartColors(bool dark)
{
    const QColor chartBg = dark ? QColor(30, 41, 59) : QColor(253, 252, 249);
    const QColor plotBg = dark ? QColor(15, 23, 42) : QColor(255, 255, 255);
    const QColor titleC = dark ? QColor(226, 232, 240) : QColor(90, 58, 34);
    const QColor labelC = dark ? QColor(148, 163, 184) : QColor(74, 64, 54);
    const QColor legendC = dark ? QColor(203, 213, 225) : QColor(90, 74, 62);
    const QColor gridC = dark ? QColor(51, 65, 85) : QColor(230, 220, 210);
    const QColor axisLine = dark ? QColor(71, 85, 105) : QColor(200, 190, 175);

    auto styleChart = [&](QChart *ch) {
        if (!ch)
            return;
        ch->setBackgroundRoundness(8);
        ch->setBackgroundBrush(QBrush(chartBg));
        ch->setTitleBrush(QBrush(titleC));
        ch->setPlotAreaBackgroundBrush(QBrush(plotBg));
        ch->setPlotAreaBackgroundVisible(true);
        for (QAbstractAxis *ax : ch->axes()) {
            ax->setLabelsBrush(QBrush(labelC));
            if (!ax->titleText().isEmpty())
                ax->setTitleBrush(QBrush(labelC));
            ax->setLinePenColor(axisLine);
            ax->setGridLineColor(gridC);
        }
        if (ch->legend()) {
            ch->legend()->setBrush(QBrush(dark ? QColor(30, 41, 59, 220) : QColor(255, 255, 255, 230)));
            ch->legend()->setPen(QPen(dark ? QColor(71, 85, 105) : QColor(224, 212, 196)));
            ch->legend()->setLabelBrush(QBrush(legendC));
        }
    };

    styleChart(m_dashBarChart);
    styleChart(m_dashPieChart);
    styleChart(m_dashOrdersBarChart);

    if (m_dashOrdersBarChart) {
        for (QAbstractSeries *s : m_dashOrdersBarChart->series()) {
            if (auto *ls = qobject_cast<QLineSeries *>(s))
                ls->setPen(QPen(dark ? QColor(96, 165, 250) : QColor(93, 46, 6), 2));
        }
    }
    if (m_dashBarChart) {
        for (QAbstractSeries *s : m_dashBarChart->series()) {
            if (auto *bs = qobject_cast<QBarSeries *>(s)) {
                for (QBarSet *set : bs->barSets())
                    set->setBrush(QBrush(dark ? QColor(251, 191, 36) : QColor(184, 134, 11)));
            }
        }
    }
    if (m_dashPieChart && m_dashPieSeries) {
        static const QVector<QColor> pieLight = {
            QColor(139, 90, 43), QColor(205, 133, 63), QColor(93, 46, 6), QColor(184, 134, 11),
            QColor(160, 82, 45), QColor(210, 105, 30)};
        static const QVector<QColor> pieDark = {
            QColor(251, 191, 36), QColor(252, 211, 77), QColor(248, 113, 113), QColor(125, 211, 252),
            QColor(167, 139, 250), QColor(52, 211, 153)};
        const QVector<QColor> &pal = dark ? pieDark : pieLight;
        int i = 0;
        for (QPieSlice *sl : m_dashPieSeries->slices()) {
            sl->setBrush(pal[i % pal.size()]);
            sl->setLabelColor(dark ? QColor(226, 232, 240) : QColor(55, 40, 30));
            ++i;
        }
    }

    if (m_dashContentRoot) {
        m_dashContentRoot->setStyleSheet(dark ? QStringLiteral("QWidget { background: #0f172a; }")
                                             : QStringLiteral("QWidget { background: transparent; }"));
    }
    if (m_dashPageTitle) {
        m_dashPageTitle->setStyleSheet(dark ? QStringLiteral(
            "font-size: 26px; font-weight: 700; color: #e2e8f0; border: none; background: transparent;")
                                           : QStringLiteral(
                                               "font-size: 26px; font-weight: 700; color: rgb(88, 41, 0); border: none; "
                                               "background: transparent;"));
    }
    if (m_dashPageSubtitle) {
        m_dashPageSubtitle->setStyleSheet(dark ? QStringLiteral("font-size: 12px; color: #94a3b8; border: none; background: transparent;")
                                               : QStringLiteral("font-size: 12px; color: #5a4a3a; border: none; background: transparent;"));
    }
    if (m_dashThemeCombo) {
        m_dashThemeCombo->setStyleSheet(dark ? QStringLiteral(
            "QComboBox { background:#1e293b; color:#e2e8f0; border:1px solid #334155; border-radius:8px; padding:4px 8px; }")
                                            : QString());
    }
    if (m_dashLegendCombo) {
        m_dashLegendCombo->setStyleSheet(m_dashThemeCombo ? m_dashThemeCombo->styleSheet() : QString());
    }
    if (m_dashAnimCheck) {
        m_dashAnimCheck->setStyleSheet(dark ? QStringLiteral("QCheckBox { color: #e2e8f0; }") : QString());
    }

    const QString cardLight = QStringLiteral(
        "QFrame#dashChartCard { background: #ffffff; border: 1px solid #e0d4c4; border-radius: 16px; }"
        "QLabel#dashCardTitle { color: #5a3a22; font-size: 14px; font-weight: 700; border: none; background: transparent; }");
    const QString cardDark = QStringLiteral(
        "QFrame#dashChartCard { background: #1e293b; border: 1px solid #334155; border-radius: 16px; }"
        "QLabel#dashCardTitle { color: #e2e8f0; font-size: 14px; font-weight: 700; border: none; background: transparent; }");
    const QString cardSs = dark ? cardDark : cardLight;
    for (QFrame *fr : {m_dashCardBar, m_dashCardPie, m_dashCardLine}) {
        if (fr)
            fr->setStyleSheet(cardSs);
    }

    const QString kpiLight = QStringLiteral(
        "QFrame#dashKpiCard { background-color: #ffffff; border: 1px solid #e0d4c4; border-radius: 14px; }");
    const QString kpiDark = QStringLiteral(
        "QFrame#dashKpiCard { background-color: #1e293b; border: 1px solid #334155; border-radius: 14px; }");
    if (m_dashContentRoot) {
        const QList<QFrame *> kpis = m_dashContentRoot->findChildren<QFrame *>(QStringLiteral("dashKpiCard"));
        for (QFrame *k : kpis)
            k->setStyleSheet(dark ? kpiDark : kpiLight);
        for (QLabel *lab : m_dashContentRoot->findChildren<QLabel *>(QStringLiteral("dashKpiValue"))) {
            lab->setStyleSheet(dark ? QStringLiteral(
                "font-size: 28px; font-weight: 700; color: #f1f5f9; border: none; background: transparent;")
                                   : QStringLiteral(
                                       "font-size: 28px; font-weight: 700; color: rgb(88, 41, 0); border: none; "
                                       "background: transparent;"));
        }
        for (QLabel *lab : m_dashContentRoot->findChildren<QLabel *>(QStringLiteral("dashKpiTitle"))) {
            lab->setStyleSheet(dark ? QStringLiteral(
                "font-size: 11px; color: #94a3b8; border: none; background: transparent; font-weight: 600;")
                                    : QStringLiteral(
                                        "font-size: 11px; color: #6b5345; border: none; background: transparent; "
                                        "font-weight: 600;"));
        }
    }

    if (QFrame *tb = m_dashContentRoot ? m_dashContentRoot->findChild<QFrame *>(QStringLiteral("dashToolbar")) : nullptr) {
        tb->setStyleSheet(dark ? QStringLiteral(
            "QFrame#dashToolbar { background: #1e293b; border: 1px solid #334155; border-radius: 14px; }"
            "QLabel { color: #cbd5e1; font-weight: 600; border: none; background: transparent; }")
                              : QStringLiteral(
                                    "QFrame#dashToolbar { background: #ffffff; border: 1px solid #e0d4c4; border-radius: 14px; }"
                                    "QLabel { color: #5a4a3a; font-weight: 600; border: none; background: transparent; }"));
    }

}

void MainWindow::applyDashboardPresentation()
{
    if (!m_dashBarChart || !m_dashThemeCombo)
        return;
    const bool dark = (m_dashThemeCombo->currentIndex() == 1);
    const bool anim = !m_dashAnimCheck || m_dashAnimCheck->isChecked();
    const auto animOpt = anim ? QChart::AllAnimations : QChart::NoAnimation;
    for (QChart *c : {m_dashBarChart, m_dashPieChart, m_dashOrdersBarChart}) {
        if (!c)
            continue;
        c->setAnimationOptions(animOpt);
        if (c->legend()) {
            const bool hideLeg = m_dashLegendCombo && m_dashLegendCombo->currentIndex() == 4;
            c->legend()->setVisible(!hideLeg);
            if (!hideLeg)
                c->legend()->setAlignment(dashboardLegendAlignment());
        }
    }
    applyDashboardChartColors(dark);
}

void MainWindow::onStatisticsDataChanged()
{
    if (!m_adminDashboardSetup)
        return;
    refreshAdministratorDashboard();
}

void MainWindow::loadLineChart()
{
    if (!m_dashOrdersBarChart || !m_dashOrdersBarChartView)
        return;

    clearChartSeriesAndAxes(m_dashOrdersBarChart);
    const QVector<QPair<QDate, int>> byDate = fetchOrdersByDate(db);
    if (byDate.isEmpty()) {
        m_dashOrdersBarChart->setTitle(QStringLiteral("Évolution des commandes (aucune donnée)"));
        if (m_dashOrdersBarChart->legend())
            m_dashOrdersBarChart->legend()->setVisible(false);
        m_dashOrdersBarChartView->update();
        return;
    }

    auto *lineSeries = new QLineSeries();
    lineSeries->setName(QStringLiteral("Commandes"));
    int maxY = 1;
    for (const auto &pt : byDate) {
        lineSeries->append(QDateTime(pt.first, QTime(0, 0)).toMSecsSinceEpoch(), pt.second);
        maxY = qMax(maxY, pt.second);
    }
    m_dashOrdersBarChart->addSeries(lineSeries);

    auto *axisX = new QDateTimeAxis();
    axisX->setTitleText(QStringLiteral("Date"));
    axisX->setFormat(QStringLiteral("dd/MM"));
    axisX->setTickCount(qMin(8, qMax(2, byDate.size())));
    axisX->setRange(QDateTime(byDate.first().first, QTime(0, 0)),
                    QDateTime(byDate.last().first, QTime(23, 59, 59)));
    m_dashOrdersBarChart->addAxis(axisX, Qt::AlignBottom);

    auto *axisY = new QValueAxis();
    axisY->setTitleText(QStringLiteral("Nombre de commandes"));
    axisY->setLabelFormat(QStringLiteral("%d"));
    axisY->setRange(0, maxY + qMax(2, maxY / 8));
    m_dashOrdersBarChart->addAxis(axisY, Qt::AlignLeft);

    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);
    if (m_dashOrdersBarChart->legend()) {
        m_dashOrdersBarChart->legend()->setVisible(true);
        m_dashOrdersBarChart->legend()->setAlignment(dashboardLegendAlignment());
    }
    m_dashOrdersBarChart->setTitle(QStringLiteral("Évolution des commandes par date"));
    m_dashOrdersBarChartView->update();
}

void MainWindow::loadBarChart()
{
    if (!m_dashBarChart || !m_dashBarChartView)
        return;

    const QVector<QPair<QString, int>> productsByType = fetchProductsByType(db);
    clearChartSeriesAndAxes(m_dashBarChart);

    auto *set = new QBarSet(QStringLiteral("Produits"));
    QStringList categories;
    int maxVal = 1;
    if (productsByType.isEmpty()) {
        *set << 0;
        categories << QStringLiteral("(aucune donnée)");
    } else {
        for (const auto &pr : productsByType) {
            *set << pr.second;
            categories << pr.first;
            maxVal = qMax(maxVal, pr.second);
        }
    }

    auto *series = new QBarSeries();
    series->append(set);
    m_dashBarChart->addSeries(series);

    auto *axisX = new QBarCategoryAxis();
    axisX->setTitleText(QStringLiteral("Type de cuir"));
    axisX->append(categories);
    m_dashBarChart->addAxis(axisX, Qt::AlignBottom);
    auto *axisY = new QValueAxis();
    axisY->setTitleText(QStringLiteral("Nombre de produits"));
    axisY->setLabelFormat(QStringLiteral("%d"));
    axisY->setRange(0, maxVal + qMax(2, maxVal / 8));
    m_dashBarChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    if (m_dashBarChart->legend()) {
        m_dashBarChart->legend()->setVisible(true);
        m_dashBarChart->legend()->setAlignment(dashboardLegendAlignment());
    }
    m_dashBarChart->setTitle(QStringLiteral("Produits par type"));
    m_dashBarChartView->update();
}

void MainWindow::loadPieChart()
{
    if (!m_dashPieSeries || !m_dashPieChart || !m_dashPieChartView)
        return;

    m_dashPieSeries->clear();
    const QVector<QPair<QString, int>> catSlices = fetchClientCategorySlices(db);
    if (catSlices.isEmpty()) {
        m_dashPieSeries->append(QStringLiteral("Sans données"), 1);
    } else {
        static const QVector<QColor> piePalette = {
            QColor(139, 90, 43), QColor(205, 133, 63), QColor(93, 46, 6), QColor(184, 134, 11),
            QColor(160, 82, 45), QColor(210, 105, 30)};
        int i = 0;
        for (const auto &pr : catSlices) {
            QPieSlice *sl = m_dashPieSeries->append(pr.first, pr.second);
            sl->setLabelVisible(true);
            sl->setBrush(piePalette[i % piePalette.size()]);
            ++i;
        }
    }
    m_dashPieChart->setTitle(QStringLiteral("Répartition des clients par catégorie"));
    if (m_dashPieChart->legend()) {
        m_dashPieChart->legend()->setVisible(true);
        m_dashPieChart->legend()->setAlignment(dashboardLegendAlignment());
    }
    m_dashPieChartView->update();
}

namespace {

bool leatherMigrateAppUsersEmailColumn(QSqlDatabase &db, QString *errorMessage)
{
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME = 'APP_USERS' AND COLUMN_NAME = 'EMAIL'"))) {
        if (errorMessage)
            *errorMessage = q.lastError().text().trimmed();
        return false;
    }
    if (!q.next())
        return false;
    if (q.value(0).toInt() > 0)
        return true;

    QSqlQuery a(db);
    if (!a.exec(QStringLiteral("ALTER TABLE APP_USERS ADD (EMAIL VARCHAR2(255))"))) {
        if (errorMessage)
            *errorMessage = a.lastError().text().trimmed();
        return false;
    }
    return true;
}

} // namespace

bool MainWindow::ensureAuthSchema(QString *errorMessage)
{
    QSqlQuery q(db);
    const QString createSql = QStringLiteral(
        "CREATE TABLE APP_USERS ("
        "USERNAME VARCHAR2(64) PRIMARY KEY, "
        "PASSWORD VARCHAR2(128) NOT NULL, "
        "ROLE VARCHAR2(32) DEFAULT 'ADMIN', "
        "EMAIL VARCHAR2(255), "
        "CREATED_AT DATE DEFAULT SYSDATE)");

    if (q.exec(createSql))
        return true;

    const QString err = q.lastError().text().trimmed();
    if (err.contains(QStringLiteral("ORA-00955"), Qt::CaseInsensitive)) {
        QString migErr;
        if (!leatherMigrateAppUsersEmailColumn(db, &migErr)) {
            if (errorMessage)
                *errorMessage = migErr;
            return false;
        }
        return true;
    }

    if (errorMessage)
        *errorMessage = err;
    return false;
}

bool MainWindow::ensureDefaultAdminUser(QString *errorMessage)
{
    QSqlQuery check(db);
    check.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_USERS WHERE UPPER(USERNAME)=UPPER(:u)"));
    check.bindValue(QStringLiteral(":u"), QStringLiteral("admin"));
    if (!check.exec() || !check.next()) {
        if (errorMessage)
            *errorMessage = check.lastError().text().trimmed();
        return false;
    }

    if (check.value(0).toInt() > 0)
        return true;

    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
        "INSERT INTO APP_USERS (USERNAME, PASSWORD, ROLE, EMAIL) "
        "VALUES (:u, :p, :r, NULL)"));
    ins.bindValue(QStringLiteral(":u"), QStringLiteral("admin"));
    ins.bindValue(QStringLiteral(":p"), hashPassword(QStringLiteral("admin123")));
    ins.bindValue(QStringLiteral(":r"), QStringLiteral("ADMIN"));
    if (!ins.exec()) {
        if (errorMessage)
            *errorMessage = ins.lastError().text().trimmed();
        return false;
    }
    return true;
}

QString MainWindow::hashPassword(const QString &plain) const
{
    const QByteArray digest = QCryptographicHash::hash(plain.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(digest.toHex());
}

bool MainWindow::validateLoginCredentials(const QString &username, const QString &password)
{
    const QString normalizedUser = username.trimmed();
    const QString providedHash = hashPassword(password);

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT PASSWORD FROM APP_USERS "
        "WHERE UPPER(USERNAME)=UPPER(:u)"));
    q.bindValue(QStringLiteral(":u"), normalizedUser);

    if (!q.exec() || !q.next())
        return false;

    const QString stored = q.value(0).toString().trimmed();
    if (stored.compare(providedHash, Qt::CaseInsensitive) == 0)
        return true;

    // Migration transparente: ancien mot de passe en clair -> hash SHA-256.
    if (stored == password) {
        QSqlQuery upd(db);
        upd.prepare(QStringLiteral(
            "UPDATE APP_USERS SET PASSWORD=:p WHERE UPPER(USERNAME)=UPPER(:u)"));
        upd.bindValue(QStringLiteral(":p"), providedHash);
        upd.bindValue(QStringLiteral(":u"), normalizedUser);
        upd.exec();
        return true;
    }
    return false;
}

void MainWindow::on_btnAjouter_8_clicked()
{
    const QString username = ui->lineEdit_5 ? ui->lineEdit_5->text().trimmed() : QString();
    const QString password = ui->lineEdit_6 ? ui->lineEdit_6->text() : QString();

    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Connexion"),
                             QStringLiteral("Connexion Oracle indisponible. Verifiez le DSN ODBC projet_cuir."));
        return;
    }

    QString authErr;
    if (!ensureAuthSchema(&authErr)) {
        QMessageBox::critical(this, QStringLiteral("Connexion"),
                              QStringLiteral("Impossible de preparer la table APP_USERS:\n%1").arg(authErr));
        return;
    }
    if (!ensureDefaultAdminUser(&authErr)) {
        QMessageBox::critical(this, QStringLiteral("Connexion"),
                              QStringLiteral("Impossible de preparer le compte admin:\n%1").arg(authErr));
        return;
    }

    if (!validateLoginCredentials(username, password)) {
        QMessageBox::warning(this, QStringLiteral("Connexion"),
                             QStringLiteral("Identifiants invalides."));
        if (ui->lineEdit_6)
            ui->lineEdit_6->clear();
        if (ui->lineEdit_5)
            ui->lineEdit_5->setFocus();
        return;
    }

    applyAuthenticationUiState(true);
    if (ui->headerUserLabel)
        ui->headerUserLabel->setText(username.isEmpty() ? QStringLiteral("Administrateur") : username);
    applyProfileDisplayFromSettings();
    if (m_bodyStack)
        m_bodyStack->setCurrentIndex(0);
    updateShellChromeVisibility();
    QMessageBox::information(this, QStringLiteral("Connexion"), QStringLiteral("Connexion reussie."));
}

void MainWindow::onFaceLoginRequested()
{
    FaceLoginDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    finishFaceIdLoginSession();
}

void MainWindow::finishFaceIdLoginSession()
{
    applyAuthenticationUiState(true);
    if (ui->headerUserLabel)
        ui->headerUserLabel->setText(QStringLiteral("Administrateur (Face ID)"));
    applyProfileDisplayFromSettings();
    if (m_bodyStack)
        m_bodyStack->setCurrentIndex(0);
    updateShellChromeVisibility();
    QMessageBox::information(this, QStringLiteral("Reconnaissance faciale"),
                             QStringLiteral("Connexion par caméra réussie."));
}

void MainWindow::on_btnSignUp_clicked()
{
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Connexion Oracle indisponible. Verifiez le DSN ODBC projet_cuir."));
        return;
    }

    QString authErr;
    if (!ensureAuthSchema(&authErr)) {
        QMessageBox::critical(this, QStringLiteral("Inscription"),
                              QStringLiteral("Impossible de preparer la table APP_USERS:\n%1").arg(authErr));
        return;
    }

    bool ok = false;
    const QString username = QInputDialog::getText(
        this,
        QStringLiteral("Inscription"),
        QStringLiteral("Nom d'utilisateur:"),
        QLineEdit::Normal,
        QString(),
        &ok).trimmed();
    if (!ok)
        return;
    if (username.size() < 3) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Le nom d'utilisateur doit contenir au moins 3 caracteres."));
        return;
    }

    const QString password = QInputDialog::getText(
        this,
        QStringLiteral("Inscription"),
        QStringLiteral("Mot de passe:"),
        QLineEdit::Password,
        QString(),
        &ok);
    if (!ok)
        return;
    if (password.size() < 4) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Le mot de passe doit contenir au moins 4 caracteres."));
        return;
    }
    const QString confirmPassword = QInputDialog::getText(
        this,
        QStringLiteral("Inscription"),
        QStringLiteral("Confirmer le mot de passe:"),
        QLineEdit::Password,
        QString(),
        &ok);
    if (!ok)
        return;
    if (confirmPassword != password) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Les mots de passe ne correspondent pas."));
        return;
    }

    const QString email = QInputDialog::getText(
        this,
        QStringLiteral("Inscription"),
        QStringLiteral("Adresse e-mail (obligatoire pour \"Mot de passe oublie\") :"),
        QLineEdit::Normal,
        QString(),
        &ok).trimmed();
    if (!ok)
        return;
    if (email.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("L'adresse e-mail est obligatoire."));
        return;
    }
    const QRegularExpression emailRegex(QStringLiteral("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$"),
                                        QRegularExpression::CaseInsensitiveOption);
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Format d'adresse e-mail invalide."));
        return;
    }

    QSqlQuery emailExists(db);
    emailExists.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_USERS WHERE LOWER(TRIM(EMAIL)) = LOWER(TRIM(:e))"));
    emailExists.bindValue(QStringLiteral(":e"), email);
    if (!emailExists.exec() || !emailExists.next()) {
        QMessageBox::critical(this, QStringLiteral("Inscription"),
                              QStringLiteral("Verification e-mail impossible:\n%1")
                                  .arg(emailExists.lastError().text().trimmed()));
        return;
    }
    if (emailExists.value(0).toInt() > 0) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Cette adresse e-mail est deja associee a un compte."));
        return;
    }

    QSqlQuery exists(db);
    exists.prepare(QStringLiteral("SELECT COUNT(*) FROM APP_USERS WHERE UPPER(USERNAME)=UPPER(:u)"));
    exists.bindValue(QStringLiteral(":u"), username);
    if (!exists.exec() || !exists.next()) {
        QMessageBox::critical(this, QStringLiteral("Inscription"),
                              QStringLiteral("Verification utilisateur impossible:\n%1")
                                  .arg(exists.lastError().text().trimmed()));
        return;
    }
    if (exists.value(0).toInt() > 0) {
        QMessageBox::warning(this, QStringLiteral("Inscription"),
                             QStringLiteral("Ce nom d'utilisateur existe deja."));
        return;
    }

    QSqlQuery ins(db);
    ins.prepare(QStringLiteral("INSERT INTO APP_USERS (USERNAME, PASSWORD, ROLE, EMAIL) VALUES (:u, :p, :r, :e)"));
    ins.bindValue(QStringLiteral(":u"), username);
    ins.bindValue(QStringLiteral(":p"), hashPassword(password));
    ins.bindValue(QStringLiteral(":r"), QStringLiteral("USER"));
    ins.bindValue(QStringLiteral(":e"), email);
    if (!ins.exec()) {
        QMessageBox::critical(this, QStringLiteral("Inscription"),
                              QStringLiteral("Creation du compte impossible:\n%1")
                                  .arg(ins.lastError().text().trimmed()));
        return;
    }

    if (ui->lineEdit_5)
        ui->lineEdit_5->setText(username);
    if (ui->lineEdit_6) {
        ui->lineEdit_6->setText(password);
        ui->lineEdit_6->setFocus();
    }

    QMessageBox::information(this, QStringLiteral("Inscription"),
                             QStringLiteral("Compte cree avec succes. Vous pouvez maintenant vous connecter."));
}

void MainWindow::onForgotPasswordRequested()
{
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Mot de passe oublie"),
                             QStringLiteral("Connexion Oracle indisponible. Verifiez le DSN ODBC projet_cuir."));
        return;
    }

    QString empErr;
    if (!Employe::ensureSchema(&empErr)) {
        QMessageBox::critical(this, QStringLiteral("Mot de passe oublie"),
                              QStringLiteral("Impossible de preparer la table EMPLOYES:\n%1").arg(empErr));
        return;
    }

    ForgotPasswordDialog dlg(db, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    if (ui->lineEdit_5)
        ui->lineEdit_5->clear();
    if (ui->lineEdit_6) {
        ui->lineEdit_6->clear();
        ui->lineEdit_6->setFocus();
    }
    QMessageBox::information(this, QStringLiteral("Mot de passe oublie"),
                             QStringLiteral("Mot de passe employe reinitialise avec succes."));
}

void MainWindow::on_btnMpremieres_2_clicked()
{
    applyAuthenticationUiState(false);
    if (ui->lineEdit_5)
        ui->lineEdit_5->clear();
    if (ui->lineEdit_6)
        ui->lineEdit_6->clear();
    if (ui->headerUserLabel)
        ui->headerUserLabel->setText(QStringLiteral("Administrateur"));
    if (m_bodyStack)
        m_bodyStack->setCurrentIndex(1);
    ui->contentStack->setCurrentIndex(0);
    setActiveButton(ui->btnAccueil);
    if (ui->lineEdit_5)
        ui->lineEdit_5->setFocus();
}

// ------------------- CRUD Client -------------------

void MainWindow::installClientPageResponsiveLayout()
{
    QWidget *page = ui->pageClient;
    if (!page)
        return;
    if (page->property("clientLayoutInstalled").toBool())
        return;

    QVBoxLayout *mainLay = qobject_cast<QVBoxLayout *>(page->layout());
    if (!mainLay) {
        mainLay = new QVBoxLayout(page);
    }
    mainLay->setContentsMargins(16, 14, 16, 12);
    mainLay->setSpacing(12);

    auto *topNavCard = new QFrame(page);
    topNavCard->setObjectName(QStringLiteral("clientTopNavBar"));
    auto *topNavLay = new QHBoxLayout(topNavCard);
    topNavLay->setContentsMargins(12, 8, 12, 8);
    topNavLay->setSpacing(10);

    const struct {
        QString text;
        QString objectName;
        QStyle::StandardPixmap icon;
    } navItems[] = {
        {QStringLiteral("Export"), QStringLiteral("clientTopExportBtn"), QStyle::SP_DialogSaveButton},
        {QStringLiteral("WhatsApp"), QStringLiteral("clientTopWhatsAppBtn"), QStyle::SP_MessageBoxInformation},
        {QStringLiteral("Avis"), QStringLiteral("clientTopAvisBtn"), QStyle::SP_DialogHelpButton},
        {QStringLiteral("Analyze Clients"), QStringLiteral("clientTopAnalyzeBtn"), QStyle::SP_ComputerIcon},
    };
    for (const auto &item : navItems) {
        auto *btn = new QPushButton(item.text, topNavCard);
        btn->setObjectName(QStringLiteral("clientTopNavButton"));
        btn->setProperty("clientActionName", item.objectName);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(style()->standardIcon(item.icon));
        btn->setIconSize(QSize(16, 16));
        btn->setMinimumHeight(34);
        topNavLay->addWidget(btn);
    }
    if (topNavLay->count() > 0) {
        if (QPushButton *first = qobject_cast<QPushButton *>(topNavLay->itemAt(0)->widget()))
            first->setChecked(true);
    }
    auto *searchTop = new QLineEdit(topNavCard);
    searchTop->setObjectName(QStringLiteral("clientTopSearchEdit"));
    searchTop->setMinimumHeight(34);
    searchTop->setMinimumWidth(260);
    searchTop->setMaximumWidth(360);
    searchTop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchTop->setPlaceholderText(QStringLiteral("Rechercher..."));
    searchTop->setStyleSheet(QStringLiteral(
        "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
        "QLineEdit:focus { border-color: #b8c4d3; }"));

    auto *searchBtnTop = new QPushButton(QStringLiteral("Rechercher"), topNavCard);
    searchBtnTop->setObjectName(QStringLiteral("clientTopSearchBtn"));
    searchBtnTop->setCursor(Qt::PointingHandCursor);
    searchBtnTop->setMinimumSize(116, 34);
    searchBtnTop->setMaximumWidth(116);
    searchBtnTop->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  border: 1px solid transparent;"
        "  border-radius: 8px;"
        "  background: #f3f5f8;"
        "  color: #1f2937;"
        "  font-weight: 600;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover { background: #edf2f8; border-color: #b8c4d3; color: #000000; }"
        "QPushButton:pressed { background: #e5ebf3; border-color: #aab8ca; color: #000000; }"));

    auto *resetBtnTop = new QPushButton(QStringLiteral("Reset"), topNavCard);
    resetBtnTop->setObjectName(QStringLiteral("clientTopResetBtn"));
    resetBtnTop->setCursor(Qt::PointingHandCursor);
    resetBtnTop->setMinimumSize(116, 34);
    resetBtnTop->setMaximumWidth(116);
    resetBtnTop->setStyleSheet(searchBtnTop->styleSheet());

    auto *filterTop = new QComboBox(topNavCard);
    filterTop->setObjectName(QStringLiteral("clientTopFilterCombo"));
    filterTop->setMinimumSize(130, 34);
    filterTop->addItem(QStringLiteral("Filtre: Tous"), -1);
    filterTop->addItem(QStringLiteral("ID"), 0);
    filterTop->addItem(QStringLiteral("Nom"), 1);
    filterTop->addItem(QStringLiteral("Prenom"), 2);
    filterTop->addItem(QStringLiteral("Email"), 3);
    filterTop->addItem(QStringLiteral("Telephone"), 4);
    filterTop->addItem(QStringLiteral("Categorie"), 7);

    topNavLay->addStretch(1);
    topNavLay->addWidget(filterTop);
    topNavLay->addWidget(searchTop, 1);
    topNavLay->addWidget(searchBtnTop);
    topNavLay->addWidget(resetBtnTop);
    mainLay->addWidget(topNavCard, 0);

    auto *contentRow = new QHBoxLayout();
    contentRow->setSpacing(14);
    mainLay->addLayout(contentRow, 1);

    auto *leftPanel = new QWidget(page);
    leftPanel->setObjectName(QStringLiteral("clientFormCard"));
    leftPanel->setMinimumWidth(300);
    leftPanel->setMaximumWidth(380);
    leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *leftCol = new QVBoxLayout(leftPanel);
    leftCol->setObjectName(QStringLiteral("clientLeftColumn"));
    leftCol->setContentsMargins(12, 12, 12, 12);
    leftCol->setSpacing(10);
    contentRow->addWidget(leftPanel, 0);

    auto *rightPanel = new QWidget(page);
    rightPanel->setObjectName(QStringLiteral("clientTableZone"));
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightCol = new QVBoxLayout(rightPanel);
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setSpacing(10);
    contentRow->addWidget(rightPanel, 1);

    if (ui->layoutWidget) {
        ui->layoutWidget->setParent(rightPanel);
        rightCol->addWidget(ui->layoutWidget, 0);
    }

    if (ui->clientTable) {
        ui->clientTable->setParent(rightPanel);
        ui->clientTable->setMinimumSize(520, 320);
        ui->clientTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        rightCol->addWidget(ui->clientTable, 1);
    }

    if (ui->employeeFormBox_3) {
        ui->employeeFormBox_3->setParent(leftPanel);
        leftCol->addWidget(ui->employeeFormBox_3, 0, Qt::AlignTop);
    }

    // Panneau « Analyse et suivi clients » (chatGroupBox_3) : non affiche — champs gardes pour la logique metier (masques).
    if (ui->horizontalLayoutWidget) {
        ui->horizontalLayoutWidget->setParent(page);
        ui->horizontalLayoutWidget->hide();
    }

    if (ui->lineEditSearch_3) {
        ui->lineEditSearch_3->setMinimumWidth(180);
        ui->lineEditSearch_3->setMaximumWidth(16777215);
        ui->lineEditSearch_3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    if (ui->layoutWidget)
        ui->layoutWidget->hide();
    page->setProperty("clientLayoutInstalled", true);
}

void MainWindow::applyFormStyle(QWidget *formRoot)
{
    if (!formRoot)
        return;
    formRoot->setStyleSheet(QStringLiteral(
        "QGroupBox {"
        "  background: #ffffff;"
        "  border: 1px solid #dde4ee;"
        "  border-radius: 12px;"
        "}"
        "QLabel { color: #1f2937; font-weight: 600; }"
        "QLineEdit, QComboBox, QDateEdit, QDateTimeEdit {"
        "  min-height: 34px;"
        "  border: 1px solid #cfd8e6;"
        "  border-radius: 8px;"
        "  padding: 0 10px;"
        "  background: #ffffff;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDateTimeEdit:focus { border-color: #b8c4d3; }"
        "QLineEdit[invalid=\"true\"], QComboBox[invalid=\"true\"],"
        "QDateEdit[invalid=\"true\"], QDateTimeEdit[invalid=\"true\"] { border: 1px solid #c62828; }"));
}

void MainWindow::setupClientFicheScrollAndHeader()
{
    if (!ui->pageClient || !ui->employeeFormBox_3 || !ui->formOuterLayout_3)
        return;
    if (ui->employeeFormBox_3->property("clientFicheModernized").toBool())
        return;

    ui->employeeFormBox_3->setFlat(true);
    ui->employeeFormBox_3->setTitle(QString());
    applyFormStyle(ui->employeeFormBox_3);

    if (ui->formRow1_3) {
        ui->formOuterLayout_3->removeItem(ui->formRow1_3);
        delete ui->formRow1_3;
        ui->formRow1_3 = nullptr;
    }
    if (ui->formRow2_3) {
        ui->formOuterLayout_3->removeItem(ui->formRow2_3);
        delete ui->formRow2_3;
        ui->formRow2_3 = nullptr;
    }
    // Masque les anciens labels de la grille UI Designer (sinon ils se superposent en haut à gauche).
    const QList<QWidget *> legacyClientLabels = {
        ui->labelCIN_3, ui->labelNom_3, ui->labelPrenom_3, ui->labelSexe_3, ui->labelSalaire_3,
        ui->labelDateEmbauche_3, ui->labelTelephone_3, ui->labelPoste_3, ui->labelAdresse_3, ui->labelEmail_3
    };
    for (QWidget *w : legacyClientLabels) {
        if (w)
            w->hide();
    }

    auto *titleLab = new QLabel(QStringLiteral("Fiche client"), ui->employeeFormBox_3);
    titleLab->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #000000; padding: 2px 4px;"));
    ui->formOuterLayout_3->insertWidget(0, titleLab);

    auto *formWrap = new QWidget(ui->employeeFormBox_3);
    auto *formLay = new QFormLayout(formWrap);
    formLay->setContentsMargins(4, 2, 4, 4);
    formLay->setHorizontalSpacing(12);
    formLay->setVerticalSpacing(12);
    formLay->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLay->setFormAlignment(Qt::AlignTop);
    formLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    ui->lineEdit_IDC->setPlaceholderText(QStringLiteral("Entrez votre CIN"));
    ui->lineEdit_nomC->setPlaceholderText(QStringLiteral("Entrez votre nom"));
    ui->lineEdit_PrenomC->setPlaceholderText(QStringLiteral("Entrez votre prenom"));
    ui->lineEdit_adresseC->setPlaceholderText(QStringLiteral("Entrez votre adresse"));
    ui->lineEdit_telephoneC->setPlaceholderText(QStringLiteral("Numero de telephone"));
    ui->lineEdit_emailC->setPlaceholderText(QStringLiteral("Entrez votre email"));
    ui->lineEdit_remiseC->setPlaceholderText(QStringLiteral("Remise accordee (%)"));
    ui->lineEdit_canalC->setPlaceholderText(QStringLiteral("Canal d'acquisition"));
    ui->lineEdit_modeC->setPlaceholderText(QStringLiteral("Mode de paiement prefere"));

    formLay->addRow(new QLabel(QStringLiteral("CIN"), formWrap), ui->lineEdit_IDC);
    formLay->addRow(new QLabel(QStringLiteral("Nom"), formWrap), ui->lineEdit_nomC);
    formLay->addRow(new QLabel(QStringLiteral("Prenom"), formWrap), ui->lineEdit_PrenomC);
    formLay->addRow(new QLabel(QStringLiteral("Adresse"), formWrap), ui->lineEdit_adresseC);
    formLay->addRow(new QLabel(QStringLiteral("Telephone"), formWrap), ui->lineEdit_telephoneC);
    formLay->addRow(new QLabel(QStringLiteral("E-mail"), formWrap), ui->lineEdit_emailC);
    formLay->addRow(new QLabel(QStringLiteral("Statut"), formWrap), ui->comboBox_statutC);
    formLay->addRow(new QLabel(QStringLiteral("Remise"), formWrap), ui->lineEdit_remiseC);
    formLay->addRow(new QLabel(QStringLiteral("Canal"), formWrap), ui->lineEdit_canalC);
    formLay->addRow(new QLabel(QStringLiteral("Paiement"), formWrap), ui->lineEdit_modeC);

    ui->formOuterLayout_3->insertWidget(1, formWrap);

    auto *sa = new QScrollArea(ui->pageClient);
    m_clientFormScrollArea = sa;
    sa->setObjectName(QStringLiteral("clientFormScrollArea"));
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::StyledPanel);
    sa->setFrameShadow(QFrame::Plain);
    sa->setStyleSheet(QStringLiteral(
        "QScrollArea { border: 1px solid rgb(230, 220, 200); border-radius: 6px; background: rgb(255, 255, 255); }"));
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->employeeFormBox_3->setParent(sa);
    sa->setWidget(ui->employeeFormBox_3);
    ui->employeeFormBox_3->setMaximumHeight(16777215);
    ui->employeeFormBox_3->setMinimumWidth(200);
    ui->employeeFormBox_3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    if (QVBoxLayout *leftCol = ui->pageClient->findChild<QVBoxLayout *>(QStringLiteral("clientLeftColumn"))) {
        // Nettoie les spacers hérités pour éviter le grand vide au-dessus de la fiche.
        for (int i = leftCol->count() - 1; i >= 0; --i) {
            QLayoutItem *it = leftCol->itemAt(i);
            if (it && it->spacerItem()) {
                QLayoutItem *removed = leftCol->takeAt(i);
                delete removed;
            }
        }
        leftCol->insertWidget(0, sa, 1);
        leftCol->addStretch(0);
    } else {
        const QRect g = ui->employeeFormBox_3->geometry();
        sa->setGeometry(g.isValid() ? g : QRect(20, 420, 700, 210));
        sa->raise();
    }
    sa->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sa->raise();
    ui->employeeFormBox_3->setProperty("clientFicheModernized", true);
}

void MainWindow::setupClientAnalyseSuiviScroll()
{
    // Colonne droite clients retiree de l'affichage (voir installClientPageResponsiveLayout).
}

void MainWindow::setupClientValidators()
{
    ui->lineEdit_IDC->setValidator(new QIntValidator(1, 999999999, this));
    ui->lineEdit_remiseC->setValidator(new QDoubleValidator(0.0, 100.0, 2, this));
    ui->lineEdit_telephoneC->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\+?[0-9 ]{0,15}$"), this));
    ui->lineEdit_emailC->setValidator(new QRegularExpressionValidator(QRegularExpression("^[A-Za-z0-9._%+-]*@[A-Za-z0-9.-]*\\.?[A-Za-z]{0,}$"), this));
    ui->lineEdit_limiteCreditSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->lineEdit_montantCommandeSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
    ui->lineEdit_montantPaiementSeg->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, this));
}

void MainWindow::setupClientUiEnhancements()
{
    // Memes dimensions / espacement que Ajouter–Modifier–Supprimer (mainwindow.ui).
    constexpr int kClientBarBtnH = 32;
    constexpr int kClientBarBtnW = 90;

    const QString btnClientTools = QStringLiteral(
        "QPushButton {"
        "  background-color: rgb(88, 41, 0);"
        "  color: rgb(248, 241, 227);"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  padding: 6px 10px;"
        "}"
        "QPushButton:hover { background-color: rgb(110, 55, 10); }");

    const auto prepareToolButton = [&](QPushButton *b) {
        b->setMinimumSize(kClientBarBtnW, kClientBarBtnH);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(btnClientTools);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    };

    QPushButton *topExportBtn = nullptr;
    QPushButton *topWhatsAppBtn = nullptr;
    QPushButton *topAvisBtn = nullptr;
    QPushButton *topAnalyzeBtn = nullptr;
    if (ui->pageClient) {
        const auto topBtns = ui->pageClient->findChildren<QPushButton *>(QStringLiteral("clientTopNavButton"));
        for (QPushButton *b : topBtns) {
            const QString action = b->property("clientActionName").toString();
            if (action == QStringLiteral("clientTopExportBtn"))
                topExportBtn = b;
            else if (action == QStringLiteral("clientTopSmsBtn")) {
                // Ancien libellé SMS (Twilio supprimé) : masquer si encore présent.
                b->hide();
                b->setEnabled(false);
            } else if (action == QStringLiteral("clientTopWhatsAppBtn"))
                topWhatsAppBtn = b;
            else if (action == QStringLiteral("clientTopAvisBtn"))
                topAvisBtn = b;
            else if (action == QStringLiteral("clientTopAnalyzeBtn"))
                topAnalyzeBtn = b;
        }
    }

    if (topExportBtn) {
        topExportBtn->setToolTip(QStringLiteral("Exporter tout le tableau des clients vers un fichier Excel (.xls)."));
        connect(topExportBtn, &QPushButton::clicked, this, &MainWindow::onExporterClientsClicked);
    }
    if (topWhatsAppBtn) {
        m_clientWhatsAppButton = topWhatsAppBtn;
        m_clientWhatsAppButton->setToolTip(
            QStringLiteral("WhatsApp (Twilio) : rappel livraison, code accueil, relance, ou lien questionnaire. "
                            "Configurer TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN et TWILIO_WHATSAPP_FROM."));
        connect(m_clientWhatsAppButton, &QPushButton::clicked, this, &MainWindow::onClientWhatsAppClicked);
    } else if (ui->pageClient) {
        m_clientWhatsAppButton = new QPushButton(QStringLiteral("WhatsApp"), ui->pageClient);
        prepareToolButton(m_clientWhatsAppButton);
        m_clientWhatsAppButton->setToolTip(
            QStringLiteral("WhatsApp (Twilio) : rappels, code accueil, relance ou questionnaire. "
                            "Variables TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN, TWILIO_WHATSAPP_FROM."));
        connect(m_clientWhatsAppButton, &QPushButton::clicked, this, &MainWindow::onClientWhatsAppClicked);
    }
    if (topAvisBtn) {
        m_clientSurveySmsButton = topAvisBtn;
        m_clientSurveySmsButton->setToolTip(
            QStringLiteral("Envoie le lien du questionnaire (LEATHER_SURVEY_URL_TEMPLATE ou reglage ClientNotif)."));
        connect(m_clientSurveySmsButton, &QPushButton::clicked, this, &MainWindow::onClientSurveySmsClicked);
    } else {
        m_clientSurveySmsButton = new QPushButton(QStringLiteral("Avis"), ui->pageClient);
        prepareToolButton(m_clientSurveySmsButton);
        connect(m_clientSurveySmsButton, &QPushButton::clicked, this, &MainWindow::onClientSurveySmsClicked);
    }
    if (topAnalyzeBtn) {
        topAnalyzeBtn->setToolTip(
            QStringLiteral("Client Scoring : lit la base, appelle Numverify si besoin, remplit les colonnes SCORE et STATUS.\n"
                           "Score 0-100 : +40 (achat <=3 mois), +20 (3-12 mois), +30 (tel valide), -100 si bloque.\n"
                           "Statuts : VIP 80-100, Active 50-79, Medium 20-49, Inactive <20, Blocked si bloque.\n"
                           "Voir aussi l infobulle sur les en-tetes SCORE / STATUS du tableau."));
        connect(topAnalyzeBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeClientsClicked);
    }

    if (ui->formBtnLayout_3) {
        ui->formBtnLayout_3->setContentsMargins(0, 4, 0, 4);
        ui->formBtnLayout_3->setSpacing(10);
        ui->formBtnLayout_3->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        for (int i = ui->formBtnLayout_3->count() - 1; i >= 0; --i) {
            QLayoutItem *lit = ui->formBtnLayout_3->itemAt(i);
            if (lit && lit->spacerItem()) {
                QLayoutItem *removed = ui->formBtnLayout_3->takeAt(i);
                delete removed;
                continue;
            }
        }
    }

    const auto sizeCrudClientButton = [&](QPushButton *b) {
        if (!b)
            return;
        b->setMinimumSize(kClientBarBtnW, kClientBarBtnH);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    };
    sizeCrudClientButton(ui->pushButton_ajouter);
    sizeCrudClientButton(ui->pushButton_modifier);
    sizeCrudClientButton(ui->pushButton_supprimer);
    if (ui->pushButton_ajouter) {
        applyUnifiedAddButtonStyle(ui->pushButton_ajouter);
        ui->pushButton_ajouter->setText(QStringLiteral("Ajouter"));
    }
    if (!m_clientCancelEditButton && ui->formBtnLayout_3 && ui->pageClient) {
        m_clientCancelEditButton = new QPushButton(QStringLiteral("Annuler"), ui->pageClient);
        m_clientCancelEditButton->setCursor(Qt::PointingHandCursor);
        m_clientCancelEditButton->setMinimumSize(kClientBarBtnW, kClientBarBtnH);
        m_clientCancelEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_clientCancelEditButton->setStyleSheet(QStringLiteral(
            "QPushButton { background:#ffffff; color:#5d2e06; border:1px solid #d8c8b2; border-radius:8px; font-weight:600; padding:6px 12px; }"
            "QPushButton:hover { background:#fff4e9; border-color:#c08a5b; }"));
        m_clientCancelEditButton->hide();
        ui->formBtnLayout_3->addWidget(m_clientCancelEditButton);
        connect(m_clientCancelEditButton, &QPushButton::clicked, this, [this]() {
            exitClientEditMode(true);
            fillClientFormFromSelectedRow();
        });
    }
    if (ui->formBtnLayout_3) {
        if (ui->pushButton_modifier) {
            ui->pushButton_modifier->hide();
            ui->formBtnLayout_3->removeWidget(ui->pushButton_modifier);
        }
        if (ui->pushButton_supprimer) {
            ui->pushButton_supprimer->hide();
            ui->formBtnLayout_3->removeWidget(ui->pushButton_supprimer);
        }
    }

    const QString clientPageStyle = QStringLiteral(
        "#pageClient { background: #ffffff; }"
        "#clientTopNavBar { background: #ffffff; border: 1px solid #e6eaf0; border-radius: 10px; }"
        "#clientTopNavButton {"
        "  border: 1px solid transparent;"
        "  border-radius: 8px;"
        "  background: #f3f5f8;"
        "  color: #1f2937;"
        "  font-weight: 600;"
        "  padding: 6px 12px;"
        "}"
        "#clientTopNavButton:hover {"
        "  background: #edf2f8;"
        "  border-color: #b8c4d3;"
        "  color: #000000;"
        "}"
        "#clientTopNavButton:checked { background: #e5ebf3; border-color: #aab8ca; color: #000000; }"
        "#clientFormCard, #clientTableZone {"
        "  background: #ffffff;"
        "  border: 1px solid #e7ecf2;"
        "  border-radius: 12px;"
        "}"
        "#layoutWidget QLineEdit, #layoutWidget QComboBox, #clientFormScrollArea QLineEdit, #clientFormScrollArea QComboBox {"
        "  min-height: 30px;"
        "  border: 1px solid #d5dbe5;"
        "  border-radius: 6px;"
        "  padding: 0 8px;"
        "  background: #ffffff;"
        "}"
        "#layoutWidget QLineEdit:focus, #layoutWidget QComboBox:focus,"
        "#clientFormScrollArea QLineEdit:focus, #clientFormScrollArea QComboBox:focus { border-color: #b8c4d3; }"
        "#clientTable {"
        "  border: 1px solid #dce3ec;"
        "  border-radius: 8px;"
        "  gridline-color: #edf1f6;"
        "  selection-background-color: #ead7c3;"
        "  selection-color: #1f2937;"
        "  alternate-background-color: #f9fbff;"
        "}"
        "#clientTable::item { padding: 6px; }"
        "#clientTable QHeaderView::section {"
        "  background-color: #582900;"
        "  color: #ffffff;"
        "  font-weight: 600;"
        "  padding: 6px;"
        "  border: none;"
        "}"
        "#clientFormScrollArea { border-radius: 10px; }");
    if (ui->pageClient)
        ui->pageClient->setStyleSheet(ui->pageClient->styleSheet() + clientPageStyle);

    if (ui->clientTable) {
        ui->clientTable->setAlternatingRowColors(true);
        ui->clientTable->setShowGrid(true);
        ui->clientTable->verticalHeader()->setVisible(false);
        ui->clientTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    }

    if (ui->btnRechercher_3) {
        ui->btnRechercher_3->setMinimumWidth(116);
        ui->btnRechercher_3->setMaximumWidth(116);
        ui->btnRechercher_3->setMinimumHeight(34);
        ui->btnRechercher_3->setCursor(Qt::PointingHandCursor);
        ui->btnRechercher_3->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  background: #f3f5f8;"
            "  color: #1f2937;"
            "  font-weight: 600;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover { background: #e9eef7; border-color: #d7e1f0; }"
            "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }"));
        ui->btnRechercher_3->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
        ui->btnRechercher_3->setIconSize(QSize(14, 14));
    }
    if (ui->pushButton_resetFiltres) {
        ui->pushButton_resetFiltres->setMinimumWidth(116);
        ui->pushButton_resetFiltres->setMaximumWidth(116);
        ui->pushButton_resetFiltres->setMinimumHeight(34);
        ui->pushButton_resetFiltres->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        ui->pushButton_resetFiltres->setCursor(Qt::PointingHandCursor);
        ui->pushButton_resetFiltres->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  background: #f3f5f8;"
            "  color: #1f2937;"
            "  font-weight: 600;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover { background: #e9eef7; border-color: #d7e1f0; }"
            "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }"));
    }

    if (ui->pageClient) {
        if (auto *topSearchEdit = ui->pageClient->findChild<QLineEdit *>(QStringLiteral("clientTopSearchEdit"))) {
            topSearchEdit->setStyleSheet(QStringLiteral(
                "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QLineEdit:focus { border-color: #c08a5b; }"));
            connect(topSearchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
                if (ui->lineEditSearch_3)
                    ui->lineEditSearch_3->setText(text);
            });
            connect(topSearchEdit, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_3_clicked);
        }

        const QString topSearchBtnStyle = QStringLiteral(
            "QPushButton {"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  background: #f3f5f8;"
            "  color: #1f2937;"
            "  font-weight: 600;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
            "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }");

        if (auto *topSearchBtn = ui->pageClient->findChild<QPushButton *>(QStringLiteral("clientTopSearchBtn"))) {
            topSearchBtn->setStyleSheet(topSearchBtnStyle);
            topSearchBtn->setMinimumWidth(116);
            connect(topSearchBtn, &QPushButton::clicked, this, &MainWindow::on_btnRechercher_3_clicked);
        }
        if (auto *topFilterCombo = ui->pageClient->findChild<QComboBox *>(QStringLiteral("clientTopFilterCombo"))) {
            topFilterCombo->setStyleSheet(QStringLiteral(
                "QComboBox { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; min-height: 34px; }"
                "QComboBox:focus { border-color: #c08a5b; }"));
            connect(topFilterCombo, &QComboBox::currentIndexChanged, this, [this](int) {
                on_btnRechercher_3_clicked();
            });
        }
        if (auto *topResetBtn = ui->pageClient->findChild<QPushButton *>(QStringLiteral("clientTopResetBtn"))) {
            topResetBtn->setStyleSheet(topSearchBtnStyle);
            topResetBtn->setMinimumWidth(116);
            connect(topResetBtn, &QPushButton::clicked, this, &MainWindow::on_pushButton_resetFiltres_clicked);
        }
    }
    if (ui->pushButton_ajouter)
        ui->pushButton_ajouter->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    if (ui->pushButton_modifier)
        ui->pushButton_modifier->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    if (ui->pushButton_supprimer)
        ui->pushButton_supprimer->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    if (topExportBtn)
        topExportBtn->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    if (m_clientWhatsAppButton)
        m_clientWhatsAppButton->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    if (m_clientSurveySmsButton)
        m_clientSurveySmsButton->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));

    connect(ui->pushButton_verifierCommandeSeg, &QPushButton::clicked, this, &MainWindow::onVerifierCommandeClicked);
    connect(ui->pushButton_paiementSeg, &QPushButton::clicked, this, &MainWindow::onEnregistrerPaiementClicked);
    connect(ui->pushButton_historiqueSeg, &QPushButton::clicked, this, &MainWindow::onVoirHistoriqueClicked);
    // btnRechercher_3 -> on_btnRechercher_3_clicked est deja connecte par connectSlotsByName (setupUi).
    connect(ui->lineEditSearch_3, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_3_clicked);
    connect(ui->comboBoxFiltreCategorie, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });
    connect(ui->comboBoxFiltreStatut, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });
    connect(ui->comboBoxFiltreRisque, &QComboBox::currentTextChanged, this, [this](const QString &) { on_btnRechercher_3_clicked(); });

    // Masquer le bloc « Gestion du crédit client » (séparateur, saisies, boutons, labels risque/crédit).
    // lineEdit_limiteCreditSeg reste rempli depuis le tableau (invisible) pour le recalcul segmentation.
    const QList<QWidget *> creditWidgetsToHide = {
        static_cast<QWidget *>(ui->lineSepSegCredit),
        static_cast<QWidget *>(ui->labelCreditTitle),
        static_cast<QWidget *>(ui->lineEdit_montantCommandeSeg),
        static_cast<QWidget *>(ui->lineEdit_montantPaiementSeg),
        static_cast<QWidget *>(ui->lineEdit_notePaiementSeg),
        static_cast<QWidget *>(ui->pushButton_verifierCommandeSeg),
        static_cast<QWidget *>(ui->pushButton_paiementSeg),
        static_cast<QWidget *>(ui->pushButton_historiqueSeg),
        static_cast<QWidget *>(ui->labelRisqueSeg),
        static_cast<QWidget *>(ui->labelCreditRestantSeg),
        static_cast<QWidget *>(ui->labelInsightsTitleSeg),
        static_cast<QWidget *>(ui->textEditInsightsSeg),
    };
    for (QWidget *w : creditWidgetsToHide) {
        if (w)
            w->hide();
    }
    if (ui->lineEdit_limiteCreditSeg)
        ui->lineEdit_limiteCreditSeg->hide();

    setupClientAnalyseSuiviScroll();
}

void MainWindow::loadClients()
{
    if (m_clientEditMode)
        exitClientEditMode(false);
    QList<ClientData> clients;
    QString err;
    if (!Client::chargerTous(clients, &err)) {
        QMessageBox::warning(this, QStringLiteral("Clients"), QStringLiteral("Chargement impossible.\n") + err);
        return;
    }
    ui->clientTable->setRowCount(0);
    for (const ClientData &c : clients) {
        const int row = ui->clientTable->rowCount();
        ui->clientTable->insertRow(row);
        ui->clientTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));
        ui->clientTable->setItem(row, 1, new QTableWidgetItem(c.nom));
        ui->clientTable->setItem(row, 2, new QTableWidgetItem(c.prenom));
        ui->clientTable->setItem(row, 3, new QTableWidgetItem(c.email));
        ui->clientTable->setItem(row, 4, new QTableWidgetItem(c.telephone));
        ui->clientTable->setItem(row, 5, new QTableWidgetItem(c.adresse));
        ui->clientTable->setItem(row, 6, new QTableWidgetItem(c.statutClient));
        ui->clientTable->setItem(row, 7, new QTableWidgetItem(c.categorie));
        ui->clientTable->setItem(row, 8, new QTableWidgetItem(QString::number(c.remiseAccordee, 'f', 2)));
        ui->clientTable->setItem(row, 9, new QTableWidgetItem(QString::number(c.totalAchats, 'f', 2)));
        ui->clientTable->setItem(row, 10, new QTableWidgetItem(QStringLiteral("-")));
        ui->clientTable->setItem(row, 11, new QTableWidgetItem(QStringLiteral("Not analyzed")));
        ui->clientTable->setItem(row, 12, new QTableWidgetItem(QString::number(c.soldeCreditUtilise, 'f', 2)));

        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), ui->clientTable);
        deleteBtn->setToolTip(QStringLiteral("Supprimer ce client"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            ui->clientTable->setCurrentCell(row, 0);
            on_pushButton_supprimer_clicked();
        });
        ui->clientTable->setCellWidget(row, 13, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), ui->clientTable);
        updateBtn->setToolTip(QStringLiteral("Modifier ce client"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(updateBtn);
        connect(updateBtn, &QPushButton::clicked, this, [this, row]() {
            ui->clientTable->setCurrentCell(row, 0);
            fillClientFormFromSelectedRow();
            beginClientEditMode();
        });
        ui->clientTable->setCellWidget(row, 14, updateBtn);
    }
    onStatisticsDataChanged();
}

bool MainWindow::isNumverifyPhoneValid(const QString &phone, bool *isValid, QString *errorMessage) const
{
    if (isValid)
        *isValid = false;
    if (phone.trimmed().isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Numero vide.");
        return false;
    }
    if (!m_networkAccessManager) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Service reseau indisponible.");
        return false;
    }

    QString apiKey = qEnvironmentVariable("NUMVERIFY_API_KEY").trimmed();
    if (apiKey.isEmpty())
        apiKey = qEnvironmentVariable("NUMVERIFY-KEY").trimmed();
    if (apiKey.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Cle absente: definissez NUMVERIFY_API_KEY ou NUMVERIFY-KEY.");
        return false;
    }

    const QString numberForApi = normalizePhoneForNumverify(phone);
    if (numberForApi.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Numero invalide apres normalisation.");
        return false;
    }

    QUrl url(QStringLiteral("https://apilayer.net/api/validate"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("access_key"), apiKey);
    query.addQueryItem(QStringLiteral("number"), numberForApi);
    if (numberForApi.startsWith(QStringLiteral("+216")))
        query.addQueryItem(QStringLiteral("country_code"), QStringLiteral("TN"));
    url.setQuery(query);

    constexpr int kMaxNumverifyAttempts = 6;
    for (int attempt = 0; attempt < kMaxNumverifyAttempts; ++attempt) {
        numverifyThrottleBeforeNextRequest();

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("RoyalLeatherHouse/1.0 (Qt)"));
        QNetworkReply *reply = m_networkAccessManager->get(req);
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &loop, [&]() {
            if (reply && reply->isRunning())
                reply->abort();
            loop.quit();
        });
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timeout.start(8000);
        loop.exec();

        numverifyMarkRequestEnded();

        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray payload = reply->readAll();
        const QString errStr = reply->errorString();
        const bool looksLike429 = (httpStatus == 429)
                                  || errStr.contains(QStringLiteral("429"), Qt::CaseInsensitive)
                                  || errStr.contains(QStringLiteral("Too Many Requests"), Qt::CaseInsensitive);

        if (looksLike429) {
            bool raOk = false;
            const int raSec = reply->rawHeader(QByteArrayLiteral("Retry-After")).toInt(&raOk);
            reply->deleteLater();
            if (attempt + 1 < kMaxNumverifyAttempts) {
                int backoffMs = 2000 * (1 << attempt);
                if (raOk && raSec > 0)
                    backoffMs = qMax(backoffMs, raSec * 1000);
                QThread::msleep(static_cast<unsigned long>(qMin(backoffMs, 30000)));
                continue;
            }
            if (errorMessage) {
                *errorMessage = numverifyRedactSecrets(
                    QStringLiteral("Quota / limite de debit Numverify (HTTP 429). Augmentez "
                                   "NUMVERIFY_MIN_INTERVAL_MS (ms) ou attendez. Dernier message : %1")
                        .arg(errStr));
            }
            return false;
        }

        if (reply->error() != QNetworkReply::NoError) {
            if (errorMessage) {
                *errorMessage = numverifyRedactSecrets(
                    QStringLiteral("%1 (HTTP %2)")
                        .arg(errStr)
                        .arg(httpStatus > 0 ? QString::number(httpStatus) : QStringLiteral("-")));
            }
            reply->deleteLater();
            return false;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (!doc.isObject()) {
            if (errorMessage) {
                const QString snippet = QString::fromUtf8(payload.left(160));
                *errorMessage = numverifyRedactSecrets(
                    QStringLiteral("Reponse Numverify invalide (pas JSON). HTTP %1\n%2")
                        .arg(httpStatus > 0 ? QString::number(httpStatus) : QStringLiteral("-"), snippet));
            }
            reply->deleteLater();
            return false;
        }

        const QJsonObject obj = doc.object();
        if (obj.value(QStringLiteral("success")).isBool() && !obj.value(QStringLiteral("success")).toBool()) {
            const QJsonObject errObj = obj.value(QStringLiteral("error")).toObject();
            const int errCode = errObj.value(QStringLiteral("code")).toInt();
            const QString errType = errObj.value(QStringLiteral("type")).toString();
            const QString errInfo = errObj.value(QStringLiteral("info")).toString();
            if (errorMessage) {
                *errorMessage = numverifyRedactSecrets(
                    QStringLiteral("API Numverify: code %1 | %2 | %3").arg(errCode).arg(errType).arg(errInfo));
            }
            reply->deleteLater();
            return false;
        }

        if (isValid)
            *isValid = obj.value(QStringLiteral("valid")).toBool(false);
        reply->deleteLater();
        return true;
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Numverify: echec inattendu apres plusieurs tentatives.");
    return false;
}

void MainWindow::applyClientScoringResults(const QHash<int, int> &scoresByClientId,
                                           const QHash<int, QString> &statusByClientId)
{
    if (!ui->clientTable)
        return;

    // Couleur de fond de toute la ligne selon STATUS (legendes identiques aux tooltips des colonnes).
    const auto statusColor = [](const QString &status) {
        if (status == QStringLiteral("VIP"))
            return QColor(QStringLiteral("#d8b4fe"));
        if (status == QStringLiteral("Active"))
            return QColor(QStringLiteral("#bbf7d0"));
        if (status == QStringLiteral("Medium"))
            return QColor(QStringLiteral("#fef08a"));
        if (status == QStringLiteral("Inactive"))
            return QColor(QStringLiteral("#fecaca"));
        return QColor(QStringLiteral("#e5e7eb"));
    };

    for (int row = 0; row < ui->clientTable->rowCount(); ++row) {
        const QTableWidgetItem *idItem = ui->clientTable->item(row, 0);
        if (!idItem)
            continue;

        const int clientId = idItem->text().toInt();
        const int score = scoresByClientId.value(clientId, 0);
        const QString status = statusByClientId.value(clientId, QStringLiteral("Inactive"));

        if (!ui->clientTable->item(row, 10))
            ui->clientTable->setItem(row, 10, new QTableWidgetItem());
        if (!ui->clientTable->item(row, 11))
            ui->clientTable->setItem(row, 11, new QTableWidgetItem());

        ui->clientTable->item(row, 10)->setText(QString::number(score));
        ui->clientTable->item(row, 11)->setText(status);

        const QColor bg = statusColor(status);
        for (int col = 0; col < ui->clientTable->columnCount(); ++col) {
            if (QTableWidgetItem *it = ui->clientTable->item(row, col)) {
                it->setBackground(bg);
                it->setForeground(QColor(QStringLiteral("#111827")));
            }
        }
    }
}

void MainWindow::onAnalyzeClientsClicked()
{
    QList<ClientScoringRow> rows;
    QString err;
    if (!fetchClientScoringRowsFromDb(rows, &err)) {
        QMessageBox::warning(this, QStringLiteral("Client Scoring System"),
                             QStringLiteral("Impossible de lire les clients depuis la base.\n%1").arg(err));
        return;
    }
    if (rows.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Client Scoring System"),
                                 QStringLiteral("Aucun client a analyser."));
        return;
    }

    QHash<int, int> scoresByClientId;
    QHash<int, QString> statusByClientId;
    int phoneApiFailures = 0;
    QStringList phoneFailureSamples;

    for (const ClientScoringRow &row : rows) {
        const bool blocked = row.statutClient.contains(QStringLiteral("block"), Qt::CaseInsensitive);
        bool phoneValid = false;
        if (!blocked) {
            QString apiErr;
            if (!isNumverifyPhoneValid(row.phone, &phoneValid, &apiErr)) {
                phoneApiFailures++;
                qDebug().noquote() << QStringLiteral("[Numverify] client %1 tel=%2 err=%3")
                                           .arg(row.id)
                                           .arg(row.phone)
                                           .arg(apiErr);
                if (phoneFailureSamples.size() < 5 && !apiErr.isEmpty()) {
                    const QString line = QStringLiteral("ID %1, tel %2 : %3")
                                               .arg(row.id)
                                               .arg(row.phone)
                                               .arg(apiErr);
                    if (!phoneFailureSamples.contains(line))
                        phoneFailureSamples.append(line);
                }
            }
        }

        const int score = computeClientScoreValue(row.lastPurchaseDate, phoneValid, blocked);
        const QString status = computeClientStatusLabel(score, blocked);
        scoresByClientId.insert(row.id, score);
        statusByClientId.insert(row.id, status);
    }

    applyClientScoringResults(scoresByClientId, statusByClientId);

    QString info = QStringLiteral("Analyse terminee pour %1 clients.").arg(rows.size());
    if (phoneApiFailures > 0) {
        info += QStringLiteral(
                    "\n%1 appels Numverify ont echoue (format international, reseau, ou quota / cle API). "
                    "Les numeros courts sont envoyes comme +216...")
                    .arg(phoneApiFailures);
        if (!phoneFailureSamples.isEmpty())
            info += QStringLiteral("\n\nDetail (echantillon):\n%1").arg(phoneFailureSamples.join(QStringLiteral("\n")));
    }
    QMessageBox::information(this, QStringLiteral("Client Scoring System"), info);
}

ClientData MainWindow::currentClientSnapshotFromForm() const
{
    ClientData c;
    c.id = ui->lineEdit_IDC ? ui->lineEdit_IDC->text().trimmed().toInt() : 0;
    c.nom = ui->lineEdit_nomC ? ui->lineEdit_nomC->text().trimmed() : QString();
    c.prenom = ui->lineEdit_PrenomC ? ui->lineEdit_PrenomC->text().trimmed() : QString();
    c.email = ui->lineEdit_emailC ? ui->lineEdit_emailC->text().trimmed() : QString();
    c.telephone = ui->lineEdit_telephoneC ? ui->lineEdit_telephoneC->text().trimmed() : QString();
    c.adresse = ui->lineEdit_adresseC ? ui->lineEdit_adresseC->text().trimmed() : QString();
    c.statutClient = ui->comboBox_statutC ? ui->comboBox_statutC->currentText().trimmed() : QString();
    c.canalAcquisition = ui->lineEdit_canalC ? ui->lineEdit_canalC->text().trimmed() : QString();
    c.modePaiementPrefere = ui->lineEdit_modeC ? ui->lineEdit_modeC->text().trimmed() : QString();
    c.remiseAccordee = ui->lineEdit_remiseC ? ui->lineEdit_remiseC->text().trimmed().toDouble() : 0.0;
    c.limiteCredit = ui->lineEdit_limiteCreditSeg ? ui->lineEdit_limiteCreditSeg->text().trimmed().toDouble() : 0.0;
    return c;
}

void MainWindow::beginClientEditMode()
{
    if (m_clientEditMode)
        return;
    if (!ui->clientTable || ui->clientTable->currentRow() < 0)
        return;

    m_clientEditSnapshot = currentClientSnapshotFromForm();
    m_clientCurrentRowId = m_clientEditSnapshot.id;
    m_clientEditMode = true;

    if (ui->pushButton_ajouter) {
        ui->pushButton_ajouter->setText(QStringLiteral("Enregistrer"));
        ui->pushButton_ajouter->setToolTip(QStringLiteral("Enregistrer les modifications du client"));
        if (m_clientCancelEditButton)
            ui->pushButton_ajouter->setMinimumSize(m_clientCancelEditButton->minimumSize());
    }
    if (m_clientCancelEditButton)
        m_clientCancelEditButton->show();
    if (ui->lineEdit_IDC)
        ui->lineEdit_IDC->setReadOnly(true);

    if (ui->clientTable) {
        ui->clientTable->setSelectionMode(QAbstractItemView::NoSelection);
        ui->clientTable->setFocusPolicy(Qt::NoFocus);
        for (int r = 0; r < ui->clientTable->rowCount(); ++r) {
            if (QWidget *w = ui->clientTable->cellWidget(r, 13))
                w->setEnabled(false);
            if (QWidget *w = ui->clientTable->cellWidget(r, 14))
                w->setEnabled(false);
        }
    }

    const QString activeFieldStyle = QStringLiteral(
        "QLineEdit { border: 2px solid #c08a5b; border-radius: 8px; background: #fffaf3; }"
        "QComboBox { border: 2px solid #c08a5b; border-radius: 8px; background: #fffaf3; }");
    if (ui->lineEdit_nomC) ui->lineEdit_nomC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_PrenomC) ui->lineEdit_PrenomC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_emailC) ui->lineEdit_emailC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_telephoneC) ui->lineEdit_telephoneC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_adresseC) ui->lineEdit_adresseC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_canalC) ui->lineEdit_canalC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_modeC) ui->lineEdit_modeC->setStyleSheet(activeFieldStyle);
    if (ui->lineEdit_limiteCreditSeg) ui->lineEdit_limiteCreditSeg->setStyleSheet(activeFieldStyle);
    if (ui->comboBox_statutC) ui->comboBox_statutC->setStyleSheet(activeFieldStyle);
}

void MainWindow::exitClientEditMode(bool restoreSnapshot)
{
    if (restoreSnapshot && m_clientEditMode) {
        ui->lineEdit_IDC->setText(QString::number(m_clientEditSnapshot.id));
        ui->lineEdit_nomC->setText(m_clientEditSnapshot.nom);
        ui->lineEdit_PrenomC->setText(m_clientEditSnapshot.prenom);
        ui->lineEdit_emailC->setText(m_clientEditSnapshot.email);
        ui->lineEdit_telephoneC->setText(m_clientEditSnapshot.telephone);
        ui->lineEdit_adresseC->setText(m_clientEditSnapshot.adresse);
        ui->lineEdit_canalC->setText(m_clientEditSnapshot.canalAcquisition);
        ui->lineEdit_modeC->setText(m_clientEditSnapshot.modePaiementPrefere);
        ui->lineEdit_remiseC->setText(QString::number(m_clientEditSnapshot.remiseAccordee, 'f', 2));
        ui->lineEdit_limiteCreditSeg->setText(QString::number(m_clientEditSnapshot.limiteCredit, 'f', 2));
        if (ui->comboBox_statutC)
            ui->comboBox_statutC->setCurrentText(m_clientEditSnapshot.statutClient);
    }

    m_clientEditMode = false;
    m_clientCurrentRowId = -1;
    m_clientEditSnapshot = ClientData{};

    if (ui->pushButton_ajouter) {
        ui->pushButton_ajouter->setText(QStringLiteral("Ajouter"));
        ui->pushButton_ajouter->setToolTip(QString());
    }
    if (m_clientCancelEditButton)
        m_clientCancelEditButton->hide();
    if (ui->lineEdit_IDC)
        ui->lineEdit_IDC->setReadOnly(false);

    if (ui->clientTable) {
        ui->clientTable->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->clientTable->setFocusPolicy(Qt::StrongFocus);
        for (int r = 0; r < ui->clientTable->rowCount(); ++r) {
            if (QWidget *w = ui->clientTable->cellWidget(r, 13))
                w->setEnabled(true);
            if (QWidget *w = ui->clientTable->cellWidget(r, 14))
                w->setEnabled(true);
        }
    }

    if (ui->lineEdit_nomC) ui->lineEdit_nomC->setStyleSheet(QString());
    if (ui->lineEdit_PrenomC) ui->lineEdit_PrenomC->setStyleSheet(QString());
    if (ui->lineEdit_emailC) ui->lineEdit_emailC->setStyleSheet(QString());
    if (ui->lineEdit_telephoneC) ui->lineEdit_telephoneC->setStyleSheet(QString());
    if (ui->lineEdit_adresseC) ui->lineEdit_adresseC->setStyleSheet(QString());
    if (ui->lineEdit_canalC) ui->lineEdit_canalC->setStyleSheet(QString());
    if (ui->lineEdit_modeC) ui->lineEdit_modeC->setStyleSheet(QString());
    if (ui->lineEdit_limiteCreditSeg) ui->lineEdit_limiteCreditSeg->setStyleSheet(QString());
    if (ui->comboBox_statutC) ui->comboBox_statutC->setStyleSheet(QString());
}

void MainWindow::fillClientFormFromSelectedRow()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) return;
    ui->lineEdit_IDC->setText(ui->clientTable->item(row, 0)->text());
    ui->lineEdit_nomC->setText(ui->clientTable->item(row, 1)->text());
    ui->lineEdit_PrenomC->setText(ui->clientTable->item(row, 2)->text());
    ui->lineEdit_emailC->setText(ui->clientTable->item(row, 3)->text());
    ui->lineEdit_telephoneC->setText(ui->clientTable->item(row, 4)->text());
    ui->lineEdit_adresseC->setText(ui->clientTable->item(row, 5)->text());
    ui->comboBox_statutC->setCurrentText(ui->clientTable->item(row, 6)->text());
    ui->lineEdit_remiseC->setText(ui->clientTable->item(row, 8)->text());
    ClientData fullClient;
    QString loadErr;
    if (Client::chargerParId(ui->clientTable->item(row, 0)->text().toInt(), fullClient, &loadErr)) {
        ui->lineEdit_limiteCreditSeg->setText(QString::number(fullClient.limiteCredit, 'f', 2));
    }
    ClientData c;
    c.id = ui->clientTable->item(row, 0)->text().toInt();
    c.totalAchats = ui->clientTable->item(row, 9)->text().toDouble();
    c.frequenceAchat = fullClient.frequenceAchat;
    c.retardsPaiement = fullClient.retardsPaiement;
    c.limiteCredit = fullClient.limiteCredit;
    c.soldeCreditUtilise = ui->clientTable->item(row, 12)->text().toDouble();
    updateAiInsightsPanel(c);
    refreshClientRecommendations();
}

bool MainWindow::readCurrentClientForm(ClientData &out) const
{
    out = {};
    bool idOk = false;
    const int id = ui->lineEdit_IDC->text().trimmed().toInt(&idOk);
    if (!idOk || id <= 0)
        return false;
    out.id = id;
    out.nom = ui->lineEdit_nomC->text().trimmed();
    out.prenom = ui->lineEdit_PrenomC->text().trimmed();
    out.email = ui->lineEdit_emailC->text().trimmed();
    out.telephone = ui->lineEdit_telephoneC->text().trimmed();
    return !out.telephone.isEmpty();
}

void MainWindow::clearClientForm()
{
    if (m_clientEditMode)
        exitClientEditMode(false);
    ui->lineEdit_IDC->clear();
    ui->lineEdit_nomC->clear();
    ui->lineEdit_PrenomC->clear();
    ui->lineEdit_emailC->clear();
    ui->lineEdit_telephoneC->clear();
    ui->lineEdit_adresseC->clear();
    ui->lineEdit_remiseC->clear();
    ui->lineEdit_canalC->clear();
    ui->lineEdit_modeC->clear();
}

bool MainWindow::validateClientFormInputs(bool isUpdate)
{
    const QString idText = ui->lineEdit_IDC->text().trimmed();
    const QString nom = ui->lineEdit_nomC->text().trimmed();
    const QString prenom = ui->lineEdit_PrenomC->text().trimmed();
    const QString email = ui->lineEdit_emailC->text().trimmed();
    const QString telephone = ui->lineEdit_telephoneC->text().trimmed();
    const QString adresse = ui->lineEdit_adresseC->text().trimmed();
    QStringList errors;
    QWidget *firstInvalid = nullptr;

    if (idText.isEmpty() || idText.toInt() <= 0) {
        errors << QStringLiteral("- ID > 0 requis.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_IDC;
    }
    if (nom.isEmpty()) {
        errors << QStringLiteral("- Nom requis.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_nomC;
    }
    if (prenom.isEmpty()) {
        errors << QStringLiteral("- Prenom requis.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_PrenomC;
    }
    if (email.isEmpty()) {
        errors << QStringLiteral("- Email requis.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_emailC;
    }
    const QRegularExpression emailRegex("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$", QRegularExpression::CaseInsensitiveOption);
    if (!emailRegex.match(email).hasMatch()) {
        errors << QStringLiteral("- Email invalide.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_emailC;
    }
    const QRegularExpression phoneRegex("^\\+?[0-9 ]{8,15}$");
    if (!phoneRegex.match(telephone).hasMatch()) {
        errors << QStringLiteral("- Telephone invalide (8 a 15 chiffres, + et espaces autorises).");
        if (!firstInvalid) firstInvalid = ui->lineEdit_telephoneC;
    }
    if (adresse.isEmpty()) {
        errors << QStringLiteral("- Adresse requise.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_adresseC;
    }

    const QString limiteText = ui->lineEdit_limiteCreditSeg->text().trimmed();
    if (limiteText.isEmpty()) {
        errors << QStringLiteral("- Limite credit requise.");
        if (!firstInvalid) firstInvalid = ui->lineEdit_limiteCreditSeg;
    } else {
        bool limiteOk = false;
        const double limite = limiteText.toDouble(&limiteOk);
        if (!limiteOk || limite < 0.0) {
            errors << QStringLiteral("- Limite credit invalide (nombre >= 0).");
            if (!firstInvalid) firstInvalid = ui->lineEdit_limiteCreditSeg;
        }
    }

    if (isUpdate && ui->clientTable->currentRow() < 0) {
        errors << QStringLiteral("- Selectionnez une ligne a modifier.");
    }
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"),
                             QStringLiteral("Veuillez corriger les champs suivants :\n%1").arg(errors.join(QLatin1Char('\n'))));
        if (firstInvalid)
            firstInvalid->setFocus();
        return false;
    }
    return true;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    if (m_clientEditMode) {
        if (!validateClientFormInputs(true))
            return;
        if (m_clientCurrentRowId <= 0) {
            QMessageBox::warning(this, QStringLiteral("Clients"), QStringLiteral("Selectionnez une ligne."));
            return;
        }

        ClientData c;
        c.id = ui->lineEdit_IDC->text().toInt();
        c.nom = ui->lineEdit_nomC->text();
        c.prenom = ui->lineEdit_PrenomC->text();
        c.email = ui->lineEdit_emailC->text();
        c.telephone = ui->lineEdit_telephoneC->text();
        c.adresse = ui->lineEdit_adresseC->text();
        c.statutClient = ui->comboBox_statutC->currentText();
        c.canalAcquisition = ui->lineEdit_canalC->text();
        c.modePaiementPrefere = ui->lineEdit_modeC->text();
        c.limiteCredit = ui->lineEdit_limiteCreditSeg->text().toDouble();

        ClientData prev;
        if (Client::chargerParId(m_clientCurrentRowId, prev, nullptr)) {
            c.totalAchats = prev.totalAchats;
            c.frequenceAchat = prev.frequenceAchat;
            c.retardsPaiement = prev.retardsPaiement;
            c.soldeCreditUtilise = prev.soldeCreditUtilise;
            c.latitude = prev.latitude;
            c.longitude = prev.longitude;
        } else {
            c.totalAchats = 0.0;
            c.frequenceAchat = 0;
            c.retardsPaiement = 0;
            c.soldeCreditUtilise = 0.0;
        }

        const QString confirmMsg = QStringLiteral("Confirmer la modification du client ID %1 ?").arg(c.id);
        if (QMessageBox::question(this, QStringLiteral("Clients"), confirmMsg,
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes) {
            return;
        }

        QString err;
        if (!Client::modifier(c, &err)) {
            QMessageBox::critical(this, QStringLiteral("Modification client"), QStringLiteral("Echec modification:\n") + err);
            return;
        }
        exitClientEditMode(false);
        loadClients();
        refreshClientRecommendations();
        QMessageBox::information(this, QStringLiteral("Clients"), QStringLiteral("Modifications enregistrées."));
        return;
    }

    if (!validateClientFormInputs(false))
        return;

    ClientData c;
    c.id = ui->lineEdit_IDC->text().toInt();
    c.nom = ui->lineEdit_nomC->text();
    c.prenom = ui->lineEdit_PrenomC->text();
    c.email = ui->lineEdit_emailC->text();
    c.telephone = ui->lineEdit_telephoneC->text();
    c.adresse = ui->lineEdit_adresseC->text();
    c.statutClient = ui->comboBox_statutC->currentText();
    c.canalAcquisition = ui->lineEdit_canalC->text();
    c.modePaiementPrefere = ui->lineEdit_modeC->text();
    c.totalAchats = 0;
    c.frequenceAchat = 0;
    c.retardsPaiement = 0;
    c.limiteCredit = ui->lineEdit_limiteCreditSeg->text().toDouble();

    const QString confirmMsg = QStringLiteral("Confirmer l'ajout du client ID %1 ?").arg(c.id);
    if (QMessageBox::question(this, QStringLiteral("Clients"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    QString err;
    if (!Client::ajouter(c, &err)) {
        QMessageBox::critical(this, "Ajout client", "Echec d'ajout:\n" + err);
        return;
    }
    loadClients();
    clearClientForm();
    refreshClientRecommendations();
    QMessageBox::information(this, QStringLiteral("Clients"), QStringLiteral("Client ajoute."));
}

void MainWindow::on_pushButton_supprimer_clicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, QStringLiteral("Clients"), QStringLiteral("Selectionnez une ligne."));
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    QString nomAff;
    if (QTableWidgetItem *it1 = ui->clientTable->item(row, 1))
        nomAff += it1->text();
    if (QTableWidgetItem *it2 = ui->clientTable->item(row, 2)) {
        if (!nomAff.isEmpty())
            nomAff += QLatin1Char(' ');
        nomAff += it2->text();
    }
    nomAff = nomAff.trimmed();
    const QString confTxt = nomAff.isEmpty()
                                ? QStringLiteral("Supprimer le client ID %1 ?").arg(id)
                                : QStringLiteral("Supprimer %1 (ID %2) ?").arg(nomAff, QString::number(id));
    if (QMessageBox::question(this, QStringLiteral("Clients"), confTxt, QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    QString err;
    if (!Client::supprimer(id, &err)) {
        QMessageBox::critical(this, QStringLiteral("Clients"), QStringLiteral("Echec.\n") + err);
        return;
    }
    loadClients();
    clearClientForm();
    refreshClientRecommendations();
    QMessageBox::information(this, QStringLiteral("Clients"), QStringLiteral("Client supprime."));
}

void MainWindow::on_pushButton_modifier_clicked()
{
    const int row = ui->clientTable ? ui->clientTable->currentRow() : -1;
    if (row < 0 || !ui->clientTable->item(row, 0)) {
        QMessageBox::warning(this, QStringLiteral("Clients"), QStringLiteral("Selectionnez une ligne."));
        return;
    }
    fillClientFormFromSelectedRow();
    beginClientEditMode();
}

void MainWindow::on_btnRechercher_3_clicked()
{
    const QString motCle = ui->lineEditSearch_3->text().trimmed();
    const QString categorie = ui->comboBoxFiltreCategorie->currentText();
    const QString statut = ui->comboBoxFiltreStatut->currentText();
    const QString risque = ui->comboBoxFiltreRisque->currentText();

    QList<ClientData> clients;
    QString err;
    if (!Client::rechercherEtFiltrer(motCle, categorie, statut, risque, clients, &err)) {
        QMessageBox::warning(this, "Recherche", err);
        return;
    }
    ui->clientTable->setRowCount(0);
    for (const ClientData &c : clients) {
        const int row = ui->clientTable->rowCount();
        ui->clientTable->insertRow(row);
        ui->clientTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));
        ui->clientTable->setItem(row, 1, new QTableWidgetItem(c.nom));
        ui->clientTable->setItem(row, 2, new QTableWidgetItem(c.prenom));
        ui->clientTable->setItem(row, 3, new QTableWidgetItem(c.email));
        ui->clientTable->setItem(row, 4, new QTableWidgetItem(c.telephone));
        ui->clientTable->setItem(row, 5, new QTableWidgetItem(c.adresse));
        ui->clientTable->setItem(row, 6, new QTableWidgetItem(c.statutClient));
        ui->clientTable->setItem(row, 7, new QTableWidgetItem(c.categorie));
        ui->clientTable->setItem(row, 8, new QTableWidgetItem(QString::number(c.remiseAccordee, 'f', 2)));
        ui->clientTable->setItem(row, 9, new QTableWidgetItem(QString::number(c.totalAchats, 'f', 2)));
        ui->clientTable->setItem(row, 10, new QTableWidgetItem(QStringLiteral("-")));
        ui->clientTable->setItem(row, 11, new QTableWidgetItem(QStringLiteral("Not analyzed")));
        ui->clientTable->setItem(row, 12, new QTableWidgetItem(QString::number(c.soldeCreditUtilise, 'f', 2)));

        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), ui->clientTable);
        deleteBtn->setToolTip(QStringLiteral("Supprimer ce client"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            ui->clientTable->setCurrentCell(row, 0);
            on_pushButton_supprimer_clicked();
        });
        ui->clientTable->setCellWidget(row, 13, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), ui->clientTable);
        updateBtn->setToolTip(QStringLiteral("Modifier ce client"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(updateBtn);
        connect(updateBtn, &QPushButton::clicked, this, [this, row]() {
            ui->clientTable->setCurrentCell(row, 0);
            fillClientFormFromSelectedRow();
            beginClientEditMode();
        });
        ui->clientTable->setCellWidget(row, 14, updateBtn);
    }
    if (ui->pageClient) {
        if (QComboBox *combo = ui->pageClient->findChild<QComboBox *>(QStringLiteral("clientTopFilterCombo")))
            applyTopColumnFilter(ui->clientTable, motCle, combo->currentData().toInt());
    }
    refreshClientRecommendations();
}

void MainWindow::on_pushButton_resetFiltres_clicked()
{
    ui->lineEditSearch_3->clear();
    ui->comboBoxFiltreCategorie->setCurrentText("Toutes");
    ui->comboBoxFiltreStatut->setCurrentText("Tous");
    ui->comboBoxFiltreRisque->setCurrentText("Tous");
    loadClients();
    refreshClientRecommendations();
}

void MainWindow::on_clientTable_cellClicked(int, int)
{
    if (m_clientEditMode)
        return;
    fillClientFormFromSelectedRow();
}

void MainWindow::onVerifierCommandeClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, QStringLiteral("Credit"), QStringLiteral("Selectionnez un client."));
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    if (ui->lineEdit_montantCommandeSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Saisie"), QStringLiteral("Montant commande requis."));
        ui->lineEdit_montantCommandeSeg->setFocus();
        return;
    }
    const double montant = ui->lineEdit_montantCommandeSeg->text().toDouble();
    if (montant <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Saisie"), QStringLiteral("Montant > 0."));
        ui->lineEdit_montantCommandeSeg->setFocus();
        return;
    }
    QString err;
    CreditCheckResult r = Client::verifierBlocageCommande(id, montant, &err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Credit", err);
        return;
    }
    ui->labelCreditRestantSeg->setText("Credit restant: " + QString::number(r.restant, 'f', 2));
    if (r.allowed)
        QMessageBox::information(this, QStringLiteral("Commande"), r.message);
    else
        QMessageBox::warning(this, QStringLiteral("Commande"), r.message);
}

void MainWindow::onEnregistrerPaiementClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, QStringLiteral("Paiement"), QStringLiteral("Selectionnez un client."));
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    if (ui->lineEdit_montantPaiementSeg->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Saisie"), QStringLiteral("Montant paiement requis."));
        ui->lineEdit_montantPaiementSeg->setFocus();
        return;
    }
    const double montant = ui->lineEdit_montantPaiementSeg->text().toDouble();
    if (montant <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Saisie"), QStringLiteral("Montant > 0."));
        ui->lineEdit_montantPaiementSeg->setFocus();
        return;
    }
    QString err;
    if (!Client::enregistrerPaiement(id, montant, ui->lineEdit_notePaiementSeg->text(), &err)) {
        QMessageBox::critical(this, "Paiement", err);
        return;
    }
    loadClients();
    refreshClientRecommendations();
    QMessageBox::information(this, QStringLiteral("Paiement"), QStringLiteral("Enregistre."));
}

void MainWindow::onVoirHistoriqueClicked()
{
    const int row = ui->clientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, QStringLiteral("Historique"), QStringLiteral("Selectionnez un client."));
        return;
    }
    const int id = ui->clientTable->item(row, 0)->text().toInt();
    QList<QPair<QString, QString>> rows;
    QString err;
    if (!Client::historiquePaiements(id, rows, &err)) {
        QMessageBox::warning(this, "Historique", err);
        return;
    }
    QString txt;
    for (const auto &r : rows) {
        txt += r.first + " | " + r.second + "\n";
    }
    if (txt.isEmpty()) txt = "Aucun paiement enregistré.";
    QMessageBox::information(this, "Historique paiements", txt);
}

void MainWindow::onExporterClientsClicked()
{
    if (ui->clientTable->rowCount() <= 0) {
        QMessageBox::information(this, "Export", "Aucun client.");
        return;
    }

    const QString defaultName = "clients_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".xls";
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter les clients",
        defaultName,
        "Fichiers Excel (*.xls)"
    );

    if (filePath.trimmed().isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export", "Erreur fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Entetes Excel (TSV compatible .xls)
    QStringList headers;
    headers.reserve(ui->clientTable->columnCount());
    for (int col = 0; col < ui->clientTable->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->clientTable->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : QString("COL_%1").arg(col));
    }
    out << headers.join('\t') << "\n";

    // Donnees Excel (TSV compatible .xls)
    for (int row = 0; row < ui->clientTable->rowCount(); ++row) {
        QStringList values;
        values.reserve(ui->clientTable->columnCount());
        for (int col = 0; col < ui->clientTable->columnCount(); ++col) {
            QTableWidgetItem *item = ui->clientTable->item(row, col);
            QString v = item ? item->text() : QString();
            v.replace('\t', ' ');
            v.replace('\n', ' ');
            values << v;
        }
        out << values.join('\t') << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export", "Export reussi.");
}

void MainWindow::onExporterMatieresPremieresClicked()
{
    if (!ui->employeeTable_5 || ui->employeeTable_5->rowCount() <= 0) {
        QMessageBox::information(this, QStringLiteral("Export"),
                                 QStringLiteral("Aucune matière première à exporter."));
        return;
    }

    const QString defaultName = QStringLiteral("matieres_premieres_")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")) + QStringLiteral(".xls");
    const QString filePath = QFileDialog::getSaveFileName(this,
                                                          QStringLiteral("Exporter les matières premières"),
                                                          defaultName,
                                                          QStringLiteral("Fichiers Excel (*.xls)"));
    if (filePath.trimmed().isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Export"), QStringLiteral("Impossible d'écrire le fichier."));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    QStringList headers;
    headers.reserve(ui->employeeTable_5->columnCount());
    for (int col = 0; col < ui->employeeTable_5->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->employeeTable_5->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : QStringLiteral("COL_%1").arg(col));
    }
    out << headers.join(QLatin1Char('\t')) << QLatin1Char('\n');

    for (int row = 0; row < ui->employeeTable_5->rowCount(); ++row) {
        if (ui->employeeTable_5->isRowHidden(row))
            continue;
        QStringList values;
        values.reserve(ui->employeeTable_5->columnCount());
        for (int col = 0; col < ui->employeeTable_5->columnCount(); ++col) {
            QString v;
            if (QTableWidgetItem *item = ui->employeeTable_5->item(row, col))
                v = item->text();
            else if (QWidget *cw = ui->employeeTable_5->cellWidget(row, col)) {
                if (auto *le = qobject_cast<QLineEdit *>(cw))
                    v = le->text();
                else if (auto *cb = qobject_cast<QComboBox *>(cw))
                    v = cb->currentText();
                else if (auto *lbl = qobject_cast<QLabel *>(cw))
                    v = lbl->text();
            }
            v.replace(QLatin1Char('\t'), QLatin1Char(' '));
            v.replace(QLatin1Char('\n'), QLatin1Char(' '));
            values << v;
        }
        out << values.join(QLatin1Char('\t')) << QLatin1Char('\n');
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Export réussi."));
}

void MainWindow::onClientSurveySmsClicked()
{
    ClientData client;
    if (!readCurrentClientForm(client)) {
        QMessageBox::information(this, QStringLiteral("Avis"),
                                 QStringLiteral("Client requis."));
        return;
    }

    if (client.email.trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Avis"),
                                 QStringLiteral("Adresse email client requise."));
        return;
    }

    QString surveyErr;
    QString surveyUrl = ClientNotificationService::surveyUrlForClient(client, &surveyErr);
    if (surveyUrl.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Avis"),
                                 surveyErr + QStringLiteral("\n\nCollez l'URL https au prochain pas."));
        bool okTpl = false;
        const QString deflt = QString::fromUtf8(qgetenv("LEATHER_SURVEY_URL_TEMPLATE")).trimmed();
        const QString tpl = QInputDialog::getMultiLineText(
            this,
            QStringLiteral("URL questionnaire"),
            QStringLiteral("URL https (marqueurs : {id} {nom} {prenom} {email})."),
            deflt,
            &okTpl);
        if (!okTpl || tpl.trimmed().isEmpty())
            return;
        QSettings s;
        s.beginGroup(QStringLiteral("ClientNotif"));
        s.setValue(QStringLiteral("surveyUrlTemplate"), tpl.trimmed());
        s.sync();
        surveyUrl = ClientNotificationService::surveyUrlForClient(client, &surveyErr);
        if (surveyUrl.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Avis"),
                                 surveyErr.isEmpty() ? QStringLiteral("Lien invalide.") : surveyErr);
            return;
        }
    }

    const QString surveyReceiverEmail = QString::fromUtf8(qgetenv("LEATHER_SURVEY_RECEIVER_EMAIL")).trimmed();
    if (!surveyReceiverEmail.isEmpty()) {
        QUrl surveyQUrl(surveyUrl);
        QUrlQuery query(surveyQUrl);
        query.addQueryItem(QStringLiteral("shopEmail"), surveyReceiverEmail);
        surveyQUrl.setQuery(query);
        surveyUrl = surveyQUrl.toString(QUrl::FullyEncoded);
    }

    const QString prenom = client.prenom.trimmed().isEmpty() ? client.nom.trimmed() : client.prenom.trimmed();
    const QString subject = QStringLiteral("Votre avis nous interesse - Leather House");
    const QString body = QStringLiteral("Bonjour %1,\n\n"
                                        "Merci pour votre achat chez Leather House.\n"
                                        "Nous serions ravis d'avoir votre avis (1 minute) :\n%2\n\n"
                                        "Merci pour votre confiance.")
                             .arg(prenom, surveyUrl);

    if (QMessageBox::question(this, QStringLiteral("Confirmation"),
                              QStringLiteral("Envoyer l'email de questionnaire a %1 ?\n\n%2")
                                  .arg(client.email.trimmed(),
                                       body.size() > 320 ? body.left(320) + QStringLiteral("…") : body),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    QString smtpErr;
    if (!LeatherSmtp::sendSurveyEmail(client.email.trimmed(), subject, body, &smtpErr)) {
        QMessageBox::warning(this, QStringLiteral("Avis"),
                             QStringLiteral("Envoi email impossible.\n%1").arg(smtpErr));
        return;
    }
    QMessageBox::information(this, QStringLiteral("Avis"),
                             QStringLiteral("Email questionnaire envoye a %1.\n"
                                            "Le client ouvre le lien, remplit le formulaire, puis envoie le rendu a votre adresse.")
                                 .arg(client.email.trimmed()));
}

void MainWindow::onClientWhatsAppClicked()
{
    if (!m_whatsappBusinessService) {
        QMessageBox::warning(this, QStringLiteral("WhatsApp"), QStringLiteral("Service indisponible."));
        return;
    }

    QString cfgHint;
    if (!WhatsAppBusinessService::isConfigured(&cfgHint)) {
        QMessageBox::information(this, QStringLiteral("WhatsApp (Twilio)"),
                                 QStringLiteral("Configuration Twilio absente.\n\n%1").arg(cfgHint));
        return;
    }

    ClientData client;
    if (!readCurrentClientForm(client)) {
        QMessageBox::information(this, QStringLiteral("WhatsApp"), QStringLiteral("Client requis."));
        return;
    }
    if (client.telephone.trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("WhatsApp"),
                                 QStringLiteral("Numero de telephone client requis (fiche client)."));
        return;
    }

    const QStringList choices = {QStringLiteral("Rappel livraison"),
                                 QStringLiteral("Code de retrait / accueil"),
                                 QStringLiteral("Relance reglement (douce)"),
                                 QStringLiteral("Lien questionnaire (avis)")};
    bool picked = false;
    const QString choice = QInputDialog::getItem(this,
                                                 QStringLiteral("WhatsApp"),
                                                 QStringLiteral("Type de message :"),
                                                 choices,
                                                 0,
                                                 false,
                                                 &picked);
    if (!picked)
        return;

    QString body;
    bool previewUrl = false;

    if (choice == choices.at(0)) {
        body = ClientNotificationService::buildPresetMessage(ClientNotificationService::SmsPreset::RappelLivraison,
                                                               client,
                                                               QString());
    } else if (choice == choices.at(1)) {
        bool okCode = false;
        const QString codeRetrait = QInputDialog::getText(this,
                                                          QStringLiteral("Code accueil"),
                                                          QStringLiteral("Code a communiquer au client :"),
                                                          QLineEdit::Normal,
                                                          QString(),
                                                          &okCode);
        if (!okCode || codeRetrait.trimmed().isEmpty()) {
            QMessageBox::information(this, QStringLiteral("WhatsApp"), QStringLiteral("Annule."));
            return;
        }
        body = ClientNotificationService::buildPresetMessage(ClientNotificationService::SmsPreset::CodeRetrait,
                                                               client,
                                                               codeRetrait);
    } else if (choice == choices.at(2)) {
        body = ClientNotificationService::buildPresetMessage(ClientNotificationService::SmsPreset::RelanceDouce,
                                                               client,
                                                               QString());
    } else {
        QString surveyErr;
        QString surveyUrl = ClientNotificationService::surveyUrlForClient(client, &surveyErr);
        if (surveyUrl.isEmpty()) {
            QMessageBox::information(this,
                                     QStringLiteral("WhatsApp"),
                                     surveyErr + QStringLiteral("\n\nCollez l'URL https au prochain pas."));
            bool okTpl = false;
            const QString deflt = QString::fromUtf8(qgetenv("LEATHER_SURVEY_URL_TEMPLATE")).trimmed();
            const QString tpl = QInputDialog::getMultiLineText(
                this,
                QStringLiteral("URL questionnaire"),
                QStringLiteral("URL https (marqueurs : {id} {nom} {prenom} {email})."),
                deflt,
                &okTpl);
            if (!okTpl || tpl.trimmed().isEmpty())
                return;
            QSettings s;
            s.beginGroup(QStringLiteral("ClientNotif"));
            s.setValue(QStringLiteral("surveyUrlTemplate"), tpl.trimmed());
            s.sync();
            surveyUrl = ClientNotificationService::surveyUrlForClient(client, &surveyErr);
            if (surveyUrl.isEmpty()) {
                QMessageBox::warning(this,
                                     QStringLiteral("WhatsApp"),
                                     surveyErr.isEmpty() ? QStringLiteral("Lien invalide.") : surveyErr);
                return;
            }
        }

        const QString surveyReceiverEmail = QString::fromUtf8(qgetenv("LEATHER_SURVEY_RECEIVER_EMAIL")).trimmed();
        if (!surveyReceiverEmail.isEmpty()) {
            QUrl surveyQUrl(surveyUrl);
            QUrlQuery query(surveyQUrl);
            query.addQueryItem(QStringLiteral("shopEmail"), surveyReceiverEmail);
            surveyQUrl.setQuery(query);
            surveyUrl = surveyQUrl.toString(QUrl::FullyEncoded);
        }

        const QString prenom = client.prenom.trimmed().isEmpty() ? client.nom.trimmed() : client.prenom.trimmed();
        body = QStringLiteral("Bonjour %1, merci pour votre achat chez Leather House. "
                              "Votre avis nous aide a progresser (1 min) : %2")
                   .arg(prenom, surveyUrl);
        previewUrl = true;
    }

    const QString prefix = QString::fromUtf8(qgetenv("LEATHER_PHONE_PREFIX")).trimmed();
    const QString e164 = ClientNotificationService::normalizePhoneE164(client.telephone, prefix);
    if (e164.isEmpty() || !e164.startsWith(QLatin1Char('+'))) {
        QMessageBox::information(
            this,
            QStringLiteral("WhatsApp"),
            QStringLiteral("Telephone invalide. Enregistrez le numero avec indicatif international (+216...) "
                           "ou definissez LEATHER_PHONE_PREFIX (+216, +33, ...)."));
        return;
    }
    QString toDigits = e164;
    toDigits.remove(0, 1);
    m_lastWhatsAppDestinationDisplay = e164;

    if (QMessageBox::question(this,
                              QStringLiteral("Confirmation"),
                              QStringLiteral("Envoyer ce message WhatsApp au %1 ?\n\n%2")
                                  .arg(e164,
                                       body.size() > 300 ? body.left(300) + QStringLiteral("…") : body),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    m_whatsappBusinessService->sendTextMessage(toDigits, body, previewUrl);
}

void MainWindow::updateAiInsightsPanel(const ClientData &client)
{
    Q_UNUSED(client)
}

void MainWindow::refreshClientRecommendations()
{
    m_lastRecommendations.clear();
}

void MainWindow::onOpenAssistantRequested()
{
    if (!m_assistantWindow) {
        m_assistantWindow = new AssistantWindow();
        m_assistantWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(m_assistantWindow, &QObject::destroyed, this, [this]() {
            m_assistantWindow = nullptr;
        });
    }
    m_assistantWindow->show();
    m_assistantWindow->raise();
    m_assistantWindow->activateWindow();
}

void MainWindow::onGlobalChatbotClicked()
{
    if (!m_globalChatbotWindow) {
        m_globalChatbotWindow = new ChatbotWindow(this);
        m_globalChatbotWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(m_globalChatbotWindow, &QObject::destroyed, this, [this]() {
            m_globalChatbotWindow = nullptr;
        });
    }
    m_globalChatbotWindow->show();
    m_globalChatbotWindow->raise();
    m_globalChatbotWindow->activateWindow();
}

// ------------------- Page Produits -------------------

void MainWindow::installProduitsPageResponsiveLayout()
{
    QWidget *page = ui->page_2;
    if (!page || page->layout())
        return;

    // Calques vides du Designer (grand rectangle sur la zone tableau + fiche) : au-dessus en z-order,
    // ils bloquent souris / clavier sur toute la fiche produit si on ne les masque pas.
    auto hideDesignerOverlay = [](QWidget *w) {
        if (!w)
            return;
        w->hide();
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    };
    static const char *const kProduitOverlays[] = {
        "horizontalLayoutWidget_7",
        "verticalLayoutWidget_6",
        "verticalLayoutWidget",
    };
    for (const char *name : kProduitOverlays) {
        if (QWidget *ow = page->findChild<QWidget *>(QLatin1String(name)))
            hideDesignerOverlay(ow);
    }

    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(10);

    auto *topActionsCard = new QFrame(page);
    topActionsCard->setObjectName(QStringLiteral("clientTopNavBar"));
    auto *topLay = new QHBoxLayout(topActionsCard);
    topLay->setContentsMargins(12, 8, 12, 8);
    topLay->setSpacing(10);

    const QString btnStyle = QStringLiteral(
        "QPushButton {"
        "  border: 1px solid #d5dbe5;"
        "  border-radius: 8px;"
        "  background: #ffffff;"
        "  color: #000000;"
        "  font-weight: 600;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
        "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }");

    auto *btnExportExcel = new QPushButton(QStringLiteral("Export Excel"), topActionsCard);
    btnExportExcel->setCursor(Qt::PointingHandCursor);
    btnExportExcel->setStyleSheet(btnStyle);
    btnExportExcel->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(btnExportExcel, &QPushButton::clicked, this, [this]() {
        if (!ui->employeeTable_4 || ui->employeeTable_4->rowCount() == 0) {
            QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Aucun produit."));
            return;
        }
        const QString path = QFileDialog::getSaveFileName(this,
                                                          QStringLiteral("Exporter Excel (CSV)"),
                                                          QStringLiteral("produits.csv"),
                                                          QStringLiteral("CSV (*.csv)"));
        if (path.isEmpty())
            return;

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Export"),
                                 QStringLiteral("Impossible d'ecrire le fichier."));
            return;
        }

        auto esc = [](QString v) {
            v.replace(QLatin1Char('"'), QStringLiteral("\"\""));
            return QStringLiteral("\"%1\"").arg(v);
        };

        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << QChar(0xFEFF);

        // Colonnes metier uniquement (0..9), sans QR/actions.
        QStringList headers;
        for (int c = 0; c <= 9; ++c) {
            QTableWidgetItem *h = ui->employeeTable_4->horizontalHeaderItem(c);
            headers << esc(h ? h->text() : QStringLiteral("Colonne %1").arg(c + 1));
        }
        out << headers.join(QLatin1Char(',')) << QLatin1Char('\n');

        for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
            if (ui->employeeTable_4->isRowHidden(r))
                continue;
            QStringList values;
            for (int c = 0; c <= 9; ++c) {
                QTableWidgetItem *it = ui->employeeTable_4->item(r, c);
                values << esc(it ? it->text() : QString());
            }
            out << values.join(QLatin1Char(',')) << QLatin1Char('\n');
        }
        file.close();
        QMessageBox::information(this,
                                 QStringLiteral("Export"),
                                 QStringLiteral("Fichier enregistre :\n%1").arg(path));
    });

    auto *btnCatalogueTop = new QPushButton(QStringLiteral("Catalogue"), topActionsCard);
    btnCatalogueTop->setCursor(Qt::PointingHandCursor);
    btnCatalogueTop->setStyleSheet(btnStyle);
    btnCatalogueTop->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    connect(btnCatalogueTop, &QPushButton::clicked, this, [this]() {
        auto *dlg = new QDialog(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose, true);
        dlg->setWindowTitle(QStringLiteral("Catalogue Produits en Cuir"));
        dlg->resize(1100, 700);
        auto *lay = new QVBoxLayout(dlg);
        lay->setContentsMargins(0, 0, 0, 0);
        auto *catalogue = new CatalogueProduitsWidget(dlg);
        lay->addWidget(catalogue);
        dlg->show();
        dlg->raise();
        dlg->activateWindow();
    });

    auto *btnAlerteTop = new QPushButton(QStringLiteral("Alerte"), topActionsCard);
    btnAlerteTop->setCursor(Qt::PointingHandCursor);
    btnAlerteTop->setStyleSheet(btnStyle);
    btnAlerteTop->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
    connect(btnAlerteTop, &QPushButton::clicked, this, &MainWindow::sendStockAlert);

    auto *searchTop = new QLineEdit(topActionsCard);
    searchTop->setMinimumHeight(34);
    searchTop->setMinimumWidth(260);
    searchTop->setMaximumWidth(360);
    searchTop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchTop->setPlaceholderText(QStringLiteral("Rechercher..."));
    searchTop->setStyleSheet(QStringLiteral(
        "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
        "QLineEdit:focus { border-color: #c08a5b; }"));

    auto *btnSearch = new QPushButton(QStringLiteral("Rechercher"), topActionsCard);
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setMinimumSize(116, 34);
    btnSearch->setMaximumWidth(116);
    btnSearch->setStyleSheet(btnStyle);
    auto *btnReset = new QPushButton(QStringLiteral("Reset"), topActionsCard);
    auto *filterTop = new QComboBox(topActionsCard);
    filterTop->setObjectName(QStringLiteral("produitsTopFilterCombo"));
    filterTop->setMinimumSize(130, 34);
    filterTop->setStyleSheet(QStringLiteral(
        "QComboBox { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
        "QComboBox:focus { border-color: #c08a5b; }"));
    filterTop->addItem(QStringLiteral("Filtre: Tous"), -1);
    filterTop->addItem(QStringLiteral("ID"), 0);
    filterTop->addItem(QStringLiteral("Nom"), 1);
    filterTop->addItem(QStringLiteral("Categorie"), 2);
    filterTop->addItem(QStringLiteral("Type cuir"), 3);
    filterTop->addItem(QStringLiteral("Gamme"), 4);
    filterTop->addItem(QStringLiteral("Etat"), 6);

    btnReset->setCursor(Qt::PointingHandCursor);
    btnReset->setMinimumSize(116, 34);
    btnReset->setMaximumWidth(116);
    btnReset->setStyleSheet(btnStyle);

    connect(searchTop, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (ui->lineEditSearch_5)
            ui->lineEditSearch_5->setText(text);
    });
    connect(searchTop, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_4_clicked);
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::on_btnRechercher_4_clicked);
    connect(filterTop, &QComboBox::currentIndexChanged, this, [this](int) { on_btnRechercher_4_clicked(); });
    connect(btnReset, &QPushButton::clicked, this, [this, searchTop]() {
        searchTop->clear();
        if (ui->lineEditSearch_5)
            ui->lineEditSearch_5->clear();
        refreshProduitsTable();
    });

    topLay->addWidget(btnExportExcel);
    topLay->addWidget(btnCatalogueTop);
    topLay->addWidget(btnAlerteTop);
    topLay->addStretch(1);
    topLay->addWidget(filterTop);
    topLay->addWidget(searchTop, 1);
    topLay->addWidget(btnSearch);
    topLay->addWidget(btnReset);
    root->addWidget(topActionsCard, 0);

    auto *body = new QHBoxLayout();
    body->setSpacing(14);
    root->addLayout(body, 1);

    auto *leftCard = new QWidget(page);
    leftCard->setObjectName(QStringLiteral("clientFormCard"));
    leftCard->setMinimumWidth(300);
    leftCard->setMaximumWidth(380);
    leftCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *leftCol = new QVBoxLayout(leftCard);
    leftCol->setContentsMargins(20, 20, 20, 20);
    leftCol->setSpacing(10);
    body->addWidget(leftCard, 0);

    auto *rightZone = new QWidget(page);
    rightZone->setObjectName(QStringLiteral("clientTableZone"));
    rightZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightCol = new QVBoxLayout(rightZone);
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setSpacing(10);
    body->addWidget(rightZone, 1);

    if (ui->layoutWidget_3)
        ui->layoutWidget_3->hide();
    if (ui->employeeTable_4) {
        ui->employeeTable_4->setParent(rightZone);
        ui->employeeTable_4->setMinimumHeight(280);
        ui->employeeTable_4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        rightCol->addWidget(ui->employeeTable_4, 1);
    }
    if (ui->employeeFormBox_4) {
        ui->employeeFormBox_4->setParent(leftCard);
        ui->employeeFormBox_4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        leftCol->addWidget(ui->employeeFormBox_4, 1);
    }

    if (ui->chatGroupBox_4)
        ui->chatGroupBox_4->hide();

    // La barre de recherche peut s'élargir avec la fenêtre (Designer limitait à 350 px).
    constexpr int kMaxWidget = 16777215;
    if (ui->lineEditSearch_5) {
        ui->lineEditSearch_5->setMinimumWidth(180);
        ui->lineEditSearch_5->setMaximumWidth(kMaxWidget);
        ui->lineEditSearch_5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    if (ui->lineEditSearch_4) {
        ui->lineEditSearch_4->setMinimumWidth(120);
        ui->lineEditSearch_4->setMaximumWidth(kMaxWidget);
        ui->lineEditSearch_4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    if (ui->contentStack) {
        ui->contentStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

void MainWindow::setupProduitPage()
{
    if (!ui->employeeTable_4)
        return;

    // Certains labels Designer peuvent rester visibles apres reconstruction dynamique de la fiche.
    if (ui->label_4)
        ui->label_4->hide();
    if (ui->page_2) {
        const QList<QLabel *> pageLabels = ui->page_2->findChildren<QLabel *>();
        for (QLabel *lab : pageLabels) {
            if (!lab)
                continue;
            if (lab->text().trimmed().compare(QStringLiteral("Id produit"), Qt::CaseInsensitive) == 0)
                lab->hide();
        }
    }

    if (!m_produitUiWired) {
        m_produitUiWired = true;

        if (!ui->employeeFormBox_4->property("produitFicheModernized").toBool()) {
            ui->employeeFormBox_4->setTitle(QString());
            applyFormStyle(ui->employeeFormBox_4);

            if (ui->formRow1_4) {
                ui->formOuterLayout_4->removeItem(ui->formRow1_4);
                delete ui->formRow1_4;
                ui->formRow1_4 = nullptr;
            }
            if (ui->formRow2_4) {
                ui->formOuterLayout_4->removeItem(ui->formRow2_4);
                delete ui->formRow2_4;
                ui->formRow2_4 = nullptr;
            }

            const QList<QWidget *> legacyProductLabels = {
                ui->labelCIN_4, ui->labelNom_4, ui->labelPrenom_4, ui->labelSexe_4, ui->labelSalaire_4,
                ui->labelDateEmbauche_4, ui->labelTelephone_2, ui->labelPoste_2, ui->labelAdresse_2, ui->label_4
            };
            for (QWidget *w : legacyProductLabels) {
                if (w)
                    w->hide();
            }
            // Masque aussi tout ancien label Designer restant dans la fiche produit.
            const QList<QLabel *> strayProductLabels = ui->employeeFormBox_4->findChildren<QLabel *>();
            for (QLabel *lab : strayProductLabels) {
                if (!lab)
                    continue;
                const QString n = lab->objectName();
                if (n == QStringLiteral("label_4") || n.endsWith(QStringLiteral("_4")))
                    lab->hide();
            }

            auto *titleLab = new QLabel(QStringLiteral("Fiche produit"), ui->employeeFormBox_4);
            titleLab->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #000000; padding: 2px 4px;"));
            ui->formOuterLayout_4->insertWidget(0, titleLab);

            auto *formWrap = new QWidget(ui->employeeFormBox_4);
            formWrap->setStyleSheet(QStringLiteral("background:#ffffff;"));
            auto *formLay = new QFormLayout(formWrap);
            formLay->setContentsMargins(4, 2, 4, 4);
            formLay->setHorizontalSpacing(12);
            formLay->setVerticalSpacing(12);
            formLay->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            formLay->setFormAlignment(Qt::AlignTop);
            formLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

            formLay->addRow(new QLabel(QStringLiteral("ID"), formWrap), ui->lineEditIdProduit);
            formLay->addRow(new QLabel(QStringLiteral("Nom produit"), formWrap), ui->lineEditCIN_4);
            formLay->addRow(new QLabel(QStringLiteral("Categorie"), formWrap), ui->lineEditNom_4);
            formLay->addRow(new QLabel(QStringLiteral("Type cuir"), formWrap), ui->lineEditPrenom_4);
            formLay->addRow(new QLabel(QStringLiteral("Qualite"), formWrap), ui->comboBox_3);
            formLay->addRow(new QLabel(QStringLiteral("Qt stock"), formWrap), ui->lineEditAdresse_2);
            formLay->addRow(new QLabel(QStringLiteral("Etat"), formWrap), ui->comboBox_4);
            formLay->addRow(new QLabel(QStringLiteral("Date fab."), formWrap), ui->dateTimeEdit);
            formLay->addRow(new QLabel(QStringLiteral("Type design"), formWrap), ui->comboBox_5);
            formLay->addRow(new QLabel(QStringLiteral("Style"), formWrap), ui->lineEditPrenom_5);
            if (!m_produitQrLabel) {
                m_produitQrLabel = new QLabel(formWrap);
                m_produitQrLabel->setObjectName(QStringLiteral("labelQR"));
                m_produitQrLabel->setMinimumSize(200, 200);
                m_produitQrLabel->setMaximumSize(220, 220);
                m_produitQrLabel->setAlignment(Qt::AlignCenter);
                m_produitQrLabel->setScaledContents(true);
                m_produitQrLabel->setStyleSheet(QStringLiteral("background:#ffffff; border:1px solid #cfd8e6; border-radius:8px;"));
                m_produitQrLabel->setText(QStringLiteral("QR non genere"));
            }
            formLay->addRow(new QLabel(QStringLiteral("QR code"), formWrap), m_produitQrLabel);

            const QList<QWidget *> produitFields = {
                static_cast<QWidget *>(ui->lineEditIdProduit),
                static_cast<QWidget *>(ui->lineEditCIN_4),
                static_cast<QWidget *>(ui->lineEditNom_4),
                static_cast<QWidget *>(ui->lineEditPrenom_4),
                static_cast<QWidget *>(ui->comboBox_3),
                static_cast<QWidget *>(ui->lineEditAdresse_2),
                static_cast<QWidget *>(ui->comboBox_4),
                static_cast<QWidget *>(ui->dateTimeEdit),
                static_cast<QWidget *>(ui->comboBox_5),
                static_cast<QWidget *>(ui->lineEditPrenom_5),
            };
            for (QWidget *w : produitFields) {
                if (!w)
                    continue;
                w->setMinimumHeight(30);
                w->setMaximumWidth(220);
                w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            }
            ui->formOuterLayout_4->insertWidget(1, formWrap);

            if (!ui->employeeFormBox_4->property("produitFormScrollWrapped").toBool()) {
                auto *sa = new QScrollArea(ui->page_2);
                sa->setObjectName(QStringLiteral("clientFormScrollArea"));
                sa->setWidgetResizable(true);
                sa->setFrameShape(QFrame::StyledPanel);
                sa->setFrameShadow(QFrame::Plain);
                sa->setStyleSheet(QStringLiteral(
                    "QScrollArea { border: 1px solid rgb(230, 220, 200); border-radius: 6px; background: rgb(255, 255, 255); }"));
                sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

                QWidget *leftCard = ui->employeeFormBox_4->parentWidget();
                QVBoxLayout *leftCol = leftCard ? qobject_cast<QVBoxLayout *>(leftCard->layout()) : nullptr;
                if (leftCol) {
                    for (int i = leftCol->count() - 1; i >= 0; --i) {
                        QLayoutItem *it = leftCol->itemAt(i);
                        if (!it)
                            continue;
                        if (it->widget() == ui->employeeFormBox_4) {
                            QLayoutItem *removed = leftCol->takeAt(i);
                            delete removed;
                            continue;
                        }
                        if (it->spacerItem()) {
                            QLayoutItem *removed = leftCol->takeAt(i);
                            delete removed;
                        }
                    }

                    auto *scrollContainer = new QWidget(sa);
                    auto *scrollLay = new QVBoxLayout(scrollContainer);
                    scrollLay->setContentsMargins(20, 20, 20, 20);
                    scrollLay->setSpacing(10);

                    ui->employeeFormBox_4->setParent(scrollContainer);
                    ui->employeeFormBox_4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    scrollLay->addWidget(ui->employeeFormBox_4, 1);
                    scrollContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

                    sa->setWidget(scrollContainer);
                    sa->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    leftCol->insertWidget(0, sa, 1);
                }
                ui->employeeFormBox_4->setProperty("produitFormScrollWrapped", true);
            }

            if (ui->formBtnLayout_4) {
                ui->formBtnLayout_4->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                ui->formBtnLayout_4->setSpacing(10);
                for (int i = ui->formBtnLayout_4->count() - 1; i >= 0; --i) {
                    QLayoutItem *it = ui->formBtnLayout_4->itemAt(i);
                    if (it && it->spacerItem()) {
                        QLayoutItem *removed = ui->formBtnLayout_4->takeAt(i);
                        delete removed;
                    }
                }
            }
            if (ui->btnAjouter_6) {
                applyUnifiedAddButtonStyle(ui->btnAjouter_6);
                ui->btnAjouter_6->setText(QStringLiteral("Ajouter produit"));
            }
            if (!m_produitCancelEditButton && ui->formBtnLayout_4 && ui->page_2) {
                m_produitCancelEditButton = new QPushButton(QStringLiteral("Annuler"), ui->page_2);
                m_produitCancelEditButton->setCursor(Qt::PointingHandCursor);
                m_produitCancelEditButton->setMinimumSize(110, 32);
                if (ui->btnAjouter_6)
                    m_produitCancelEditButton->setStyleSheet(ui->btnAjouter_6->styleSheet());
                m_produitCancelEditButton->hide();
                ui->formBtnLayout_4->addWidget(m_produitCancelEditButton);
                connect(m_produitCancelEditButton, &QPushButton::clicked, this, [this]() {
                    m_produitEditMode = false;
                    m_produitEditingId = -1;
                    if (ui->btnAjouter_6)
                        ui->btnAjouter_6->setText(QStringLiteral("Ajouter produit"));
                    if (m_produitCancelEditButton)
                        m_produitCancelEditButton->hide();
                    if (ui->employeeTable_4) {
                        ui->employeeTable_4->setSelectionMode(QAbstractItemView::SingleSelection);
                        for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
                            if (QWidget *w = ui->employeeTable_4->cellWidget(r, 11)) w->setEnabled(true);
                            if (QWidget *w = ui->employeeTable_4->cellWidget(r, 12)) w->setEnabled(true);
                        }
                    }
                });
            }
            if (!m_produitVoirQrButton && ui->formBtnLayout_4) {
                m_produitVoirQrButton = new QPushButton(QStringLiteral("Voir QR code"), ui->page_2);
                m_produitVoirQrButton->setCursor(Qt::PointingHandCursor);
                // Meme gabarit que « Ajouter produit » (applyUnifiedAddButtonStyle : 180 x 38).
                if (ui->btnAjouter_6)
                    m_produitVoirQrButton->setMinimumSize(ui->btnAjouter_6->minimumSize());
                else
                    m_produitVoirQrButton->setMinimumSize(180, 38);
                if (ui->btnAjouter_6)
                    m_produitVoirQrButton->setStyleSheet(ui->btnAjouter_6->styleSheet());
                m_produitVoirQrButton->setEnabled(false);
                ui->formBtnLayout_4->addWidget(m_produitVoirQrButton);
                connect(m_produitVoirQrButton, &QPushButton::clicked, this, [this]() {
                    if (!m_lastProduitQrPath.isEmpty()) {
                        QPixmap px(m_lastProduitQrPath);
                        if (!px.isNull()) {
                            QDialog dlg(this);
                            dlg.setWindowTitle(QStringLiteral("QR code produit"));
                            auto *lay = new QVBoxLayout(&dlg);
                            auto *lab = new QLabel(&dlg);
                            lab->setAlignment(Qt::AlignCenter);
                            lab->setPixmap(px.scaled(420, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                            lay->addWidget(lab);
                            dlg.resize(460, 480);
                            dlg.exec();
                            return;
                        }
                    }
                    const int row = ui->employeeTable_4 ? ui->employeeTable_4->currentRow() : -1;
                    if (row >= 0)
                        showProduitQrDialog(row);
                    else
                        QMessageBox::information(this, QStringLiteral("QR Code"),
                                                 QStringLiteral("Aucun QR code disponible pour le moment."));
                });
            }
            if (ui->btnModifier_4) {
                ui->btnModifier_4->hide();
                if (ui->formBtnLayout_4)
                    ui->formBtnLayout_4->removeWidget(ui->btnModifier_4);
            }
            if (ui->btnSupprimer_4) {
                ui->btnSupprimer_4->hide();
                if (ui->formBtnLayout_4)
                    ui->formBtnLayout_4->removeWidget(ui->btnSupprimer_4);
            }
            if (ui->lineEditPrenom_5)
                ui->lineEditPrenom_5->show();
            if (ui->comboBox_5)
                ui->comboBox_5->show();
            if (ui->dateTimeEdit)
                ui->dateTimeEdit->show();
            ui->employeeFormBox_4->setProperty("produitFicheModernized", true);
        }

        ui->employeeTable_4->setColumnCount(13);
        ui->employeeTable_4->setHorizontalHeaderLabels({
            QStringLiteral("ID"),
            QStringLiteral("Nom produit"),
            QStringLiteral("Categorie"),
            QStringLiteral("Type cuir"),
            QStringLiteral("Qualite"),
            QStringLiteral("Qt stock"),
            QStringLiteral("Etat"),
            QStringLiteral("Date fab."),
            QStringLiteral("Type design"),
            QStringLiteral("Style (interne)"),
            QStringLiteral("QR Code"),
            QStringLiteral("SUPPR."),
            QStringLiteral("MODIF."),
        });
        ui->employeeTable_4->horizontalHeader()->setStretchLastSection(true);
        ui->employeeTable_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->employeeTable_4->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->employeeTable_4->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->employeeTable_4->setEditTriggers(QAbstractItemView::NoEditTriggers);

        if (ui->comboBox_4 && ui->comboBox_4->findText(QStringLiteral("Inactif")) < 0)
            ui->comboBox_4->addItem(QStringLiteral("Inactif"));

        if (ui->lineEditAdresse_2)
            ui->lineEditAdresse_2->setValidator(new QIntValidator(0, 999999999, this));

        // Le Designer met du texte par defaut ("Nom", "Type"...) : confus avec des libelles figes.
        // Remettre des champs vides, editables, et remonter la fiche au-dessus des calques residuels.
        const auto prepLineEdit = [](QLineEdit *le, const QString &placeholder) {
            if (!le)
                return;
            le->setReadOnly(false);
            le->setEnabled(true);
            le->setFocusPolicy(Qt::StrongFocus);
            le->clear();
            le->setPlaceholderText(placeholder);
        };
        if (ui->lineEditIdProduit) {
            ui->lineEditIdProduit->setReadOnly(false);
            ui->lineEditIdProduit->setEnabled(true);
            ui->lineEditIdProduit->setFocusPolicy(Qt::StrongFocus);
            ui->lineEditIdProduit->setValidator(new QIntValidator(1, 999999999, this));
            ui->lineEditIdProduit->setPlaceholderText(QStringLiteral("Entrez ID (ou laissez vide pour auto)"));
            ui->lineEditIdProduit->clear();
        }
        prepLineEdit(ui->lineEditCIN_4, QStringLiteral("Entrez nom produit"));
        prepLineEdit(ui->lineEditNom_4, QStringLiteral("Selectionnez categorie"));
        prepLineEdit(ui->lineEditPrenom_4, QStringLiteral("Selectionnez type de cuir"));
        prepLineEdit(ui->lineEditAdresse_2, QStringLiteral("Quantite en stock (entier)"));
        prepLineEdit(ui->lineEditPrenom_5, QStringLiteral("Entrez le style"));

        if (ui->comboBox_3) {
            ui->comboBox_3->setEnabled(true);
            ui->comboBox_3->setFocusPolicy(Qt::StrongFocus);
            ui->comboBox_3->setPlaceholderText(QStringLiteral("Selectionnez qualite"));
        }
        if (ui->comboBox_4) {
            ui->comboBox_4->setEnabled(true);
            ui->comboBox_4->setFocusPolicy(Qt::StrongFocus);
            ui->comboBox_4->setPlaceholderText(QStringLiteral("Selectionnez etat"));
        }
        if (ui->comboBox_5) {
            ui->comboBox_5->setEnabled(true);
            ui->comboBox_5->setFocusPolicy(Qt::StrongFocus);
            ui->comboBox_5->setPlaceholderText(QStringLiteral("Selectionnez type design"));
        }
        if (ui->comboBox_6) {
            ui->comboBox_6->setEnabled(true);
            ui->comboBox_6->setFocusPolicy(Qt::StrongFocus);
        }
        if (ui->dateTimeEdit) {
            ui->dateTimeEdit->setVisible(true);
            ui->dateTimeEdit->setCalendarPopup(true);
            ui->dateTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
        }
        const auto markInvalid = [](QWidget *w, bool invalid) {
            if (!w)
                return;
            w->setProperty("invalid", invalid);
            w->style()->unpolish(w);
            w->style()->polish(w);
            w->update();
        };
        const auto wireRequiredLine = [markInvalid](QLineEdit *le) {
            if (!le)
                return;
            QObject::connect(le, &QLineEdit::textChanged, le, [le, markInvalid](const QString &txt) {
                markInvalid(le, txt.trimmed().isEmpty());
            });
            markInvalid(le, le->text().trimmed().isEmpty());
        };
        wireRequiredLine(ui->lineEditCIN_4);
        wireRequiredLine(ui->lineEditNom_4);
        wireRequiredLine(ui->lineEditPrenom_4);
        wireRequiredLine(ui->lineEditAdresse_2);
        wireRequiredLine(ui->lineEditPrenom_5);

        const auto wireRequiredCombo = [markInvalid](QComboBox *cb) {
            if (!cb)
                return;
            QObject::connect(cb, &QComboBox::currentTextChanged, cb, [cb, markInvalid](const QString &txt) {
                markInvalid(cb, txt.trimmed().isEmpty());
            });
            markInvalid(cb, cb->currentText().trimmed().isEmpty());
        };
        wireRequiredCombo(ui->comboBox_3);
        wireRequiredCombo(ui->comboBox_4);
        wireRequiredCombo(ui->comboBox_5);
        if (ui->lineEditSearch_5) {
            ui->lineEditSearch_5->setReadOnly(false);
            ui->lineEditSearch_5->setEnabled(true);
            ui->lineEditSearch_5->setFocusPolicy(Qt::StrongFocus);
            ui->lineEditSearch_5->clear();
            ui->lineEditSearch_5->setPlaceholderText(QStringLiteral("Rechercher..."));
        }
        if (ui->lineEditSearch_4)
            ui->lineEditSearch_4->hide();
        if (ui->pushButton_8)
            ui->pushButton_8->hide();
        if (ui->groupBox_3)
            ui->groupBox_3->hide();
        if (ui->employeeFormBox_4) {
            ui->employeeFormBox_4->setEnabled(true);
            ui->employeeFormBox_4->setFocusPolicy(Qt::WheelFocus);
        }

        if (ui->employeeTable_4->selectionModel()) {
            connect(ui->employeeTable_4->selectionModel(),
                    &QItemSelectionModel::currentRowChanged,
                    this,
                    [this](const QModelIndex &current, const QModelIndex &) {
                        if (!ui->employeeTable_4 || !current.isValid())
                            return;
                        const int row = current.row();
                        if (row < 0 || !ui->employeeTable_4->item(row, 0))
                            return;
                        fillProduitFormFromTableRow(row);
                    });
        }
        // Ne pas connecter btnAjouter_6 / btnModifier_4 / btnSupprimer_4 ici : setupUi() appelle deja
        // QMetaObject::connectSlotsByName(), qui lie ces boutons aux slots on_btn*_clicked (double connexion = double action).

        // Recherche produits : pas d'auto-connect pour returnPressed
        if (ui->lineEditSearch_5)
            connect(ui->lineEditSearch_5, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_4_clicked);
        // btnRechercher_4 -> on_btnRechercher_4_clicked via connectSlotsByName

        if (ui->textEdit) {
            ui->textEdit->setReadOnly(true);
            ui->textEdit->setPlaceholderText(QStringLiteral("Alertes stock faible, produits inactifs, mentions defauts…"));
        }

        // Actions rapides visibles dans la barre Produits.
        if (ui->employesHeaderRow_4 && ui->page_2) {
            auto *btnVocal = new QPushButton(QStringLiteral("Vocale"), ui->page_2);
            btnVocal->setObjectName(QStringLiteral("btnProduitVocaleTop"));
            btnVocal->setMinimumSize(96, 32);
            btnVocal->setCursor(Qt::PointingHandCursor);
            btnVocal->setStyleSheet(QStringLiteral(
                "QPushButton { background:#f3f5f8; border:1px solid #d7e1f0; border-radius:8px; font-weight:600; color:#1f2937; padding:6px 10px; }"
                "QPushButton:hover { background:#fff4e9; border-color:#d8b18f; color:#7a4d2b; }"));
            connect(btnVocal, &QPushButton::clicked, this, &MainWindow::on_pushButton_7_clicked);

            ui->employesHeaderRow_4->addWidget(btnVocal);
        }
    }

    if (db.isOpen())
        refreshProduitsTable();
}

/// API Node « api-test » (Express + oracledb) : `/stats`, `/stock/faible`, `/prediction`.
/// Base URL : variable `LEATHER_STOCK_API_BASE` (ex. `http://localhost:3000`), défaut sans slash final.
static QString leatherStockApiBaseUrl()
{
    QString u = QString::fromUtf8(qgetenv("LEATHER_STOCK_API_BASE")).trimmed();
    while (u.endsWith(QLatin1Char('/')))
        u.chop(1);
    if (u.isEmpty())
        return QStringLiteral("http://localhost:3000");
    return u;
}

void MainWindow::loadStats()
{
    if (!m_networkAccessManager) {
        qDebug() << "[API][Stats] QNetworkAccessManager indisponible.";
        return;
    }

    const QUrl url(leatherStockApiBaseUrl() + QStringLiteral("/stats"));
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray payload = reply->readAll();
        qDebug().noquote() << "[API][Stats] Réponse /stats:\n" << QString::fromUtf8(payload);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[API][Stats] Erreur réseau:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "[API][Stats] JSON invalide:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        const int totalProduits = obj.value(QStringLiteral("totalProduits")).toInt();
        const int produitsRupture = obj.value(QStringLiteral("produitsRupture")).toInt();
        const int stockFaible = obj.value(QStringLiteral("stockFaible")).toInt();

        setProperty("api_totalProduits", totalProduits);
        setProperty("api_produitsRupture", produitsRupture);
        setProperty("api_stockFaible", stockFaible);

        if (m_dashKpiProduits)
            m_dashKpiProduits->setText(QString::number(totalProduits));

        if (m_dashCatalogueStats) {
            m_dashCatalogueStats->setText(
                QStringLiteral("Total produits : %1  |  Rupture : %2  |  Stock faible : %3")
                    .arg(totalProduits)
                    .arg(produitsRupture)
                    .arg(stockFaible));
        }

        if (ui->statTitle1_2)
            ui->statTitle1_2->setText(QStringLiteral("Total produits : %1").arg(totalProduits));
        if (ui->statTitle2_2)
            ui->statTitle2_2->setText(QStringLiteral("Produits rupture : %1").arg(produitsRupture));
        if (ui->statTitle3_2)
            ui->statTitle3_2->setText(QStringLiteral("Stock faible : %1").arg(stockFaible));

        reply->deleteLater();
    });
}

void MainWindow::loadStockFaible()
{
    if (!m_networkAccessManager) {
        qDebug() << "[API][StockFaible] QNetworkAccessManager indisponible.";
        return;
    }

    const QUrl url(leatherStockApiBaseUrl() + QStringLiteral("/stock/faible"));
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray payload = reply->readAll();
        qDebug().noquote() << "[API][StockFaible] Réponse /stock/faible:\n" << QString::fromUtf8(payload);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[API][StockFaible] Erreur réseau:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            qDebug() << "[API][StockFaible] JSON invalide:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        const QJsonArray arr = doc.array();
        setProperty("api_stockFaible", arr.size());

        QStringList criticalRows;
        int ruptureCount = 0;
        for (const QJsonValue &v : arr) {
            if (!v.isObject())
                continue;
            const QJsonObject o = v.toObject();
            const QString nom = o.value(QStringLiteral("nom")).toString(
                o.value(QStringLiteral("name")).toString(QStringLiteral("Produit")));
            const int stock = o.value(QStringLiteral("stock")).toInt(o.value(QStringLiteral("quantite")).toInt(0));
            if (stock == 0)
                ++ruptureCount;
            if (stock < 5)
                criticalRows << QStringLiteral("%1 - Stock %2").arg(nom).arg(stock);
        }

        setProperty("api_produitsRupture", ruptureCount);

        if (m_dashCatalogueTopCategories) {
            m_dashCatalogueTopCategories->setText(
                criticalRows.isEmpty()
                    ? QStringLiteral("Produits critiques (<5) : aucun")
                    : QStringLiteral("Produits critiques (<5) :\n%1").arg(criticalRows.join(QStringLiteral("\n"))));
        }

        QWidget *rightParent = ui->employeeTable_5 ? ui->employeeTable_5->parentWidget() : nullptr;
        QVBoxLayout *rightLay = rightParent ? qobject_cast<QVBoxLayout *>(rightParent->layout()) : nullptr;
        QTextEdit *mpCritical = ui->page_3 ? ui->page_3->findChild<QTextEdit *>(QStringLiteral("mpCriticalListText")) : nullptr;
        if (!mpCritical && rightLay) {
            mpCritical = new QTextEdit(rightParent);
            mpCritical->setObjectName(QStringLiteral("mpCriticalListText"));
            mpCritical->setReadOnly(true);
            mpCritical->setMinimumHeight(120);
            mpCritical->setPlaceholderText(QStringLiteral("Produits critiques (<5)"));
            rightLay->addWidget(mpCritical, 0);
        }
        if (mpCritical) {
            mpCritical->setPlainText(
                criticalRows.isEmpty()
                    ? QStringLiteral("Produits critiques (<5) : aucun")
                    : criticalRows.join(QStringLiteral("\n")));
        }

        if (m_dashCatalogueStats) {
            const int totalProduits = property("api_totalProduits").toInt();
            const int produitsRupture = property("api_produitsRupture").toInt();
            const int stockFaible = property("api_stockFaible").toInt();
            m_dashCatalogueStats->setText(
                QStringLiteral("Total produits : %1  |  Rupture : %2  |  Stock faible : %3")
                    .arg(totalProduits)
                    .arg(produitsRupture)
                    .arg(stockFaible));
        }

        reply->deleteLater();
    });
}

void MainWindow::loadPrediction()
{
    if (!m_networkAccessManager) {
        qDebug() << "[API][Prediction] QNetworkAccessManager indisponible.";
        return;
    }

    const QUrl url(leatherStockApiBaseUrl() + QStringLiteral("/prediction"));
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray payload = reply->readAll();
        qDebug().noquote() << "[API][Prediction] Réponse /prediction:\n" << QString::fromUtf8(payload);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[API][Prediction] Erreur réseau:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "[API][Prediction] JSON invalide:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        const QString produit = obj.value(QStringLiteral("produit")).toString(
            obj.value(QStringLiteral("nom")).toString(QStringLiteral("N/A")));
        const int stock = obj.value(QStringLiteral("stock")).toInt(obj.value(QStringLiteral("quantite")).toInt(0));
        const double consommation = obj.value(QStringLiteral("consommation")).toDouble(
            obj.value(QStringLiteral("consommationJour")).toDouble(0.0));
        const int jours = obj.value(QStringLiteral("joursRestants")).toInt(
            obj.value(QStringLiteral("ruptureDans")).toInt(-1));

        QWidget *rightParent = ui->employeeTable_5 ? ui->employeeTable_5->parentWidget() : nullptr;
        QVBoxLayout *rightLay = rightParent ? qobject_cast<QVBoxLayout *>(rightParent->layout()) : nullptr;
        QLabel *mpPrediction = ui->page_3 ? ui->page_3->findChild<QLabel *>(QStringLiteral("mpPredictionLabel")) : nullptr;
        if (!mpPrediction && rightLay) {
            mpPrediction = new QLabel(rightParent);
            mpPrediction->setObjectName(QStringLiteral("mpPredictionLabel"));
            mpPrediction->setWordWrap(true);
            rightLay->insertWidget(1, mpPrediction, 0);
        }
        if (mpPrediction) {
            mpPrediction->setText(
                QStringLiteral("Produit : %1 | Stock : %2 | Consommation : %3 | Rupture dans : %4 jours")
                    .arg(produit)
                    .arg(stock)
                    .arg(QString::number(consommation, 'f', 2))
                    .arg(jours));
            mpPrediction->setStyleSheet(
                jours >= 0 && jours < 3
                    ? QStringLiteral("font-size: 12px; color: #c0392b; font-weight: 700; border: none; background: transparent;")
                    : QStringLiteral("font-size: 12px; color: #5a4a3a; border: none; background: transparent;"));
        }

        if (m_dashPageSubtitle) {
            m_dashPageSubtitle->setText(
                QStringLiteral("Produit : %1 | Stock : %2 | Consommation : %3 | Rupture dans : %4 jours")
                    .arg(produit)
                    .arg(stock)
                    .arg(QString::number(consommation, 'f', 2))
                    .arg(jours));
            m_dashPageSubtitle->setStyleSheet(
                jours >= 0 && jours < 3
                    ? QStringLiteral("font-size: 12px; color: #c0392b; font-weight: 700; border: none; background: transparent;")
                    : QStringLiteral("font-size: 12px; color: #5a4a3a; border: none; background: transparent;"));
        }

        reply->deleteLater();
    });
}

void MainWindow::sendStockAlert()
{
    if (!m_networkAccessManager) {
        qDebug() << "[API][Alerte] QNetworkAccessManager indisponible.";
        QMessageBox::warning(this, QStringLiteral("Alerte stock"), QStringLiteral("Service réseau indisponible."));
        return;
    }


    const QUrl url(leatherStockApiBaseUrl() + QStringLiteral("/stock/faible"));
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray payload = reply->readAll();
        qDebug().noquote() << "[API][Alerte] Réponse /stock/faible:\n" << QString::fromUtf8(payload);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[API][Alerte] Erreur réseau:" << reply->errorString();
            QMessageBox::warning(this, QStringLiteral("Alerte stock"),
                                 QStringLiteral("Erreur réseau : %1\n\nVérifie que l'API stock est démarrée (%2).")
                                     .arg(reply->errorString(), leatherStockApiBaseUrl()));
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            qDebug() << "[API][Alerte] JSON invalide:" << parseError.errorString();
            QMessageBox::warning(this, QStringLiteral("Alerte stock"), QStringLiteral("Réponse API invalide."));
            reply->deleteLater();
            return;
        }

        const QJsonArray arr = doc.array();
        if (arr.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("Alerte stock"), QStringLiteral("Stock OK"));
            reply->deleteLater();
            return;
        }

        QStringList lines;
        for (const QJsonValue &v : arr) {
            if (!v.isObject())
                continue;
            const QJsonObject o = v.toObject();
            const QString nom = o.value(QStringLiteral("nom")).toString(
                o.value(QStringLiteral("name")).toString(QStringLiteral("Produit")));
            const int stock = o.value(QStringLiteral("stock")).toInt(o.value(QStringLiteral("quantite")).toInt(0));
            lines << QStringLiteral("%1 - Stock %2").arg(nom).arg(stock);
        }

        QMessageBox::warning(this, QStringLiteral("Alerte stock"),
                             QStringLiteral("Produits critiques :\n\n%1").arg(lines.join(QStringLiteral("\n"))));
        reply->deleteLater();
    });
    return;

    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Alerte stock"),
                             QStringLiteral("Connexion base indisponible."));
        return;
    }

    struct RuptureRow {
        int id = 0;
        QString nom;
        int quantite = 0;
    };
    QVector<RuptureRow> ruptures;

    const QString nomCol = CommerceStore::produitsLibelleColumnPhysical();
    QSqlQuery q(db);
    QString sql = QStringLiteral(
        "SELECT ID, NVL(%1, 'Produit'), NVL(QUANTITE, 0) "
        "FROM PRODUITS "
        "WHERE NVL(QUANTITE, 0) = 0 "
        "ORDER BY ID").arg(nomCol);

    bool usingStockJoinFallback = false;
    if (!q.exec(sql)) {
        // Fallback schema courant: quantite dans STOCK.QTE_DISPONIBLE.
        usingStockJoinFallback = true;
        sql = QStringLiteral(
            "SELECT P.ID, NVL(P.%1, 'Produit'), NVL(S.QTE_DISPONIBLE, 0) "
            "FROM PRODUITS P "
            "LEFT JOIN STOCK S ON S.ID_PRODUIT = P.ID "
            "WHERE NVL(S.QTE_DISPONIBLE, 0) = 0 "
            "ORDER BY P.ID").arg(nomCol);
        if (!q.exec(sql)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Alerte stock"),
                                 QStringLiteral("Erreur SQL:\n%1").arg(q.lastError().text()));
            return;
        }
    }

    while (q.next()) {
        RuptureRow row;
        row.id = q.value(0).toInt();
        row.nom = q.value(1).toString().trimmed();
        row.quantite = q.value(2).toInt();
        ruptures.push_back(row);
    }

    qDebug() << "[AlerteStock] Produits trouves en rupture:" << ruptures.size()
             << "| fallback STOCK utilisé:" << usingStockJoinFallback;

    if (ruptures.isEmpty()) {
        QMessageBox::information(this,
                                 QStringLiteral("Alerte stock"),
                                 QStringLiteral("Aucun produit en rupture."));
        return;
    }

    const QString subject = QStringLiteral("Alerte Stock Produits");

    QStringList lines;
    lines << QStringLiteral("Bonjour,")
          << QString()
          << QStringLiteral("Alerte : produits sans stock (quantité = 0)")
          << QString();
    for (const RuptureRow &r : ruptures) {
        lines << QStringLiteral("- ID %1 : %2 (stock = %3)")
                     .arg(QString::number(r.id), r.nom, QString::number(r.quantite));
    }
    lines << QString()
          << QStringLiteral("Notification automatique.");
    const QString body = lines.join(QStringLiteral("\n"));

    qDebug().noquote() << "[AlerteStock] Contenu message:\n" << body;

    QString to = QStringLiteral("jahhamoufida64@gmail.com");
    if (to.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("Alerte stock"),
                             QStringLiteral("Destinataire e-mail manquant."));
        return;
    }
    qDebug() << "[AlerteStock] Destinataire e-mail:" << to;

    QString smtpErr;
    if (!LeatherSmtp::sendEmail(to, subject, body, &smtpErr)) {
        QMessageBox::warning(this,
                             QStringLiteral("Alerte stock"),
                             QStringLiteral("Échec envoi e-mail:\n%1").arg(smtpErr));
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("Alerte stock"),
                             QStringLiteral("Email envoyé avec succès."));
}

void MainWindow::refreshProduitsTable()
{
    if (!ui->employeeTable_4 || !db.isOpen())
        return;

    const int keepSelectedId = m_produitSelectedId;

    QString err;
    if (!Produit::populateProductTable(ui->employeeTable_4, &err)) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Chargement impossible.\n%1").arg(err));
        return;
    }

    if (ui->textEdit) {
        QString alertErr;
        const QString alerts = Produit::defectAlertsPlainText(&alertErr);
        ui->textEdit->setPlainText(alertErr.isEmpty()
                                        ? alerts
                                        : QStringLiteral("Erreur lecture alertes :\n%1").arg(alertErr));
    }

    if (ui->lineEditSearch_5 && !ui->lineEditSearch_5->text().trimmed().isEmpty()) {
        const int mode = Produit::searchFilterModeFromComboText(
            ui->comboBox_6 ? ui->comboBox_6->currentText() : QString());
        Produit::filterProductTable(ui->employeeTable_4, ui->lineEditSearch_5->text(), mode);
    }

    if (ui->employeeTable_4->columnCount() < 13)
        ui->employeeTable_4->setColumnCount(13);
    ui->employeeTable_4->setHorizontalHeaderItem(10, new QTableWidgetItem(QStringLiteral("QR Code")));
    ui->employeeTable_4->setHorizontalHeaderItem(11, new QTableWidgetItem(QStringLiteral("SUPPR.")));
    ui->employeeTable_4->setHorizontalHeaderItem(12, new QTableWidgetItem(QStringLiteral("MODIF.")));
    for (int row = 0; row < ui->employeeTable_4->rowCount(); ++row) {
        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), ui->employeeTable_4);
        deleteBtn->setToolTip(QStringLiteral("Supprimer ce produit"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, deleteBtn]() {
            int clickedRow = -1;
            for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
                if (ui->employeeTable_4->cellWidget(r, 11) == deleteBtn) {
                    clickedRow = r;
                    break;
                }
            }
            if (clickedRow < 0)
                return;
            ui->employeeTable_4->setCurrentCell(clickedRow, 0);
            on_produitTable_cellClicked(clickedRow, 0);
            on_btnSupprimer_4_clicked();
        });
        ui->employeeTable_4->setCellWidget(row, 11, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), ui->employeeTable_4);
        updateBtn->setToolTip(QStringLiteral("Modifier ce produit"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(updateBtn);
        connect(updateBtn, &QPushButton::clicked, this, [this, updateBtn]() {
            int clickedRow = -1;
            for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
                if (ui->employeeTable_4->cellWidget(r, 12) == updateBtn) {
                    clickedRow = r;
                    break;
                }
            }
            if (clickedRow < 0)
                return;
            ui->employeeTable_4->setCurrentCell(clickedRow, 0);
            on_produitTable_cellClicked(clickedRow, 0);
            m_produitEditMode = true;
            m_produitEditingId = m_produitSelectedId;
            if (ui->btnAjouter_6)
                ui->btnAjouter_6->setText(QStringLiteral("Enregistrer"));
            if (ui->btnAjouter_6 && m_produitCancelEditButton)
                ui->btnAjouter_6->setMinimumSize(m_produitCancelEditButton->minimumSize());
            if (m_produitCancelEditButton)
                m_produitCancelEditButton->show();
            if (ui->employeeTable_4) {
                ui->employeeTable_4->setSelectionMode(QAbstractItemView::NoSelection);
                for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
                    if (QWidget *w = ui->employeeTable_4->cellWidget(r, 11)) w->setEnabled(false);
                    if (QWidget *w = ui->employeeTable_4->cellWidget(r, 12)) w->setEnabled(false);
                }
            }
        });
        ui->employeeTable_4->setCellWidget(row, 12, updateBtn);
    }
    afficherQR();

    if (keepSelectedId > 0) {
        for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
            if (ui->employeeTable_4->isRowHidden(r))
                continue;
            QTableWidgetItem *idIt = ui->employeeTable_4->item(r, 0);
            if (!idIt)
                continue;
            if (Produit::parseProductIdText(idIt->text()) == keepSelectedId) {
                ui->employeeTable_4->selectRow(r);
                fillProduitFormFromTableRow(r);
                break;
            }
        }
    }
    onStatisticsDataChanged();
}

void MainWindow::afficherQR()
{
    if (!ui->employeeTable_4)
        return;
    const int qrColumn = 10;
    if (ui->employeeTable_4->columnCount() <= qrColumn)
        return;

    ui->employeeTable_4->setColumnWidth(qrColumn, 60);
    for (int row = 0; row < ui->employeeTable_4->rowCount(); ++row) {
        QString path;
        if (QTableWidgetItem *it = ui->employeeTable_4->item(row, qrColumn))
            path = it->text().trimmed();
        if (path.isEmpty()) {
            if (QTableWidgetItem *idIt = ui->employeeTable_4->item(row, 0)) {
                const int id = Produit::parseProductIdText(idIt->text());
                path = loadProduitQrPathFromDb(id);
            }
        }

        auto *label = new QLabel(ui->employeeTable_4);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumSize(50, 50);
        label->setMaximumSize(50, 50);
        if (!path.isEmpty()) {
            auto it = m_produitQrPixmapCache.constFind(path);
            if (it != m_produitQrPixmapCache.constEnd()) {
                label->setPixmap(it.value());
                label->setToolTip(path);
            } else {
                QPixmap pix(path);
                if (!pix.isNull()) {
                    const QPixmap scaled = pix.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    m_produitQrPixmapCache.insert(path, scaled);
                    label->setPixmap(scaled);
                    label->setToolTip(path);
                } else {
                    label->setText(QStringLiteral("QR\nN/A"));
                    label->setToolTip(QStringLiteral("QR non disponible"));
                }
            }
        } else {
            label->setText(QStringLiteral("QR\nN/A"));
            label->setToolTip(QStringLiteral("QR non disponible"));
        }
        ui->employeeTable_4->setRowHeight(row, qMax(ui->employeeTable_4->rowHeight(row), 54));
        ui->employeeTable_4->setCellWidget(row, qrColumn, label);
    }
}

void MainWindow::on_btnRechercher_4_clicked()
{
    if (!ui->employeeTable_4)
        return;
    const QString q = ui->lineEditSearch_5 ? ui->lineEditSearch_5->text() : QString();
    const int mode = Produit::searchFilterModeFromComboText(
        ui->comboBox_6 ? ui->comboBox_6->currentText() : QString());
    Produit::filterProductTable(ui->employeeTable_4, q, mode);
    if (ui->page_2) {
        if (QComboBox *combo = ui->page_2->findChild<QComboBox *>(QStringLiteral("produitsTopFilterCombo")))
            applyTopColumnFilter(ui->employeeTable_4, q, combo->currentData().toInt());
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    if (!ui->textEdit)
        return;
    QString alertErr;
    const QString alerts = Produit::defectAlertsPlainText(&alertErr);
    ui->textEdit->setPlainText(alertErr.isEmpty()
                                    ? alerts
                                    : QStringLiteral("Erreur lecture alertes :\n%1").arg(alertErr));
}

ProduitEditorWidgets MainWindow::produitEditorBindings() const
{
    ProduitEditorWidgets w;
    w.idProduit = ui->lineEditIdProduit;
    w.nomProduit = ui->lineEditCIN_4;
    w.categorie = ui->lineEditNom_4;
    w.typeCuir = ui->lineEditPrenom_4;
    w.quantiteStock = ui->lineEditAdresse_2;
    w.style = ui->lineEditPrenom_5;
    w.qualite = ui->comboBox_3;
    w.etat = ui->comboBox_4;
    w.typeDesign = ui->comboBox_5;
    w.dateFabrication = ui->dateTimeEdit;
    return w;
}

void MainWindow::on_pushButton_8_clicked()
{
    if (!m_chatbotService || !ui->lineEditSearch_4 || !ui->textEdit_3)
        return;
    const QString msg = ui->lineEditSearch_4->text().trimmed();
    if (msg.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Chatbot"), QStringLiteral("Champ vide."));
        return;
    }
    ui->lineEditSearch_4->clear();
    ui->textEdit_3->append(QStringLiteral("Vous :\n%1\n").arg(msg));
    m_chatbotSink = ChatbotSink::Produit;
    m_chatbotService->ask(msg, Produit::chatbotContextFromProductTable(ui->employeeTable_4));
}

void MainWindow::on_pushButton_7_clicked()
{
    QMessageBox::information(this, QStringLiteral("Vocal"), QStringLiteral("Non disponible."));
}

QString MainWindow::buildProduitQrPayload(int row) const
{
    if (!ui->employeeTable_4 || row < 0 || row >= ui->employeeTable_4->rowCount())
        return QString();
    auto cellText = [this, row](int col) -> QString {
        if (!ui->employeeTable_4)
            return QString();
        if (QTableWidgetItem *it = ui->employeeTable_4->item(row, col))
            return it->text().trimmed();
        return QString();
    };

    const int id = Produit::parseProductIdText(cellText(0));
    const QString nom = cellText(1);
    const QString categorie = cellText(2);
    const QString typeCuir = cellText(3);
    const QString stock = cellText(5);
    const QString etat = cellText(6);

    const QString unique = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return QStringLiteral("ID:%1;Nom:%2;Cat:%3;Type:%4;Stock:%5;Etat:%6;UUID:%7")
        .arg(id > 0 ? id : 0)
        .arg(nom)
        .arg(categorie)
        .arg(typeCuir)
        .arg(stock)
        .arg(etat)
        .arg(unique);
}

QImage MainWindow::generateQRCode(const QString &data) const
{
    if (data.trimmed().isEmpty())
        return QImage();

    const QrCode qr = QrCode::encodeText(data.toStdString().c_str(), QrCode::Ecc::LOW);
    const int size = qr.getSize();
    if (size <= 0)
        return QImage();

    QImage image(size, size, QImage::Format_RGB32);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            image.setPixel(x, y, qr.getModule(x, y) ? qRgb(0, 0, 0) : qRgb(255, 255, 255));
        }
    }
    return image.scaled(200, 200, Qt::KeepAspectRatio, Qt::FastTransformation);
}

QPixmap MainWindow::generateProduitQrPixmap(const QString &payload, int size, QString *errorMessage) const
{
    if (errorMessage)
        errorMessage->clear();
    if (payload.trimmed().isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Contenu QR vide.");
        return QPixmap();
    }

    const QImage qrImg = generateQRCode(payload);
    if (qrImg.isNull()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Generation QR invalide.");
        return QPixmap();
    }
    return QPixmap::fromImage(qrImg).scaled(qBound(128, size, 1024),
                                            qBound(128, size, 1024),
                                            Qt::KeepAspectRatio,
                                            Qt::FastTransformation);
}

bool MainWindow::ensureProduitQrSchema(QString *errorMessage) const
{
    if (!db.isOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Connexion base fermee.");
        return false;
    }
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT COUNT(*) FROM USER_TAB_COLUMNS "
            "WHERE TABLE_NAME='PRODUITS' AND COLUMN_NAME='QR_CODE'"))
        || !q.next()) {
        if (errorMessage)
            *errorMessage = q.lastError().text().trimmed();
        return false;
    }
    if (q.value(0).toInt() > 0)
        return true;

    QSqlQuery alter(db);
    if (!alter.exec(QStringLiteral("ALTER TABLE PRODUITS ADD (QR_CODE VARCHAR2(400))"))) {
        if (errorMessage)
            *errorMessage = alter.lastError().text().trimmed();
        return false;
    }
    return true;
}

bool MainWindow::saveProduitQrPathInDb(int produitId, const QString &qrPath, QString *errorMessage) const
{
    if (produitId <= 0 || qrPath.trimmed().isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Donnees QR invalides.");
        return false;
    }
    QString schemaErr;
    if (!ensureProduitQrSchema(&schemaErr)) {
        if (errorMessage)
            *errorMessage = schemaErr;
        return false;
    }
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE PRODUITS SET QR_CODE=:p WHERE ID=:id"));
    q.bindValue(QStringLiteral(":p"), qrPath.trimmed());
    q.bindValue(QStringLiteral(":id"), produitId);
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text().trimmed();
        return false;
    }
    return true;
}

QString MainWindow::loadProduitQrPathFromDb(int produitId) const
{
    if (produitId <= 0 || !db.isOpen())
        return QString();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT QR_CODE FROM PRODUITS WHERE ID=:id"));
    q.bindValue(QStringLiteral(":id"), produitId);
    if (!q.exec() || !q.next())
        return QString();
    return q.value(0).toString().trimmed();
}

void MainWindow::setProduitQrPreview(const QString &qrPath)
{
    m_lastProduitQrPath = qrPath.trimmed();
    if (!m_produitQrLabel)
        return;
    if (m_lastProduitQrPath.isEmpty()) {
        m_produitQrLabel->setPixmap(QPixmap());
        m_produitQrLabel->setText(QStringLiteral("QR non genere"));
        if (m_produitVoirQrButton)
            m_produitVoirQrButton->setEnabled(false);
        return;
    }
    QPixmap px;
    auto it = m_produitQrPixmapCache.constFind(m_lastProduitQrPath);
    if (it != m_produitQrPixmapCache.constEnd()) {
        px = it.value();
    } else {
        QPixmap raw(m_lastProduitQrPath);
        if (!raw.isNull()) {
            px = raw.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_produitQrPixmapCache.insert(m_lastProduitQrPath, px);
        }
    }
    if (px.isNull()) {
        m_produitQrLabel->setPixmap(QPixmap());
        m_produitQrLabel->setText(QStringLiteral("QR introuvable"));
        if (m_produitVoirQrButton)
            m_produitVoirQrButton->setEnabled(false);
        return;
    }
    m_produitQrLabel->setText(QString());
    m_produitQrLabel->setPixmap(px);
    if (m_produitVoirQrButton)
        m_produitVoirQrButton->setEnabled(true);
}

void MainWindow::showProduitQrDialog(int row)
{
    const QString payload = buildProduitQrPayload(row);
    if (payload.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("QR Code"), QStringLiteral("Ligne produit invalide."));
        return;
    }

    QString err;
    const QPixmap qrPixmap = generateProduitQrPixmap(payload, 420, &err);
    if (qrPixmap.isNull()) {
        QMessageBox::warning(this, QStringLiteral("QR Code"), err.isEmpty() ? QStringLiteral("Échec génération QR.") : err);
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("QR Code produit"));
    dlg.setModal(true);
    dlg.resize(520, 620);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: #f8f5ef; }"
        "QFrame#qrCard { background: #ffffff; border: 1px solid #e7dbc9; border-radius: 14px; }"
        "QLabel#qrTitle { font-size: 18px; font-weight: 700; color: #5d2e06; }"
        "QPushButton { min-height: 34px; border-radius: 8px; padding: 6px 12px; font-weight: 600; }"));

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(10);

    auto *card = new QFrame(&dlg);
    card->setObjectName(QStringLiteral("qrCard"));
    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(16, 16, 16, 16);
    cardLay->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("QR Code Produit"), card);
    title->setObjectName(QStringLiteral("qrTitle"));
    title->setAlignment(Qt::AlignCenter);
    cardLay->addWidget(title);

    auto *imgLab = new QLabel(card);
    imgLab->setAlignment(Qt::AlignCenter);
    imgLab->setPixmap(qrPixmap.scaled(360, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    cardLay->addWidget(imgLab, 1);

    auto *meta = new QLabel(QStringLiteral("Contenu structuré (JSON)"), card);
    meta->setStyleSheet(QStringLiteral("font-size: 12px; color: #6f5d4d;"));
    cardLay->addWidget(meta);

    auto *payloadView = new QTextEdit(card);
    payloadView->setReadOnly(true);
    payloadView->setPlainText(payload);
    payloadView->setMinimumHeight(120);
    payloadView->setStyleSheet(QStringLiteral("QTextEdit { background:#fbf8f3; border:1px solid #e4d7c5; border-radius:8px; }"));
    cardLay->addWidget(payloadView);

    root->addWidget(card, 1);

    auto *actions = new QHBoxLayout();
    actions->setSpacing(10);
    auto *btnSave = new QPushButton(QStringLiteral("Sauvegarder PNG"), &dlg);
    auto *btnPrint = new QPushButton(QStringLiteral("Imprimer"), &dlg);
    auto *btnClose = new QPushButton(QStringLiteral("Fermer"), &dlg);
    btnSave->setStyleSheet(QStringLiteral("QPushButton { background:#ffffff; color:#5d2e06; border:1px solid #d9c8b0; }"
                                          "QPushButton:hover { background:#fff5e8; border-color:#c08a5b; }"));
    btnPrint->setStyleSheet(btnSave->styleSheet());
    btnClose->setStyleSheet(QStringLiteral("QPushButton { background:#5d2e06; color:#ffffff; border:none; }"
                                           "QPushButton:hover { background:#6f3708; }"));
    actions->addWidget(btnSave);
    actions->addWidget(btnPrint);
    actions->addStretch(1);
    actions->addWidget(btnClose);
    root->addLayout(actions);

    connect(btnClose, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(btnSave, &QPushButton::clicked, &dlg, [this, qrPixmap]() {
        const QString path = QFileDialog::getSaveFileName(this,
                                                          QStringLiteral("Enregistrer QR code"),
                                                          QStringLiteral("qr_produit.png"),
                                                          QStringLiteral("PNG (*.png)"));
        if (path.isEmpty())
            return;
        if (!qrPixmap.save(path, "PNG")) {
            QMessageBox::warning(this, QStringLiteral("QR Code"), QStringLiteral("Impossible d'enregistrer le fichier."));
            return;
        }
        QMessageBox::information(this, QStringLiteral("QR Code"), QStringLiteral("Image enregistrée :\n%1").arg(path));
    });
    connect(btnPrint, &QPushButton::clicked, &dlg, [this, qrPixmap]() {
        QPrinter printer(QPrinter::HighResolution);
        QPrintDialog printDlg(&printer, this);
        printDlg.setWindowTitle(QStringLiteral("Imprimer QR code"));
        if (printDlg.exec() != QDialog::Accepted)
            return;
        QPainter painter(&printer);
        if (!painter.isActive()) {
            QMessageBox::warning(this, QStringLiteral("QR Code"), QStringLiteral("Impression impossible."));
            return;
        }
        const QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);
        const QPixmap scaled = qrPixmap.scaled(int(pageRect.width() * 0.65),
                                               int(pageRect.height() * 0.65),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
        const QPointF topLeft((pageRect.width() - scaled.width()) / 2.0, (pageRect.height() - scaled.height()) / 2.0);
        painter.drawPixmap(topLeft, scaled);
        painter.end();
    });

    dlg.exec();
}

void MainWindow::clearProduitForm()
{
    m_produitSelectedId = -1;
    Produit::clearEditorFields(produitEditorBindings());
    setProduitQrPreview(QString());
}

void MainWindow::fillProduitFormFromTableRow(int row)
{
    const int id = Produit::fillEditorFromTableRow(produitEditorBindings(), ui->employeeTable_4, row);
    m_produitSelectedId = id > 0 ? id : -1;
    setProduitQrPreview(loadProduitQrPathFromDb(m_produitSelectedId));
}

void MainWindow::on_produitTable_cellClicked(int row, int)
{
    if (m_produitEditMode)
        return;
    fillProduitFormFromTableRow(row);
}

void MainWindow::on_employeeTable_4_cellClicked(int row, int column)
{
    if (column == 10 && ui->employeeTable_4) {
        int produitId = -1;
        if (QTableWidgetItem *idItem = ui->employeeTable_4->item(row, 0))
            produitId = Produit::parseProductIdText(idItem->text());
        const QString qrPath = loadProduitQrPathFromDb(produitId);
        QPixmap px(qrPath);
        if (!qrPath.isEmpty() && !px.isNull()) {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("QR Code produit"));
            auto *lay = new QVBoxLayout(&dlg);
            auto *img = new QLabel(&dlg);
            img->setAlignment(Qt::AlignCenter);
            img->setPixmap(px.scaled(420, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            lay->addWidget(img);
            dlg.resize(460, 480);
            dlg.exec();
        } else {
            QMessageBox::information(this, QStringLiteral("QR Code"), QStringLiteral("QR non disponible."));
        }
    }
    on_produitTable_cellClicked(row, 0);
}

void MainWindow::on_btnAjouter_6_clicked()
{
    if (m_produitEditMode) {
        on_btnModifier_4_clicked();
        return;
    }
    if (!ensureDbOpenForProduits()) {
        const QString detail = db.lastError().text().trimmed();
        QMessageBox::warning(this, QStringLiteral("Produits"),
                              QStringLiteral("Base fermee.")
                                  + (detail.isEmpty() ? QString() : QStringLiteral("\n") + detail));
        return;
    }

    QStringList errors;
    QWidget *firstInvalid = nullptr;
    bool qtyOk = false;
    const int qte = ui->lineEditAdresse_2 ? ui->lineEditAdresse_2->text().trimmed().toInt(&qtyOk) : 0;
    if (!qtyOk || qte < 0) {
        errors << QStringLiteral("- Stock : entier >= 0.");
        if (!firstInvalid) firstInvalid = ui->lineEditAdresse_2;
    }

    int nid = Produit::nextAvailableId();
    if (nid <= 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"),
                              Produit::lastSqlError.isEmpty() ? QStringLiteral("ID indisponible.") : Produit::lastSqlError);
        return;
    }
    if (ui->lineEditIdProduit) {
        bool idOk = false;
        const int typed = ui->lineEditIdProduit->text().trimmed().toInt(&idOk);
        if (idOk && typed > 0)
            nid = typed;
    }
    if (Produit::idExisteDeja(nid))
        errors << QStringLiteral("- ID %1 deja utilise.").arg(nid);
    const QString nom = ui->lineEditCIN_4 ? ui->lineEditCIN_4->text().trimmed() : QString();
    if (nom.isEmpty()) {
        errors << QStringLiteral("- Nom requis.");
        if (!firstInvalid) firstInvalid = ui->lineEditCIN_4;
    }
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"),
                             QStringLiteral("Veuillez corriger les champs suivants :\n%1").arg(errors.join(QLatin1Char('\n'))));
        if (firstInvalid)
            firstInvalid->setFocus();
        return;
    }

    Produit p(nid,
              nom,
              ui->lineEditNom_4 ? ui->lineEditNom_4->text().trimmed() : QString(),
              ui->lineEditPrenom_4 ? ui->lineEditPrenom_4->text().trimmed() : QString(),
              ui->comboBox_3 ? ui->comboBox_3->currentText() : QString(),
              qte,
              ui->comboBox_4 ? ui->comboBox_4->currentText() : QString(),
              (ui->dateTimeEdit ? ui->dateTimeEdit->date() : QDate::currentDate()),
              ui->comboBox_5 ? ui->comboBox_5->currentText() : QString(),
              ui->lineEditPrenom_5 ? ui->lineEditPrenom_5->text().trimmed() : QString());

    const QString confirmMsg = QStringLiteral("Confirmer l'ajout du produit ID %1 ?").arg(nid);
    if (QMessageBox::question(this, QStringLiteral("Produits"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    if (!p.ajouter()) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec ajout:\n%1").arg(Produit::lastSqlError));
        return;
    }

    const QString unique = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString qrData = QStringLiteral("ID:%1;Nom:%2;Cat:%3;UUID:%4")
                               .arg(nid)
                               .arg(nom)
                               .arg(ui->lineEditNom_4 ? ui->lineEditNom_4->text().trimmed() : QString())
                               .arg(unique);
    const QImage qrImage = generateQRCode(qrData);
    if (qrImage.isNull()) {
        QMessageBox::warning(this, QStringLiteral("QR Code"),
                             QStringLiteral("Produit ajoute, mais la generation du QR code a echoue."));
    } else {
        QDir().mkpath(QStringLiteral("qrcodes"));
        const QString path = QDir(QStringLiteral("qrcodes")).filePath(QString::number(nid) + QStringLiteral(".png"));
        if (!qrImage.save(path)) {
            QMessageBox::warning(this, QStringLiteral("QR Code"),
                                 QStringLiteral("Produit ajoute, mais la sauvegarde du QR code a echoue."));
        } else {
            QString qrErr;
            if (!saveProduitQrPathInDb(nid, path, &qrErr)) {
                QMessageBox::warning(this, QStringLiteral("QR Code"),
                                     QStringLiteral("Produit ajoute, mais le chemin QR n'a pas ete sauvegarde:\n%1").arg(qrErr));
            }
            setProduitQrPreview(path);
        }
    }

    refreshProduitsTable();
    Produit::clearEditorFields(produitEditorBindings());
    m_produitSelectedId = -1;
    QMessageBox::information(this, QStringLiteral("Produits"),
                             QStringLiteral("Produit ajoute avec QR code genere (ID %1).").arg(nid));
}

void MainWindow::on_btnModifier_4_clicked()
{
    if (!ensureDbOpenForProduits()) {
        const QString detail = db.lastError().text().trimmed();
        QMessageBox::warning(this, QStringLiteral("Produits"),
                              QStringLiteral("Base fermee.")
                                  + (detail.isEmpty() ? QString() : QStringLiteral("\n") + detail));
        return;
    }
    if (m_produitSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Selectionnez une ligne."));
        return;
    }

    QStringList errors;
    QWidget *firstInvalid = nullptr;
    bool newIdOk = false;
    const int newId = ui->lineEditIdProduit ? ui->lineEditIdProduit->text().trimmed().toInt(&newIdOk) : m_produitSelectedId;
    if (!newIdOk || newId <= 0) {
        errors << QStringLiteral("- ID entier > 0.");
        if (!firstInvalid) firstInvalid = ui->lineEditIdProduit;
    }
    if (newId != m_produitSelectedId && Produit::idExisteDeja(newId))
        errors << QStringLiteral("- ID %1 deja pris.").arg(newId);

    bool qtyOk = false;
    const int qte = ui->lineEditAdresse_2 ? ui->lineEditAdresse_2->text().trimmed().toInt(&qtyOk) : 0;
    if (!qtyOk || qte < 0) {
        errors << QStringLiteral("- Stock : entier >= 0.");
        if (!firstInvalid) firstInvalid = ui->lineEditAdresse_2;
    }

    const QString nom = ui->lineEditCIN_4 ? ui->lineEditCIN_4->text().trimmed() : QString();
    if (nom.isEmpty()) {
        errors << QStringLiteral("- Nom requis.");
        if (!firstInvalid) firstInvalid = ui->lineEditCIN_4;
    }
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"),
                             QStringLiteral("Veuillez corriger les champs suivants :\n%1").arg(errors.join(QLatin1Char('\n'))));
        if (firstInvalid)
            firstInvalid->setFocus();
        return;
    }

    Produit p(newId,
              nom,
              ui->lineEditNom_4 ? ui->lineEditNom_4->text().trimmed() : QString(),
              ui->lineEditPrenom_4 ? ui->lineEditPrenom_4->text().trimmed() : QString(),
              ui->comboBox_3 ? ui->comboBox_3->currentText() : QString(),
              qte,
              ui->comboBox_4 ? ui->comboBox_4->currentText() : QString(),
              (ui->dateTimeEdit ? ui->dateTimeEdit->date() : QDate::currentDate()),
              ui->comboBox_5 ? ui->comboBox_5->currentText() : QString(),
              ui->lineEditPrenom_5 ? ui->lineEditPrenom_5->text().trimmed() : QString());

    const QString confirmMsg = QStringLiteral("Confirmer la modification du produit ID %1 ?").arg(m_produitSelectedId);
    if (QMessageBox::question(this, QStringLiteral("Produits"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    if (!p.modifier(m_produitSelectedId, newId)) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec modification:\n%1").arg(Produit::lastSqlError));
        return;
    }
    m_produitSelectedId = newId;
    m_produitEditMode = false;
    m_produitEditingId = -1;
    if (ui->btnAjouter_6)
        ui->btnAjouter_6->setText(QStringLiteral("Ajouter produit"));
    if (m_produitCancelEditButton)
        m_produitCancelEditButton->hide();
    if (ui->employeeTable_4) {
        ui->employeeTable_4->setSelectionMode(QAbstractItemView::SingleSelection);
        for (int r = 0; r < ui->employeeTable_4->rowCount(); ++r) {
            if (QWidget *w = ui->employeeTable_4->cellWidget(r, 11)) w->setEnabled(true);
            if (QWidget *w = ui->employeeTable_4->cellWidget(r, 12)) w->setEnabled(true);
        }
    }
    if (ui->lineEditIdProduit)
        ui->lineEditIdProduit->setText(QString::number(newId));
    refreshProduitsTable();
    QMessageBox::information(this, QStringLiteral("Produits"), QStringLiteral("Modifie."));
}

void MainWindow::on_btnSupprimer_4_clicked()
{
    if (!ensureDbOpenForProduits()) {
        const QString detail = db.lastError().text().trimmed();
        QMessageBox::warning(this, QStringLiteral("Produits"),
                              QStringLiteral("Base fermee.")
                                  + (detail.isEmpty() ? QString() : QStringLiteral("\n") + detail));
        return;
    }
    if (m_produitSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Produits"), QStringLiteral("Selectionnez une ligne."));
        return;
    }

    const auto r = QMessageBox::question(this,
                                         QStringLiteral("Produits"),
                                         QStringLiteral("Supprimer le produit ID %1 ?").arg(m_produitSelectedId),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
    if (r != QMessageBox::Yes)
        return;

    if (!Produit().supprimer(m_produitSelectedId)) {
        QMessageBox::critical(this, QStringLiteral("Produits"),
                              QStringLiteral("Echec suppression:\n%1").arg(Produit::lastSqlError));
        return;
    }
    refreshProduitsTable();
    clearProduitForm();
    QMessageBox::information(this, QStringLiteral("Produits"), QStringLiteral("Supprime."));
}

// ------------------- Page Matieres premieres -------------------

void MainWindow::installMatieresPageResponsiveLayout()
{
    // Page matières : layout défini dans le .ui (projet leather house). Ajustements légers seulement.
    if (ui->lineEditSearch_6) {
        constexpr int kMaxW = 16777215;
        ui->lineEditSearch_6->setMinimumWidth(180);
        ui->lineEditSearch_6->setMaximumWidth(kMaxW);
        ui->lineEditSearch_6->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

void MainWindow::installEmployesPageClientLikeLayout()
{
    if (!ui->pageEmployes || ui->pageEmployes->property("clientLikeLayoutInstalled").toBool())
        return;
    if (ui->employesHeaderRow && ui->employesMainLayout) {
        for (int i = 0; i < ui->employesMainLayout->count(); ++i) {
            QLayoutItem *it = ui->employesMainLayout->itemAt(i);
            if (it && it->layout() == ui->employesHeaderRow) {
                QLayoutItem *removed = ui->employesMainLayout->takeAt(i);
                delete removed;
                break;
            }
        }
    }
    if (ui->lineEditSearch)
        ui->lineEditSearch->hide();
    if (ui->btnRechercher)
        ui->btnRechercher->hide();
    if (ui->employesTitle)
        ui->employesTitle->hide();

    if (!ui->pageEmployes->property("clientLikeTopActionsInstalled").toBool()) {
        auto *mainLay = qobject_cast<QVBoxLayout *>(ui->pageEmployes->layout());
        if (mainLay) {
            auto *topActionsCard = new QFrame(ui->pageEmployes);
            topActionsCard->setObjectName(QStringLiteral("clientTopNavBar"));
            auto *topLay = new QHBoxLayout(topActionsCard);
            topLay->setContentsMargins(12, 8, 12, 8);
            topLay->setSpacing(10);

            const QString btnStyle = QStringLiteral(
                "QPushButton {"
                "  border: 1px solid #d5dbe5;"
                "  border-radius: 8px;"
                "  background: #ffffff;"
                "  color: #000000;"
                "  font-weight: 600;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
                "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }");

            auto *btnExport = new QPushButton(QStringLiteral("Export Excel"), topActionsCard);
            btnExport->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
            btnExport->setCursor(Qt::PointingHandCursor);
            btnExport->setStyleSheet(btnStyle);
            connect(btnExport, &QPushButton::clicked, this, &MainWindow::onEmployeExportExcelClicked);

            auto *searchTop = new QLineEdit(topActionsCard);
            searchTop->setMinimumHeight(34);
            searchTop->setMinimumWidth(250);
            searchTop->setMaximumWidth(360);
            searchTop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            searchTop->setPlaceholderText(QStringLiteral("Rechercher..."));
            searchTop->setStyleSheet(QStringLiteral(
                "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QLineEdit:focus { border-color: #c08a5b; }"));

            auto *btnSearch = new QPushButton(QStringLiteral("Rechercher"), topActionsCard);
            btnSearch->setCursor(Qt::PointingHandCursor);
            btnSearch->setMinimumSize(116, 34);
            btnSearch->setMaximumWidth(116);
            btnSearch->setStyleSheet(btnStyle);

            auto *btnReset = new QPushButton(QStringLiteral("Reset"), topActionsCard);
            auto *filterTop = new QComboBox(topActionsCard);
            filterTop->setObjectName(QStringLiteral("employesTopFilterCombo"));
            filterTop->setMinimumSize(130, 34);
            filterTop->setStyleSheet(QStringLiteral(
                "QComboBox { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QComboBox:focus { border-color: #c08a5b; }"));
            filterTop->addItem(QStringLiteral("Filtre: Tous"), -1);
            filterTop->addItem(QStringLiteral("CIN"), 0);
            filterTop->addItem(QStringLiteral("Nom"), 1);
            filterTop->addItem(QStringLiteral("Prenom"), 2);
            filterTop->addItem(QStringLiteral("Poste"), 7);
            filterTop->addItem(QStringLiteral("Email"), 9);

            btnReset->setCursor(Qt::PointingHandCursor);
            btnReset->setMinimumSize(116, 34);
            btnReset->setMaximumWidth(116);
            btnReset->setStyleSheet(btnStyle);

            connect(searchTop, &QLineEdit::textChanged, this, [this](const QString &text) {
                if (ui->lineEditSearch)
                    ui->lineEditSearch->setText(text);
            });
            connect(searchTop, &QLineEdit::returnPressed, this, &MainWindow::onEmployeRechercherClicked);
            connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onEmployeRechercherClicked);
            connect(filterTop, &QComboBox::currentIndexChanged, this, [this](int) { onEmployeRechercherClicked(); });
            connect(btnReset, &QPushButton::clicked, this, [this, searchTop]() {
                searchTop->clear();
                if (ui->lineEditSearch)
                    ui->lineEditSearch->clear();
                onEmployeRechercherClicked();
            });

            topLay->addWidget(btnExport);
            topLay->addStretch(1);
            topLay->addWidget(filterTop);
            topLay->addWidget(searchTop, 1);
            topLay->addWidget(btnSearch);
            topLay->addWidget(btnReset);
            mainLay->insertWidget(0, topActionsCard, 0);
        }
        ui->pageEmployes->setProperty("clientLikeTopActionsInstalled", true);
    }

    auto *content = ui->pageEmployes->findChild<QHBoxLayout *>(QStringLiteral("employesContentLayout"));
    if (!content || !ui->employeeTable || !ui->employeeFormBox)
        return;

    if (QLayoutItem *oldLeft = content->takeAt(0))
        delete oldLeft;
    if (ui->employesRightPanel) {
        content->removeWidget(ui->employesRightPanel);
        ui->employesRightPanel->hide();
    }
    content->setSpacing(14);

    auto *leftCard = new QWidget(ui->pageEmployes);
    leftCard->setObjectName(QStringLiteral("clientFormCard"));
    leftCard->setMinimumWidth(300);
    leftCard->setMaximumWidth(380);
    leftCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *leftLay = new QVBoxLayout(leftCard);
    leftLay->setContentsMargins(12, 12, 12, 12);
    leftLay->setSpacing(10);

    auto *rightZone = new QWidget(ui->pageEmployes);
    rightZone->setObjectName(QStringLiteral("clientTableZone"));
    rightZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightLay = new QVBoxLayout(rightZone);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(10);

    if (!leftCard->property("employeFormWrapped").toBool()) {
        auto *sa = new QScrollArea(leftCard);
        sa->setObjectName(QStringLiteral("employeFormScrollArea"));
        sa->setWidgetResizable(true);
        sa->setFrameShape(QFrame::StyledPanel);
        sa->setFrameShadow(QFrame::Plain);
        sa->setStyleSheet(QStringLiteral(
            "QScrollArea { border: 1px solid rgb(230, 220, 200); border-radius: 10px; background: rgb(255, 255, 255); }"
            "QScrollArea > QWidget > QWidget { background: #ffffff; }"));
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->employeeFormBox->setParent(sa);
        sa->setWidget(ui->employeeFormBox);
        ui->employeeFormBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        leftLay->addWidget(sa, 1);
        leftCard->setProperty("employeFormWrapped", true);
    }

    ui->employeeTable->setParent(rightZone);
    ui->employeeTable->setMinimumSize(520, 320);
    ui->employeeTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLay->addWidget(ui->employeeTable, 1);

    content->addWidget(leftCard, 0);
    content->addWidget(rightZone, 1);
    ui->pageEmployes->setProperty("clientLikeLayoutInstalled", true);
}

void MainWindow::installFournisseursPageClientLikeLayout()
{
    if (!ui->page || ui->page->property("clientLikeLayoutInstalled").toBool())
        return;
    if (ui->employesHeaderRow_2 && ui->fournisseursMainLayout) {
        for (int i = 0; i < ui->fournisseursMainLayout->count(); ++i) {
            QLayoutItem *it = ui->fournisseursMainLayout->itemAt(i);
            if (it && it->layout() == ui->employesHeaderRow_2) {
                QLayoutItem *removed = ui->fournisseursMainLayout->takeAt(i);
                delete removed;
                break;
            }
        }
    }
    if (ui->employesTitle_2)
        ui->employesTitle_2->hide();
    if (ui->lineEditSearch_2)
        ui->lineEditSearch_2->hide();
    if (ui->btnRechercher_6)
        ui->btnRechercher_6->hide();
    if (QComboBox *combo = ui->page->findChild<QComboBox *>(QStringLiteral("comboFiltreFournisseur")))
        combo->hide();

    if (!ui->page->property("clientLikeTopActionsInstalled").toBool()) {
        auto *mainLay = qobject_cast<QVBoxLayout *>(ui->page->layout());
        if (mainLay) {
            auto *topActionsCard = new QFrame(ui->page);
            topActionsCard->setObjectName(QStringLiteral("clientTopNavBar"));
            auto *topLay = new QHBoxLayout(topActionsCard);
            topLay->setContentsMargins(12, 8, 12, 8);
            topLay->setSpacing(10);

            const QString btnStyle = QStringLiteral(
                "QPushButton {"
                "  border: 1px solid transparent;"
                "  border-radius: 8px;"
                "  background: #f3f5f8;"
                "  color: #1f2937;"
                "  font-weight: 600;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
                "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }");

            auto *btnExport = new QPushButton(QStringLiteral("Export"), topActionsCard);
            btnExport->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
            btnExport->setCursor(Qt::PointingHandCursor);
            btnExport->setStyleSheet(btnStyle);
            connect(btnExport, &QPushButton::clicked, this, [this]() {
                if (ui->btnAjouter_2)
                    ui->btnAjouter_2->click();
            });

            auto *searchTop = new QLineEdit(topActionsCard);
            searchTop->setMinimumHeight(34);
            searchTop->setMinimumWidth(250);
            searchTop->setMaximumWidth(360);
            searchTop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            searchTop->setPlaceholderText(QStringLiteral("Rechercher..."));
            searchTop->setStyleSheet(QStringLiteral(
                "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QLineEdit:focus { border-color: #c08a5b; }"));

            auto *btnSearch = new QPushButton(QStringLiteral("Rechercher"), topActionsCard);
            btnSearch->setCursor(Qt::PointingHandCursor);
            btnSearch->setMinimumSize(116, 34);
            btnSearch->setMaximumWidth(116);
            btnSearch->setStyleSheet(btnStyle);

            auto *btnReset = new QPushButton(QStringLiteral("Reset"), topActionsCard);
            btnReset->setCursor(Qt::PointingHandCursor);
            btnReset->setMinimumSize(116, 34);
            btnReset->setMaximumWidth(116);
            btnReset->setStyleSheet(btnStyle);

            auto *filterTop = new QComboBox(topActionsCard);
            filterTop->setObjectName(QStringLiteral("fournisseursTopFilterCombo"));
            filterTop->setMinimumSize(130, 34);
            filterTop->setStyleSheet(QStringLiteral(
                "QComboBox { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QComboBox:focus { border-color: #c08a5b; }"));
            filterTop->addItem(QStringLiteral("Filtre: Tous"), -1);
            filterTop->addItem(QStringLiteral("Code"), 0);
            filterTop->addItem(QStringLiteral("Raison sociale"), 1);
            filterTop->addItem(QStringLiteral("Zone"), 4);
            filterTop->addItem(QStringLiteral("Email"), 3);

            const auto applyFournisseursTopFilter = [this, searchTop, filterTop]() {
                if (!ui->employeeTable_2)
                    return;
                const int col = filterTop->currentData().toInt();
                applyTopColumnFilter(ui->employeeTable_2, searchTop->text(), col);
            };

            connect(searchTop, &QLineEdit::textChanged, this, [this](const QString &text) {
                if (ui->lineEditSearch_2)
                    ui->lineEditSearch_2->setText(text);
            });
            connect(btnSearch, &QPushButton::clicked, this, [this]() {
                if (ui->btnRechercher_6)
                    ui->btnRechercher_6->click();
            });
            connect(searchTop, &QLineEdit::returnPressed, this, [this]() {
                if (ui->btnRechercher_6)
                    ui->btnRechercher_6->click();
            });
            connect(btnReset, &QPushButton::clicked, this, [this, searchTop]() {
                searchTop->clear();
                if (ui->lineEditSearch_2)
                    ui->lineEditSearch_2->clear();
                if (ui->btnRechercher_6)
                    ui->btnRechercher_6->click();
            });
            connect(btnSearch, &QPushButton::clicked, this, applyFournisseursTopFilter);
            connect(searchTop, &QLineEdit::returnPressed, this, applyFournisseursTopFilter);
            connect(filterTop, &QComboBox::currentIndexChanged, this, [this, applyFournisseursTopFilter](int) {
                if (ui->btnRechercher_6)
                    ui->btnRechercher_6->click();
                applyFournisseursTopFilter();
            });
            connect(btnReset, &QPushButton::clicked, this, applyFournisseursTopFilter);

            topLay->addWidget(btnExport);
            topLay->addStretch(1);
            topLay->addWidget(filterTop);
            topLay->addWidget(searchTop, 1);
            topLay->addWidget(btnSearch);
            topLay->addWidget(btnReset);
            mainLay->insertWidget(0, topActionsCard, 0);
        }
        ui->page->setProperty("clientLikeTopActionsInstalled", true);
    }
    auto *content = ui->page->findChild<QHBoxLayout *>(QStringLiteral("fournisseursContentLayout"));
    if (!content || !ui->employeeTable_2 || !ui->employeeFormBox_2)
        return;

    if (QLayoutItem *oldLeft = content->takeAt(0))
        delete oldLeft;
    if (ui->fournisseursRightPanel) {
        content->removeWidget(ui->fournisseursRightPanel);
        ui->fournisseursRightPanel->hide();
    }
    content->setSpacing(14);

    auto *leftCard = new QWidget(ui->page);
    leftCard->setObjectName(QStringLiteral("clientFormCard"));
    leftCard->setMinimumWidth(300);
    leftCard->setMaximumWidth(380);
    leftCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *leftLay = new QVBoxLayout(leftCard);
    leftLay->setContentsMargins(12, 12, 12, 12);
    leftLay->setSpacing(10);

    auto *rightZone = new QWidget(ui->page);
    rightZone->setObjectName(QStringLiteral("clientTableZone"));
    rightZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightLay = new QVBoxLayout(rightZone);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(10);

    if (!leftCard->property("fournisseurFormWrapped").toBool()) {
        auto *sa = new QScrollArea(leftCard);
        sa->setObjectName(QStringLiteral("fournisseurFormScrollArea"));
        sa->setWidgetResizable(true);
        sa->setFrameShape(QFrame::StyledPanel);
        sa->setFrameShadow(QFrame::Plain);
        sa->setStyleSheet(QStringLiteral(
            "QScrollArea { border: 1px solid rgb(230, 220, 200); border-radius: 10px; background: rgb(255, 255, 255); }"
            "QScrollArea > QWidget > QWidget { background: #ffffff; }"));
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->employeeFormBox_2->setParent(sa);
        sa->setWidget(ui->employeeFormBox_2);
        ui->employeeFormBox_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        leftLay->addWidget(sa, 1);
        leftCard->setProperty("fournisseurFormWrapped", true);
    }

    ui->employeeTable_2->setParent(rightZone);
    ui->employeeTable_2->setMinimumSize(520, 320);
    ui->employeeTable_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLay->addWidget(ui->employeeTable_2, 1);
    setupSmartMapFournisseursUi();
    if (QWidget *smartMapBlock = ui->page->findChild<QWidget *>(QStringLiteral("smartMapBlock"))) {
        smartMapBlock->hide();
    }

    content->addWidget(leftCard, 0);
    content->addWidget(rightZone, 1);
    ui->page->setProperty("clientLikeLayoutInstalled", true);
}

void MainWindow::installMatieresPageClientLikeLayout()
{
    if (!ui->page_3 || ui->page_3->property("clientLikeLayoutInstalled").toBool())
        return;
    if (ui->matieresHeaderRow && ui->matieresMainLayout) {
        for (int i = 0; i < ui->matieresMainLayout->count(); ++i) {
            QLayoutItem *it = ui->matieresMainLayout->itemAt(i);
            if (it && it->layout() == ui->matieresHeaderRow) {
                QLayoutItem *removed = ui->matieresMainLayout->takeAt(i);
                delete removed;
                break;
            }
        }
    }
    if (ui->employesTitle_5)
        ui->employesTitle_5->hide();
    if (ui->comboBoxTri_mp)
        ui->comboBoxTri_mp->hide();
    if (ui->comboBoxOrdre_mp)
        ui->comboBoxOrdre_mp->hide();
    if (ui->lineEditSearch_6)
        ui->lineEditSearch_6->hide();
    if (ui->btnRechercher_5)
        ui->btnRechercher_5->hide();

    if (!ui->page_3->property("clientLikeTopActionsInstalled").toBool()) {
        auto *mainLay = qobject_cast<QVBoxLayout *>(ui->page_3->layout());
        if (mainLay) {
            auto *topActionsCard = new QFrame(ui->page_3);
            topActionsCard->setObjectName(QStringLiteral("clientTopNavBar"));
            auto *topLay = new QHBoxLayout(topActionsCard);
            topLay->setContentsMargins(12, 8, 12, 8);
            topLay->setSpacing(10);

            const QString btnStyle = QStringLiteral(
                "QPushButton {"
                "  border: 1px solid transparent;"
                "  border-radius: 8px;"
                "  background: #f3f5f8;"
                "  color: #1f2937;"
                "  font-weight: 600;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
                "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }");

            auto *searchTop = new QLineEdit(topActionsCard);
            searchTop->setMinimumHeight(34);
            searchTop->setMinimumWidth(260);
            searchTop->setMaximumWidth(360);
            searchTop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            searchTop->setPlaceholderText(QStringLiteral("Rechercher..."));
            searchTop->setStyleSheet(QStringLiteral(
                "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QLineEdit:focus { border-color: #c08a5b; }"));

            auto *btnSearch = new QPushButton(QStringLiteral("Rechercher"), topActionsCard);
            btnSearch->setCursor(Qt::PointingHandCursor);
            btnSearch->setMinimumSize(116, 34);
            btnSearch->setMaximumWidth(116);
            btnSearch->setStyleSheet(btnStyle);

            auto *btnReset = new QPushButton(QStringLiteral("Reset"), topActionsCard);
            btnReset->setCursor(Qt::PointingHandCursor);
            btnReset->setMinimumSize(116, 34);
            btnReset->setMaximumWidth(116);
            btnReset->setStyleSheet(btnStyle);

            auto *btnBusySeason = new QPushButton(QStringLiteral("Busy Season"), topActionsCard);
            btnBusySeason->setCursor(Qt::PointingHandCursor);
            btnBusySeason->setMinimumSize(116, 34);
            btnBusySeason->setMaximumWidth(140);
            btnBusySeason->setStyleSheet(btnStyle);
            btnBusySeason->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));

            auto *btnAlerteMp = new QPushButton(QStringLiteral("Alerte MP"), topActionsCard);
            btnAlerteMp->setCursor(Qt::PointingHandCursor);
            btnAlerteMp->setMinimumSize(116, 34);
            btnAlerteMp->setMaximumWidth(116);
            btnAlerteMp->setStyleSheet(btnStyle);
            btnAlerteMp->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));

            auto *filterTop = new QComboBox(topActionsCard);
            filterTop->setObjectName(QStringLiteral("matieresTopFilterCombo"));
            filterTop->setMinimumSize(130, 34);
            filterTop->setStyleSheet(QStringLiteral(
                "QComboBox { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
                "QComboBox:focus { border-color: #c08a5b; }"));
            filterTop->addItem(QStringLiteral("Filtre: Tous"), -1);
            filterTop->addItem(QStringLiteral("ID"), 0);
            filterTop->addItem(QStringLiteral("Reference"), 1);
            filterTop->addItem(QStringLiteral("Nom cuir"), 2);
            filterTop->addItem(QStringLiteral("Type cuir"), 3);
            filterTop->addItem(QStringLiteral("Gamme"), 4);
            filterTop->addItem(QStringLiteral("Statut"), 12);

            connect(searchTop, &QLineEdit::textChanged, this, [this](const QString &text) {
                if (ui->lineEditSearch_6)
                    ui->lineEditSearch_6->setText(text);
            });
            connect(searchTop, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_5_clicked);
            connect(btnSearch, &QPushButton::clicked, this, &MainWindow::on_btnRechercher_5_clicked);
            connect(filterTop, &QComboBox::currentIndexChanged, this, [this](int) { on_btnRechercher_5_clicked(); });
            connect(btnBusySeason, &QPushButton::clicked, this, &MainWindow::onOpenBusySeasonCalendar);
            connect(btnAlerteMp, &QPushButton::clicked, this, &MainWindow::sendStockAlert);
            connect(btnReset, &QPushButton::clicked, this, [this, searchTop]() {
                searchTop->clear();
                if (ui->lineEditSearch_6)
                    ui->lineEditSearch_6->clear();
                on_btnRechercher_5_clicked();
            });

            auto *btnExportMp = new QPushButton(QStringLiteral("Export Excel"), topActionsCard);
            btnExportMp->setObjectName(QStringLiteral("matieresTopExportExcelBtn"));
            btnExportMp->setCursor(Qt::PointingHandCursor);
            btnExportMp->setMinimumSize(116, 34);
            btnExportMp->setMaximumWidth(140);
            btnExportMp->setStyleSheet(btnStyle);
            btnExportMp->setToolTip(QStringLiteral(
                "Exporter tout le tableau des matières premières vers un fichier Excel (.xls), comme pour les clients."));
            connect(btnExportMp, &QPushButton::clicked, this, &MainWindow::onExporterMatieresPremieresClicked);

            topLay->addWidget(btnBusySeason);
            topLay->addWidget(btnAlerteMp);
            topLay->addStretch(1);
            topLay->addWidget(filterTop);
            topLay->addWidget(searchTop, 1);
            topLay->addWidget(btnSearch);
            topLay->addWidget(btnReset);
            topLay->addWidget(btnExportMp);
            mainLay->insertWidget(0, topActionsCard, 0);
        }
        ui->page_3->setProperty("clientLikeTopActionsInstalled", true);
    }

    if (QFrame *oauthCard = ui->page_3->findChild<QFrame *>(QStringLiteral("googleOAuthCard"))) {
        oauthCard->hide();
        oauthCard->deleteLater();
    }
    m_googleCalendarWidget = nullptr;
    m_googleCalendarEventsList = nullptr;

    auto *content = ui->page_3->findChild<QHBoxLayout *>(QStringLiteral("matieresContentLayout"));
    if (!content || !ui->employeeTable_5 || !ui->employeeFormBox_5)
        return;

    if (QLayoutItem *oldLeft = content->takeAt(0))
        delete oldLeft;
    if (ui->matieresRightPanel_mp) {
        content->removeWidget(ui->matieresRightPanel_mp);
        ui->matieresRightPanel_mp->hide();
    }
    content->setSpacing(14);

    auto *leftCard = new QWidget(ui->page_3);
    leftCard->setObjectName(QStringLiteral("clientFormCard"));
    leftCard->setMinimumWidth(300);
    leftCard->setMaximumWidth(380);
    leftCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *leftLay = new QVBoxLayout(leftCard);
    leftLay->setContentsMargins(12, 12, 12, 12);
    leftLay->setSpacing(10);

    auto *rightZone = new QWidget(ui->page_3);
    rightZone->setObjectName(QStringLiteral("clientTableZone"));
    rightZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightLay = new QVBoxLayout(rightZone);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(10);

    if (!leftCard->property("matiereFormWrapped").toBool()) {
        auto *sa = new QScrollArea(leftCard);
        sa->setObjectName(QStringLiteral("matiereFormScrollArea"));
        sa->setWidgetResizable(true);
        sa->setFrameShape(QFrame::StyledPanel);
        sa->setFrameShadow(QFrame::Plain);
        sa->setStyleSheet(QStringLiteral(
            "QScrollArea { border: 1px solid rgb(230, 220, 200); border-radius: 10px; background: rgb(255, 255, 255); }"
            "QScrollArea > QWidget > QWidget { background: #ffffff; }"));
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->employeeFormBox_5->setParent(sa);
        sa->setWidget(ui->employeeFormBox_5);
        ui->employeeFormBox_5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        leftLay->addWidget(sa, 1);
        leftCard->setProperty("matiereFormWrapped", true);
    }

    ui->employeeTable_5->setParent(rightZone);
    ui->employeeTable_5->setMinimumSize(520, 320);
    ui->employeeTable_5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLay->addWidget(ui->employeeTable_5, 1);

    content->addWidget(leftCard, 0);
    content->addWidget(rightZone, 1);
    ui->page_3->setProperty("clientLikeLayoutInstalled", true);
}

void MainWindow::applyMatieresViewFilters()
{
    if (!ui->employeeTable_5)
        return;
    const QString n = ui->lineEditSearch_6 ? ui->lineEditSearch_6->text().trimmed().toUpper() : QString();
    for (int r = 0; r < ui->employeeTable_5->rowCount(); ++r) {
        bool show = n.isEmpty();
        if (!show) {
            for (int c = 0; c < ui->employeeTable_5->columnCount(); ++c) {
                if (QTableWidgetItem *it = ui->employeeTable_5->item(r, c)) {
                    if (it->text().toUpper().contains(n)) {
                        show = true;
                        break;
                    }
                }
            }
        }
        if (show && m_mpFilterDisponible) {
            const QString st = ui->employeeTable_5->item(r, 12) ? ui->employeeTable_5->item(r, 12)->text() : QString();
            show = st.contains(QStringLiteral("DISPONIBLE"), Qt::CaseInsensitive);
        }
        if (show && m_mpSeuilCritique >= 0) {
            bool ok = false;
            const int res = ui->employeeTable_5->item(r, 9) ? ui->employeeTable_5->item(r, 9)->text().toInt(&ok) : 0;
            show = ok && res <= m_mpSeuilCritique;
        }
        ui->employeeTable_5->setRowHidden(r, !show);
    }
}

void MainWindow::applyMatieresTableSortIfNeeded()
{
    if (!ui->employeeTable_5 || !ui->comboBoxTri_mp)
        return;
    const int triIdx = ui->comboBoxTri_mp->currentIndex();
    if (triIdx <= 0)
        return;
    // Ordre du combo (leather house) : Nom, Gamme, Épaisseur, Quantité stock, Type de cuir
    static const int kCols[] = {-1, 2, 4, 6, 9, 3};
    if (triIdx < 1 || triIdx >= int(sizeof(kCols) / sizeof(kCols[0])))
        return;
    const int col = kCols[triIdx];
    const Qt::SortOrder ord = (ui->comboBoxOrdre_mp && ui->comboBoxOrdre_mp->currentIndex() == 1)
                                   ? Qt::DescendingOrder
                                   : Qt::AscendingOrder;
    ui->employeeTable_5->sortItems(col, ord);
}

void MainWindow::openEmployesModule()
{
    ui->contentStack->setCurrentIndex(1);
    setActiveButton(ui->btnEmployes);
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Employes"),
                             QStringLiteral("Connexion Oracle impossible. Verifiez le DSN ODBC (projet_cuir)."));
        return;
    }
    QString err;
    if (!Employe::ensureSchema(&err)) {
        QMessageBox::critical(this, QStringLiteral("Employes"),
                              QStringLiteral("Preparation du schema EMPLOYES :\n%1").arg(err));
        return;
    }
    Employe::seedDemoIfEmpty(&err);
    refreshEmployesTable();
}

EmployeEditorWidgets MainWindow::employeEditorBindings() const
{
    EmployeEditorWidgets w;
    w.cin = ui->lineEditCIN;
    w.nom = ui->lineEditNom;
    w.prenom = ui->lineEditPrenom;
    w.sexe = ui->comboBoxSexe;
    w.salaire = ui->lineEditSalaire;
    w.dateEmbauche = ui->dateEditEmbauche;
    w.telephone = ui->lineEditTelephone;
    w.poste = ui->lineEditPoste;
    w.adresse = ui->lineEditAdresse;
    w.email = ui->lineEditEmail;
    return w;
}

void MainWindow::setEmployeFormReadOnlyCin(bool readOnly)
{
    if (ui->lineEditCIN)
        ui->lineEditCIN->setReadOnly(readOnly);
}

void MainWindow::clearEmployeForm()
{
    Employe::clearEditor(employeEditorBindings());
    setEmployeFormReadOnlyCin(false);
    if (ui->btnModifier)
        ui->btnModifier->setEnabled(false);
    if (ui->btnSupprimer)
        ui->btnSupprimer->setEnabled(false);
}

void MainWindow::refreshEmployesTable()
{
    if (!ui->employeeTable || !db.isOpen())
        return;
    QString err;
    if (!Employe::populateTable(ui->employeeTable, &err)) {
        QMessageBox::warning(this, QStringLiteral("Employes"),
                             QStringLiteral("Chargement :\n%1").arg(err));
        return;
    }
    Employe::applySearchFilter(ui->employeeTable,
                               ui->lineEditSearch ? ui->lineEditSearch->text() : QString());

    if (ui->employeeTable->columnCount() < 12)
        ui->employeeTable->setColumnCount(12);
    ui->employeeTable->setHorizontalHeaderItem(10, new QTableWidgetItem(QStringLiteral("SUPPR.")));
    ui->employeeTable->setHorizontalHeaderItem(11, new QTableWidgetItem(QStringLiteral("MODIF.")));

    for (int row = 0; row < ui->employeeTable->rowCount(); ++row) {
        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), ui->employeeTable);
        deleteBtn->setToolTip(QStringLiteral("Supprimer cet employe"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            ui->employeeTable->setCurrentCell(row, 0);
            onEmployeSelectionChanged();
            onEmployeSupprimerClicked();
        });
        ui->employeeTable->setCellWidget(row, 10, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), ui->employeeTable);
        updateBtn->setToolTip(QStringLiteral("Modifier cet employe"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(updateBtn);
        connect(updateBtn, &QPushButton::clicked, this, [this, row]() {
            ui->employeeTable->setCurrentCell(row, 0);
            onEmployeSelectionChanged();
            m_employeEditMode = true;
            m_employeEditingCin = ui->lineEditCIN ? ui->lineEditCIN->text().trimmed() : QString();
            if (ui->btnAjouter)
                ui->btnAjouter->setText(QStringLiteral("Enregistrer"));
            if (ui->btnAjouter && m_employeCancelEditButton)
                ui->btnAjouter->setMinimumSize(m_employeCancelEditButton->minimumSize());
            if (m_employeCancelEditButton)
                m_employeCancelEditButton->show();
            if (ui->employeeTable) {
                ui->employeeTable->setSelectionMode(QAbstractItemView::NoSelection);
                for (int r = 0; r < ui->employeeTable->rowCount(); ++r) {
                    if (QWidget *w = ui->employeeTable->cellWidget(r, 10)) w->setEnabled(false);
                    if (QWidget *w = ui->employeeTable->cellWidget(r, 11)) w->setEnabled(false);
                }
            }
        });
        ui->employeeTable->setCellWidget(row, 11, updateBtn);
    }

    updateEmployesStatsPanel();
    onStatisticsDataChanged();
}

void MainWindow::updateEmployesStatsPanel()
{
    if (!ui->employeeTable)
        return;
    int total = 0;
    double sumSalaire = 0.0;
    int countSalaire = 0;
    int nouveauxMois = 0;
    const QDate now = QDate::currentDate();

    for (int r = 0; r < ui->employeeTable->rowCount(); ++r) {
        if (ui->employeeTable->isRowHidden(r))
            continue;
        ++total;

        bool ok = false;
        const double sal = ui->employeeTable->item(r, 4)
            ? ui->employeeTable->item(r, 4)->text().replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok)
            : 0.0;
        if (ok) {
            sumSalaire += sal;
            ++countSalaire;
        }

        const QString dateTxt = ui->employeeTable->item(r, 5) ? ui->employeeTable->item(r, 5)->text() : QString();
        const QDate d = QDate::fromString(dateTxt, QStringLiteral("dd/MM/yyyy"));
        if (d.isValid() && d.year() == now.year() && d.month() == now.month())
            ++nouveauxMois;
    }

    if (ui->statValue1)
        ui->statValue1->setText(QString::number(total));
    if (ui->statValue2)
        ui->statValue2->setText(countSalaire > 0 ? QString::number(sumSalaire / countSalaire, 'f', 2)
                                                 : QStringLiteral("0"));
    if (ui->statValue3)
        ui->statValue3->setText(QString::number(nouveauxMois));
}

void MainWindow::setupEmployePage()
{
    if (!ui->employeeTable || !ui->employeeFormBox)
        return;
    if (m_employeUiWired)
        return;
    m_employeUiWired = true;

    if (!ui->employeeFormBox->property("employeFicheModernized").toBool()) {
        ui->employeeFormBox->setTitle(QString());
        applyFormStyle(ui->employeeFormBox);

        if (ui->formRow1) {
            ui->formOuterLayout->removeItem(ui->formRow1);
            delete ui->formRow1;
            ui->formRow1 = nullptr;
        }
        if (ui->formRow2) {
            ui->formOuterLayout->removeItem(ui->formRow2);
            delete ui->formRow2;
            ui->formRow2 = nullptr;
        }

        const QList<QWidget *> legacyLabels = {
            ui->labelCIN, ui->labelNom, ui->labelPrenom, ui->labelSexe, ui->labelSalaire,
            ui->labelDateEmbauche, ui->labelTelephone, ui->labelPoste, ui->labelAdresse, ui->labelEmail
        };
        for (QWidget *w : legacyLabels) {
            if (w)
                w->hide();
        }

        auto *titleLab = new QLabel(QStringLiteral("Fiche employe"), ui->employeeFormBox);
        titleLab->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #000000; padding: 2px 4px;"));
        ui->formOuterLayout->insertWidget(0, titleLab);

        auto *formWrap = new QWidget(ui->employeeFormBox);
        formWrap->setStyleSheet(QStringLiteral("background:#ffffff;"));
        auto *formLay = new QFormLayout(formWrap);
        formLay->setContentsMargins(4, 2, 4, 4);
        formLay->setHorizontalSpacing(12);
        formLay->setVerticalSpacing(12);
        formLay->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        formLay->setFormAlignment(Qt::AlignTop);
        formLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        if (ui->lineEditCIN) ui->lineEditCIN->setPlaceholderText(QStringLiteral("Entrez votre CIN"));
        if (ui->lineEditNom) ui->lineEditNom->setPlaceholderText(QStringLiteral("Entrez votre nom"));
        if (ui->lineEditPrenom) ui->lineEditPrenom->setPlaceholderText(QStringLiteral("Entrez votre prenom"));
        if (ui->lineEditAdresse) ui->lineEditAdresse->setPlaceholderText(QStringLiteral("Entrez votre adresse"));
        if (ui->lineEditTelephone) ui->lineEditTelephone->setPlaceholderText(QStringLiteral("Numero de telephone"));
        if (ui->lineEditEmail) ui->lineEditEmail->setPlaceholderText(QStringLiteral("Entrez votre email"));
        if (ui->lineEditPoste) ui->lineEditPoste->setPlaceholderText(QStringLiteral("Entrez votre poste"));
        if (ui->lineEditSalaire) ui->lineEditSalaire->setPlaceholderText(QStringLiteral("Salaire"));
        const QList<QWidget *> employeFields = {
            static_cast<QWidget *>(ui->lineEditCIN),
            static_cast<QWidget *>(ui->lineEditNom),
            static_cast<QWidget *>(ui->lineEditPrenom),
            static_cast<QWidget *>(ui->lineEditAdresse),
            static_cast<QWidget *>(ui->lineEditTelephone),
            static_cast<QWidget *>(ui->lineEditEmail),
            static_cast<QWidget *>(ui->lineEditPoste),
            static_cast<QWidget *>(ui->comboBoxSexe),
            static_cast<QWidget *>(ui->dateEditEmbauche),
            static_cast<QWidget *>(ui->lineEditSalaire),
        };
        for (QWidget *w : employeFields) {
            if (!w)
                continue;
            w->setMinimumHeight(34);
            w->setMaximumWidth(220);
            w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }

        formLay->addRow(new QLabel(QStringLiteral("CIN"), formWrap), ui->lineEditCIN);
        formLay->addRow(new QLabel(QStringLiteral("Nom"), formWrap), ui->lineEditNom);
        formLay->addRow(new QLabel(QStringLiteral("Prenom"), formWrap), ui->lineEditPrenom);
        formLay->addRow(new QLabel(QStringLiteral("Adresse"), formWrap), ui->lineEditAdresse);
        formLay->addRow(new QLabel(QStringLiteral("Telephone"), formWrap), ui->lineEditTelephone);
        formLay->addRow(new QLabel(QStringLiteral("E-mail"), formWrap), ui->lineEditEmail);
        formLay->addRow(new QLabel(QStringLiteral("Poste"), formWrap), ui->lineEditPoste);
        formLay->addRow(new QLabel(QStringLiteral("Sexe"), formWrap), ui->comboBoxSexe);
        formLay->addRow(new QLabel(QStringLiteral("Date embauche"), formWrap), ui->dateEditEmbauche);
        formLay->addRow(new QLabel(QStringLiteral("Salaire"), formWrap), ui->lineEditSalaire);
        ui->formOuterLayout->insertWidget(1, formWrap);

        if (ui->formBtnLayout) {
            ui->formBtnLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->formBtnLayout->setSpacing(10);
            for (int i = ui->formBtnLayout->count() - 1; i >= 0; --i) {
                QLayoutItem *it = ui->formBtnLayout->itemAt(i);
                if (it && it->spacerItem()) {
                    QLayoutItem *removed = ui->formBtnLayout->takeAt(i);
                    delete removed;
                }
            }
        }
        if (ui->btnAjouter) {
            applyUnifiedAddButtonStyle(ui->btnAjouter);
        }
        if (!m_employeCancelEditButton && ui->formBtnLayout && ui->pageEmployes) {
            m_employeCancelEditButton = new QPushButton(QStringLiteral("Annuler"), ui->pageEmployes);
            m_employeCancelEditButton->setCursor(Qt::PointingHandCursor);
            m_employeCancelEditButton->setMinimumSize(110, 32);
            if (ui->btnAjouter)
                m_employeCancelEditButton->setStyleSheet(ui->btnAjouter->styleSheet());
            m_employeCancelEditButton->hide();
            ui->formBtnLayout->addWidget(m_employeCancelEditButton);
            connect(m_employeCancelEditButton, &QPushButton::clicked, this, [this]() {
                m_employeEditMode = false;
                m_employeEditingCin.clear();
                if (ui->btnAjouter)
                    ui->btnAjouter->setText(QStringLiteral("Ajouter"));
                if (m_employeCancelEditButton)
                    m_employeCancelEditButton->hide();
                if (ui->employeeTable) {
                    ui->employeeTable->setSelectionMode(QAbstractItemView::SingleSelection);
                    for (int r = 0; r < ui->employeeTable->rowCount(); ++r) {
                        if (QWidget *w = ui->employeeTable->cellWidget(r, 10)) w->setEnabled(true);
                        if (QWidget *w = ui->employeeTable->cellWidget(r, 11)) w->setEnabled(true);
                    }
                }
            });
        }
        if (ui->btnModifier) {
            ui->btnModifier->hide();
            if (ui->formBtnLayout)
                ui->formBtnLayout->removeWidget(ui->btnModifier);
        }
        if (ui->btnSupprimer) {
            ui->btnSupprimer->hide();
            if (ui->formBtnLayout)
                ui->formBtnLayout->removeWidget(ui->btnSupprimer);
        }
        if (ui->btnExportPDF) {
            ui->btnExportPDF->hide();
            if (ui->formBtnLayout)
                ui->formBtnLayout->removeWidget(ui->btnExportPDF);
        }
        ui->employeeFormBox->setProperty("employeFicheModernized", true);
    }

    ui->employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->employeeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->employeeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->employeeTable->setSortingEnabled(true);

    if (ui->lineEditSearch) {
        ui->lineEditSearch->setPlaceholderText(QStringLiteral("Rechercher..."));
    }
    if (ui->lineEditCIN)
        ui->lineEditCIN->setPlaceholderText(QStringLiteral("8 chiffres"));
    if (ui->lineEditTelephone)
        ui->lineEditTelephone->setPlaceholderText(QStringLiteral("8 chiffres"));

    if (ui->dateEditEmbauche)
        ui->dateEditEmbauche->setMaximumDate(QDate::currentDate());

    Employe::installInputValidators(employeEditorBindings());

    if (ui->btnModifier)
        ui->btnModifier->setEnabled(false);
    if (ui->btnSupprimer)
        ui->btnSupprimer->setEnabled(false);

    connect(ui->employeeTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onEmployeSelectionChanged);
    connect(ui->btnAjouter, &QPushButton::clicked, this, &MainWindow::onEmployeAjouterClicked);
    connect(ui->btnModifier, &QPushButton::clicked, this, &MainWindow::onEmployeModifierClicked);
    connect(ui->btnSupprimer, &QPushButton::clicked, this, &MainWindow::onEmployeSupprimerClicked);
    connect(ui->btnRechercher, &QPushButton::clicked, this, &MainWindow::onEmployeRechercherClicked);
    if (ui->lineEditSearch)
        connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::onEmployeSearchTextChanged);
    connect(ui->btnExportPDF, &QPushButton::clicked, this, &MainWindow::onEmployeExportPdfClicked);

    if (ui->formBtnLayout && !m_employeExportExcelBtn) {
        m_employeExportExcelBtn = new QPushButton(QStringLiteral("Export Excel"), ui->employeeFormBox);
        m_employeExportExcelBtn->setMinimumSize(QSize(110, 32));
        m_employeExportExcelBtn->setCursor(Qt::PointingHandCursor);
        if (ui->btnExportPDF)
            m_employeExportExcelBtn->setStyleSheet(ui->btnExportPDF->styleSheet());
        ui->formBtnLayout->addWidget(m_employeExportExcelBtn);
        connect(m_employeExportExcelBtn, &QPushButton::clicked, this, &MainWindow::onEmployeExportExcelClicked);
    }
    if (m_employeExportExcelBtn) {
        m_employeExportExcelBtn->hide();
        if (ui->formBtnLayout)
            ui->formBtnLayout->removeWidget(m_employeExportExcelBtn);
    }

    if (ui->btnSendChat)
        ui->btnSendChat->hide();
}

void MainWindow::onEmployeSelectionChanged()
{
    if (!ui->employeeTable)
        return;
    const int row = ui->employeeTable->currentRow();
    if (row < 0 || !ui->employeeTable->item(row, 0)) {
        setEmployeFormReadOnlyCin(false);
        if (ui->btnModifier)
            ui->btnModifier->setEnabled(false);
        if (ui->btnSupprimer)
            ui->btnSupprimer->setEnabled(false);
        return;
    }
    Employe::fillEditorFromTableRow(employeEditorBindings(), ui->employeeTable, row);
    setEmployeFormReadOnlyCin(true);
    if (ui->btnModifier)
        ui->btnModifier->setEnabled(true);
    if (ui->btnSupprimer)
        ui->btnSupprimer->setEnabled(true);
}

void MainWindow::onEmployeAjouterClicked()
{
    if (m_employeEditMode) {
        onEmployeModifierClicked();
        return;
    }
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Employes"), QStringLiteral("Base de donnees fermee."));
        return;
    }
    QString schemaErr;
    if (!Employe::ensureSchema(&schemaErr)) {
        QMessageBox::critical(this, QStringLiteral("Employes"),
                              QStringLiteral("Schema EMPLOYES :\n%1").arg(schemaErr));
        return;
    }
    const EmployeEditorWidgets w = employeEditorBindings();
    const QString validation = Employe::validateForm(w);
    if (!validation.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"), validation);
        return;
    }
    const QString cin = w.cin->text().trimmed();
    QString err;
    if (Employe::cinExists(cin, &err)) {
        QMessageBox::warning(this, QStringLiteral("CIN"), QStringLiteral("Un employe avec ce CIN existe deja."));
        return;
    }

    QString salaireNorm = w.salaire->text().trimmed();
    salaireNorm.replace(QLatin1Char(','), QLatin1Char('.'));
    const double salaireVal = salaireNorm.toDouble();
    const QDate dEmb = w.dateEmbauche ? w.dateEmbauche->date() : QDate::currentDate();

    Employe e(cin,
              w.nom->text().trimmed(),
              w.prenom->text().trimmed(),
              w.sexe->currentText(),
              salaireVal,
              dEmb,
              w.telephone->text().trimmed(),
              w.poste->text().trimmed(),
              w.adresse->text().trimmed(),
              w.email->text().trimmed());

    const QString confirmMsg = QStringLiteral("Confirmer l'ajout de l'employe CIN %1 ?").arg(cin);
    if (QMessageBox::question(this, QStringLiteral("Employes"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    if (!e.ajouter(&err)) {
        QMessageBox::warning(this, QStringLiteral("Employes"), QStringLiteral("Erreur a l'ajout :\n%1").arg(err));
        return;
    }
    ui->employeeTable->clearSelection();
    clearEmployeForm();
    refreshEmployesTable();
}

void MainWindow::onEmployeModifierClicked()
{
    if (!ui->lineEditCIN || ui->lineEditCIN->text().trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Employes"),
                                 QStringLiteral("Selectionnez un employe dans le tableau."));
        return;
    }
    QString schemaErr;
    if (!Employe::ensureSchema(&schemaErr)) {
        QMessageBox::critical(this, QStringLiteral("Employes"),
                              QStringLiteral("Schema EMPLOYES :\n%1").arg(schemaErr));
        return;
    }
    const EmployeEditorWidgets w = employeEditorBindings();
    const QString validation = Employe::validateForm(w);
    if (!validation.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"), validation);
        return;
    }

    QString salaireNorm = w.salaire->text().trimmed();
    salaireNorm.replace(QLatin1Char(','), QLatin1Char('.'));
    const double salaireVal = salaireNorm.toDouble();
    const QDate dEmb = w.dateEmbauche ? w.dateEmbauche->date() : QDate::currentDate();

    Employe e(w.cin->text().trimmed(),
              w.nom->text().trimmed(),
              w.prenom->text().trimmed(),
              w.sexe->currentText(),
              salaireVal,
              dEmb,
              w.telephone->text().trimmed(),
              w.poste->text().trimmed(),
              w.adresse->text().trimmed(),
              w.email->text().trimmed());

    const QString confirmMsg = QStringLiteral("Confirmer la modification de l'employe CIN %1 ?").arg(w.cin->text().trimmed());
    if (QMessageBox::question(this, QStringLiteral("Employes"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }

    QString err;
    if (!e.modifier(&err)) {
        QMessageBox::warning(this, QStringLiteral("Employes"), QStringLiteral("Erreur :\n%1").arg(err));
        return;
    }
    refreshEmployesTable();
    m_employeEditMode = false;
    m_employeEditingCin.clear();
    if (ui->btnAjouter)
        ui->btnAjouter->setText(QStringLiteral("Ajouter"));
    if (m_employeCancelEditButton)
        m_employeCancelEditButton->hide();
    if (ui->employeeTable) {
        ui->employeeTable->setSelectionMode(QAbstractItemView::SingleSelection);
        for (int r = 0; r < ui->employeeTable->rowCount(); ++r) {
            if (QWidget *w = ui->employeeTable->cellWidget(r, 10)) w->setEnabled(true);
            if (QWidget *w = ui->employeeTable->cellWidget(r, 11)) w->setEnabled(true);
        }
    }
    QMessageBox::information(this, QStringLiteral("Employes"), QStringLiteral("Modification enregistree."));
}

void MainWindow::onEmployeSupprimerClicked()
{
    if (!ui->lineEditCIN || ui->lineEditCIN->text().trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Employes"),
                                 QStringLiteral("Selectionnez un employe avant suppression."));
        return;
    }
    const QString cin = ui->lineEditCIN->text().trimmed();
    const QString nom = ui->lineEditNom ? ui->lineEditNom->text().trimmed() : QString();
    const QString prenom = ui->lineEditPrenom ? ui->lineEditPrenom->text().trimmed() : QString();
    const QString question = QStringLiteral("Supprimer l'employe :\n\nCIN : %1\nNom : %2\nPrenom : %3 ?")
                                 .arg(cin, nom, prenom);
    if (QMessageBox::question(this, QStringLiteral("Confirmation"), question, QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    QString err;
    if (!Employe::supprimer(cin, &err)) {
        QMessageBox::warning(this, QStringLiteral("Employes"), QStringLiteral("Erreur :\n%1").arg(err));
        return;
    }
    if (ui->employeeTable)
        ui->employeeTable->clearSelection();
    clearEmployeForm();
    refreshEmployesTable();
    QMessageBox::information(this, QStringLiteral("Employes"), QStringLiteral("Suppression reussie."));
}

void MainWindow::onEmployeRechercherClicked()
{
    if (!ui->employeeTable)
        return;
    const QString q = ui->lineEditSearch ? ui->lineEditSearch->text() : QString();
    Employe::applySearchFilter(ui->employeeTable, q);
    if (ui->pageEmployes) {
        if (QComboBox *combo = ui->pageEmployes->findChild<QComboBox *>(QStringLiteral("employesTopFilterCombo")))
            applyTopColumnFilter(ui->employeeTable, q, combo->currentData().toInt());
    }
    updateEmployesStatsPanel();
}

void MainWindow::onEmployeSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    onEmployeRechercherClicked();
}

void MainWindow::onEmployeExportPdfClicked()
{
    if (!ui->employeeTable)
        return;
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Exporter PDF"), QString(),
                                                      QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty())
        return;
    QString err;
    if (!Employe::exportPdfFromTable(ui->employeeTable, path, &err)) {
        QMessageBox::warning(this, QStringLiteral("Export PDF"), err.isEmpty() ? QStringLiteral("Echec.") : err);
    }
}

void MainWindow::onEmployeExportExcelClicked()
{
    if (!ui->employeeTable)
        return;
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Exporter Excel (CSV)"), QString(),
                                                      QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty())
        return;
    QString err;
    if (!Employe::exportCsvFromTable(ui->employeeTable, path, &err)) {
        QMessageBox::warning(this, QStringLiteral("Export CSV"), err.isEmpty() ? QStringLiteral("Echec.") : err);
    }
}

void MainWindow::onEmployeSendChatClicked()
{
    if (!ui->lineEditChat || !ui->chatDisplay)
        return;
    const QString message = ui->lineEditChat->text().trimmed();
    if (message.isEmpty())
        return;

    ui->chatDisplay->append(QStringLiteral("<b>Vous:</b> ") + message);
    ui->lineEditChat->clear();

    QString reponse;
    if (message.contains(QStringLiteral("bonjour"), Qt::CaseInsensitive)) {
        reponse = QStringLiteral("Bonjour ! Assistant employes LEATHER HOUSE. Comment puis-je vous aider ?");
    } else if (message.contains(QStringLiteral("stat"), Qt::CaseInsensitive)
               || message.contains(QStringLiteral("employ"), Qt::CaseInsensitive)) {
        reponse = QStringLiteral(
            "Les statistiques (total, salaire moyen, nouveaux ce mois) sont dans le panneau de droite.");
    } else if (message.contains(QStringLiteral("cin"), Qt::CaseInsensitive)) {
        reponse = QStringLiteral("Le CIN tunisien attendu est 8 chiffres (meme format que telephone).");
    } else {
        reponse = QStringLiteral(
            "Pour ajouter un employe, remplissez la fiche puis cliquez sur Ajouter apres validation.");
    }
    ui->chatDisplay->append(QStringLiteral("<i><b>IA:</b> ") + reponse + QStringLiteral("</i><br>"));
}

void MainWindow::setupMatierePage()
{
    if (!ui->employeeTable_5 || !ui->formOuterLayout_5 || !ui->employeeFormBox_5)
        return;
    if (m_matiereUiWired)
        return;
    m_matiereUiWired = true;

    if (!ui->employeeFormBox_5->property("matiereFicheModernized").toBool()) {
        ui->employeeFormBox_5->setTitle(QString());
        applyFormStyle(ui->employeeFormBox_5);

        std::function<void(QLayout *)> hideLayoutWidgets = [&](QLayout *lay) {
            if (!lay)
                return;
            for (int i = 0; i < lay->count(); ++i) {
                QLayoutItem *it = lay->itemAt(i);
                if (!it)
                    continue;
                if (QWidget *w = it->widget())
                    w->hide();
                if (QLayout *child = it->layout())
                    hideLayoutWidgets(child);
            }
        };
        hideLayoutWidgets(ui->formRow1_5);
        hideLayoutWidgets(ui->formRow2_5);
        hideLayoutWidgets(ui->formRowStatut_5);

        const QList<QWidget *> legacyLabels = {
            ui->labelCIN_5, ui->labelNom_5, ui->labelPrenom_5, ui->labelSexe_5, ui->labelSalaire_5,
            ui->labelDateEmbauche_5, ui->labelTelephone_4, ui->labelPoste_4, ui->labelAdresse_4, ui->labelEmail_2, ui->labelStatut_5
        };
        for (QWidget *w : legacyLabels) {
            if (w)
                w->hide();
        }

        auto *titleLab = new QLabel(QStringLiteral("Fiche cuir"), ui->employeeFormBox_5);
        titleLab->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 700; color: #000000; padding: 2px 4px;"));
        ui->formOuterLayout_5->insertWidget(0, titleLab);

        auto *formWrap = new QWidget(ui->employeeFormBox_5);
        formWrap->setStyleSheet(QStringLiteral("background:#ffffff;"));
        auto *formLay = new QFormLayout(formWrap);
        formLay->setContentsMargins(4, 2, 4, 4);
        formLay->setHorizontalSpacing(12);
        formLay->setVerticalSpacing(12);
        formLay->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        formLay->setFormAlignment(Qt::AlignTop);
        formLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        const QList<QWidget *> fieldsToShow = {
            static_cast<QWidget *>(ui->lineEditCIN_5),
            static_cast<QWidget *>(ui->lineEditNom_5),
            static_cast<QWidget *>(ui->lineEditPrenom_6),
            static_cast<QWidget *>(ui->lineEditSalaire_2),
            static_cast<QWidget *>(ui->comboBoxSexe_2),
            static_cast<QWidget *>(ui->lineEditTelephone_2),
            static_cast<QWidget *>(ui->lineEditPoste_2),
            static_cast<QWidget *>(ui->lineEditAdresse_4),
            static_cast<QWidget *>(ui->lineEditEmail_4),
            static_cast<QWidget *>(ui->dateEditEmbauche_2),
            static_cast<QWidget *>(ui->lineEditStatut_5),
        };
        for (QWidget *w : fieldsToShow) {
            if (!w)
                continue;
            w->show();
            w->setMinimumHeight(34);
            w->setMaximumWidth(220);
            w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }

        formLay->addRow(new QLabel(QStringLiteral("ID"), formWrap), ui->lineEditCIN_5);
        formLay->addRow(new QLabel(QStringLiteral("Reference"), formWrap), ui->lineEditNom_5);
        formLay->addRow(new QLabel(QStringLiteral("Nom cuir"), formWrap), ui->lineEditPrenom_6);
        formLay->addRow(new QLabel(QStringLiteral("Type cuir"), formWrap), ui->lineEditSalaire_2);
        formLay->addRow(new QLabel(QStringLiteral("Gamme"), formWrap), ui->comboBoxSexe_2);
        formLay->addRow(new QLabel(QStringLiteral("Couleur"), formWrap), ui->lineEditTelephone_2);
        formLay->addRow(new QLabel(QStringLiteral("Qt stock"), formWrap), ui->lineEditPoste_2);
        formLay->addRow(new QLabel(QStringLiteral("Epaisseur"), formWrap), ui->lineEditAdresse_4);
        formLay->addRow(new QLabel(QStringLiteral("Origine"), formWrap), ui->lineEditEmail_4);
        formLay->addRow(new QLabel(QStringLiteral("Date achat"), formWrap), ui->dateEditEmbauche_2);
        formLay->addRow(new QLabel(QStringLiteral("Statut"), formWrap), ui->lineEditStatut_5);
        ui->formOuterLayout_5->insertWidget(1, formWrap);

        if (ui->formBtnLayout_5) {
            ui->formBtnLayout_5->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->formBtnLayout_5->setSpacing(10);
            for (int i = ui->formBtnLayout_5->count() - 1; i >= 0; --i) {
                QLayoutItem *it = ui->formBtnLayout_5->itemAt(i);
                if (it && it->spacerItem()) {
                    QLayoutItem *removed = ui->formBtnLayout_5->takeAt(i);
                    delete removed;
                }
            }
        }
        if (ui->btnAjouter_7) {
            applyUnifiedAddButtonStyle(ui->btnAjouter_7);
        }
        if (!m_matiereCancelEditButton && ui->formBtnLayout_5 && ui->page_3) {
            m_matiereCancelEditButton = new QPushButton(QStringLiteral("Annuler"), ui->page_3);
            m_matiereCancelEditButton->setCursor(Qt::PointingHandCursor);
            m_matiereCancelEditButton->setMinimumSize(110, 32);
            if (ui->btnAjouter_7)
                m_matiereCancelEditButton->setStyleSheet(ui->btnAjouter_7->styleSheet());
            m_matiereCancelEditButton->hide();
            ui->formBtnLayout_5->addWidget(m_matiereCancelEditButton);
            connect(m_matiereCancelEditButton, &QPushButton::clicked, this, [this]() {
                m_matiereEditMode = false;
                m_matiereEditingId = -1;
                if (ui->btnAjouter_7)
                    ui->btnAjouter_7->setText(QStringLiteral("Ajouter"));
                if (m_matiereCancelEditButton)
                    m_matiereCancelEditButton->hide();
                if (ui->employeeTable_5) {
                    ui->employeeTable_5->setSelectionMode(QAbstractItemView::SingleSelection);
                    for (int r = 0; r < ui->employeeTable_5->rowCount(); ++r) {
                        if (QWidget *w = ui->employeeTable_5->cellWidget(r, 13)) w->setEnabled(true);
                        if (QWidget *w = ui->employeeTable_5->cellWidget(r, 14)) w->setEnabled(true);
                    }
                }
            });
        }
        if (ui->btnModifier_5) { ui->btnModifier_5->hide(); if (ui->formBtnLayout_5) ui->formBtnLayout_5->removeWidget(ui->btnModifier_5); }
        if (ui->btnSupprimer_5) { ui->btnSupprimer_5->hide(); if (ui->formBtnLayout_5) ui->formBtnLayout_5->removeWidget(ui->btnSupprimer_5); }
        if (ui->btnExportPDF_2) ui->btnExportPDF_2->hide();
        if (ui->btnExportExcel) ui->btnExportExcel->hide();
        if (ui->btnFiltreDisponible) ui->btnFiltreDisponible->hide();
        if (ui->btnFiltreSeuilCritique) ui->btnFiltreSeuilCritique->hide();
        if (ui->btnAfficherTous) ui->btnAfficherTous->hide();
        ui->employeeFormBox_5->setProperty("matiereFicheModernized", true);
    }

    if (ui->btnAjouter_7)
        ui->btnAjouter_7->show();

    ui->employeeTable_5->setColumnCount(15);
    ui->employeeTable_5->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Reference"),
        QStringLiteral("Nom du cuir"),
        QStringLiteral("Type de cuir"),
        QStringLiteral("Gamme"),
        QStringLiteral("Couleur"),
        QStringLiteral("Epaisseur"),
        QStringLiteral("Origine"),
        QStringLiteral("Fournisseur associe"),
        QStringLiteral("Quantite de stock"),
        QStringLiteral("Prix"),
        QStringLiteral("Date d'achat"),
        QStringLiteral("Status"),
        QStringLiteral("SUPPR."),
        QStringLiteral("MODIF."),
    });
    ui->employeeTable_5->horizontalHeader()->setStretchLastSection(true);
    ui->employeeTable_5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->employeeTable_5->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->employeeTable_5->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->employeeTable_5->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (ui->comboBoxSexe_2) {
        ui->comboBoxSexe_2->setEditable(true);
        ui->comboBoxSexe_2->setInsertPolicy(QComboBox::NoInsert);
    }

    if (ui->lineEditCIN_5) {
        ui->lineEditCIN_5->clear();
        ui->lineEditCIN_5->setPlaceholderText(QStringLiteral("Vide = ID auto, sinon entier > 0"));
        ui->lineEditCIN_5->setReadOnly(false);
        ui->lineEditCIN_5->setEnabled(true);
        ui->lineEditCIN_5->setFocusPolicy(Qt::StrongFocus);
    }
    if (ui->lineEditNom_5) {
        ui->lineEditNom_5->clear();
        ui->lineEditNom_5->setPlaceholderText(QStringLiteral("Reference interne"));
    }
    if (ui->lineEditPrenom_6) {
        ui->lineEditPrenom_6->clear();
        ui->lineEditPrenom_6->setPlaceholderText(QStringLiteral("Nom du cuir"));
    }
    if (ui->lineEditSalaire_2) {
        ui->lineEditSalaire_2->clear();
        ui->lineEditSalaire_2->setPlaceholderText(QStringLiteral("Type de cuir"));
    }
    if (ui->lineEditPoste_2) {
        ui->lineEditPoste_2->clear();
        ui->lineEditPoste_2->setPlaceholderText(QStringLiteral("Quantite en stock (entier)"));
        ui->lineEditPoste_2->setValidator(new QIntValidator(0, 999999999, this));
    }
    if (ui->lineEditAdresse_4) {
        ui->lineEditAdresse_4->clear();
        ui->lineEditAdresse_4->setPlaceholderText(QStringLiteral("Epaisseur (ex. 1,2 ou 1.2)"));
        static const QRegularExpression kEpaisseurSaisie(QStringLiteral(R"(^\d*([.,]\d{0,3})?$)"));
        ui->lineEditAdresse_4->setValidator(new QRegularExpressionValidator(kEpaisseurSaisie, this));
    }
    if (ui->lineEditEmail_4) {
        ui->lineEditEmail_4->clear();
        ui->lineEditEmail_4->setPlaceholderText(QStringLiteral("Origine"));
    }
    if (ui->lineEditTelephone_2) {
        ui->lineEditTelephone_2->clear();
        ui->lineEditTelephone_2->setPlaceholderText(QStringLiteral("Couleur"));
    }
    if (ui->lineEditStatut_5) {
        ui->lineEditStatut_5->clear();
        ui->lineEditStatut_5->setPlaceholderText(QStringLiteral("Statut (ex. Disponible)"));
    }
    if (ui->lineEditSearch_6) {
        ui->lineEditSearch_6->clear();
        ui->lineEditSearch_6->setPlaceholderText(QStringLiteral("Rechercher..."));
        ui->lineEditSearch_6->setMinimumHeight(34);
        ui->lineEditSearch_6->setMinimumWidth(260);
        ui->lineEditSearch_6->setMaximumWidth(360);
        ui->lineEditSearch_6->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->lineEditSearch_6->setStyleSheet(QStringLiteral(
            "QLineEdit { border: 1px solid #d5dbe5; border-radius: 8px; padding: 0 10px; background: #ffffff; }"
            "QLineEdit:focus { border-color: #c08a5b; }"));
    }
    if (ui->btnRechercher_5) {
        ui->btnRechercher_5->setMinimumSize(116, 34);
        ui->btnRechercher_5->setMaximumWidth(116);
        ui->btnRechercher_5->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  border: 1px solid transparent;"
            "  border-radius: 8px;"
            "  background: #f3f5f8;"
            "  color: #1f2937;"
            "  font-weight: 600;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover { background: #fff4e9; border-color: #d8b18f; color: #7a4d2b; }"
            "QPushButton:pressed { background: #ecd7c3; border-color: #d8b18f; color: #7a4d2b; }"));
    }
    if (ui->matieresHeaderRow && !ui->matieresHeaderRow->property("searchRightAligned").toBool()) {
        const int searchIdx = ui->matieresHeaderRow->indexOf(ui->lineEditSearch_6);
        if (searchIdx > 0)
            ui->matieresHeaderRow->insertStretch(searchIdx, 1);
        ui->matieresHeaderRow->setProperty("searchRightAligned", true);
    }

    connect(ui->lineEditSearch_6, &QLineEdit::returnPressed, this, &MainWindow::on_btnRechercher_5_clicked);
}

MatierePremiereEditorWidgets MainWindow::matiereEditorBindings() const
{
    MatierePremiereEditorWidgets w;
    w.idMp = ui->lineEditCIN_5;
    w.reference = ui->lineEditNom_5;
    w.nomCuir = ui->lineEditPrenom_6;
    w.typeCuir = ui->lineEditSalaire_2;
    w.gamme = ui->comboBoxSexe_2;
    w.couleur = ui->lineEditTelephone_2;
    w.statut = ui->lineEditStatut_5;
    w.epaisseur = ui->lineEditAdresse_4;
    w.origine = ui->lineEditEmail_4;
    w.reserve = ui->lineEditPoste_2;
    w.fournisseurAffiche = nullptr;
    w.prixAffiche = nullptr;
    w.dateAchatAffiche = ui->dateEditEmbauche_2;
    return w;
}

bool MainWindow::readMatiereFromForm(MatierePremiere &out) const
{
    const MatierePremiereEditorWidgets w = matiereEditorBindings();
    if (!w.reference || !w.nomCuir || !w.typeCuir || !w.gamme || !w.couleur || !w.statut || !w.epaisseur || !w.origine
        || !w.reserve) {
        return false;
    }
    if (w.reference->text().trimmed().isEmpty() || w.nomCuir->text().trimmed().isEmpty())
        return false;

    int id = 0;
    if (w.idMp && !w.idMp->text().trimmed().isEmpty()) {
        bool ok = false;
        id = w.idMp->text().trimmed().toInt(&ok);
        if (!ok || id <= 0)
            return false;
    }

    bool okE = false;
    QString epRaw = w.epaisseur->text().trimmed();
    epRaw.remove(QLatin1Char(' '));
    epRaw.replace(QLatin1Char(','), QLatin1Char('.'));
    const double ep = epRaw.toDouble(&okE);
    if (!okE || ep < 0.0)
        return false;
    bool okR = false;
    const int res = w.reserve->text().trimmed().toInt(&okR);
    if (!okR || res < 0)
        return false;

    const QString gammeTxt = w.gamme->currentText().trimmed();
    out = MatierePremiere(id,
                          w.reference->text().trimmed(),
                          w.nomCuir->text().trimmed(),
                          w.typeCuir->text().trimmed(),
                          gammeTxt,
                          w.couleur->text().trimmed(),
                          w.statut->text().trimmed(),
                          ep,
                          w.origine->text().trimmed(),
                          res);
    return true;
}

void MainWindow::updateMatieresStatsPanel()
{
    QString err;
    const QString s = MatierePremiere::statsSummaryPlain(&err);
    QStringList lines = s.split(QLatin1Char('\n'));
    lines.removeAll(QString());
    if (ui->statTitle1_2 && !lines.isEmpty())
        ui->statTitle1_2->setText(lines.value(0));
    if (ui->statTitle2_2 && lines.size() > 1)
        ui->statTitle2_2->setText(lines.value(1));
    if (ui->statTitle3_2 && lines.size() > 2)
        ui->statTitle3_2->setText(lines.value(2));
    Q_UNUSED(err);
}

void MainWindow::refreshMatieresTable()
{
    if (!ui->employeeTable_5)
        return;
    QString err;
    if (!MatierePremiere::populateTable(ui->employeeTable_5, &err)) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"),
                             QStringLiteral("Chargement impossible.\n%1").arg(err));
        return;
    }
    if (ui->employeeTable_5->columnCount() < 15)
        ui->employeeTable_5->setColumnCount(15);
    ui->employeeTable_5->setHorizontalHeaderItem(13, new QTableWidgetItem(QStringLiteral("SUPPR.")));
    ui->employeeTable_5->setHorizontalHeaderItem(14, new QTableWidgetItem(QStringLiteral("MODIF.")));
    for (int row = 0; row < ui->employeeTable_5->rowCount(); ++row) {
        auto *deleteBtn = new QPushButton(QStringLiteral("♟"), ui->employeeTable_5);
        deleteBtn->setToolTip(QStringLiteral("Supprimer cette matiere premiere"));
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, row]() {
            ui->employeeTable_5->setCurrentCell(row, 0);
            on_employeeTable_5_cellClicked(row, 0);
            on_btnSupprimer_5_clicked();
        });
        ui->employeeTable_5->setCellWidget(row, 13, deleteBtn);

        auto *updateBtn = new QPushButton(QStringLiteral("✎"), ui->employeeTable_5);
        updateBtn->setToolTip(QStringLiteral("Modifier cette matiere premiere"));
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setMinimumSize(38, 28);
        applyUnifiedButtonStyle(updateBtn);
        connect(updateBtn, &QPushButton::clicked, this, [this, row]() {
            ui->employeeTable_5->setCurrentCell(row, 0);
            on_employeeTable_5_cellClicked(row, 0);
            m_matiereEditMode = true;
            m_matiereEditingId = m_matiereSelectedId;
            if (ui->btnAjouter_7)
                ui->btnAjouter_7->setText(QStringLiteral("Enregistrer"));
            if (ui->btnAjouter_7 && m_matiereCancelEditButton)
                ui->btnAjouter_7->setMinimumSize(m_matiereCancelEditButton->minimumSize());
            if (m_matiereCancelEditButton)
                m_matiereCancelEditButton->show();
            if (ui->employeeTable_5) {
                ui->employeeTable_5->setSelectionMode(QAbstractItemView::NoSelection);
                for (int r = 0; r < ui->employeeTable_5->rowCount(); ++r) {
                    if (QWidget *w = ui->employeeTable_5->cellWidget(r, 13)) w->setEnabled(false);
                    if (QWidget *w = ui->employeeTable_5->cellWidget(r, 14)) w->setEnabled(false);
                }
            }
        });
        ui->employeeTable_5->setCellWidget(row, 14, updateBtn);
    }

    applyMatieresViewFilters();
    applyMatieresTableSortIfNeeded();
    updateMatieresStatsPanel();

    const int keep = m_matiereSelectedId;
    m_matiereSelectedId = -1;
    if (keep > 0) {
        for (int r = 0; r < ui->employeeTable_5->rowCount(); ++r) {
            if (ui->employeeTable_5->isRowHidden(r))
                continue;
            if (ui->employeeTable_5->item(r, 0)
                && ui->employeeTable_5->item(r, 0)->text().toInt() == keep) {
                ui->employeeTable_5->selectRow(r);
                fillMatiereFormFromTableRow(r);
                break;
            }
        }
    }
    onStatisticsDataChanged();
    loadStats();
    loadStockFaible();
    loadPrediction();
}

void MainWindow::clearMatiereForm()
{
    m_matiereSelectedId = -1;
    MatierePremiere::clearEditorFields(matiereEditorBindings());
}

void MainWindow::fillMatiereFormFromTableRow(int row)
{
    if (!ui->employeeTable_5)
        return;
    const int id = MatierePremiere::fillEditorFromTableRow(matiereEditorBindings(), ui->employeeTable_5, row);
    m_matiereSelectedId = id > 0 ? id : -1;
}

void MainWindow::on_employeeTable_5_cellClicked(int row, int)
{
    if (m_matiereEditMode)
        return;
    if (ui->employeeTable_5 && ui->employeeTable_5->isRowHidden(row))
        return;
    fillMatiereFormFromTableRow(row);
}

void MainWindow::on_btnRechercher_5_clicked()
{
    applyMatieresViewFilters();
    applyMatieresTableSortIfNeeded();
    if (ui->page_3) {
        if (QComboBox *combo = ui->page_3->findChild<QComboBox *>(QStringLiteral("matieresTopFilterCombo"))) {
            const QString q = ui->lineEditSearch_6 ? ui->lineEditSearch_6->text() : QString();
            applyTopColumnFilter(ui->employeeTable_5, q, combo->currentData().toInt());
        }
    }
}

void MainWindow::on_btnAjouter_7_clicked()
{
    if (m_matiereEditMode) {
        on_btnModifier_5_clicked();
        return;
    }
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"), QStringLiteral("Base fermee."));
        return;
    }
    QString schemaErr;
    if (!MatierePremiere::ensureSchema(&schemaErr)) {
        QMessageBox::critical(this, QStringLiteral("Matieres premieres"), schemaErr);
        return;
    }

    QStringList errors;
    QWidget *firstInvalid = nullptr;
    MatierePremiere m;
    if (!readMatiereFromForm(m)) {
        errors << QStringLiteral("- Verifiez : reference, nom du cuir, type, gamme, couleur, statut, origine.");
        errors << QStringLiteral("- Epaisseur : nombre valide (ex. 1,2 ou 1.2).");
        errors << QStringLiteral("- Quantite : entier >= 0.");
        if (!firstInvalid) firstInvalid = ui->lineEditNom_5;
    }

    int nid = m.getId();
    if (ui->lineEditCIN_5 && !ui->lineEditCIN_5->text().trimmed().isEmpty()) {
        bool idOk = false;
        nid = ui->lineEditCIN_5->text().trimmed().toInt(&idOk);
        if (!idOk || nid <= 0) {
            errors << QStringLiteral("- ID entier > 0.");
            if (!firstInvalid) firstInvalid = ui->lineEditCIN_5;
        }
        if (MatierePremiere::idExiste(nid))
            errors << QStringLiteral("- ID %1 deja utilise.").arg(nid);
    } else {
        nid = MatierePremiere::nextAvailableId();
    }
    if (nid <= 0 && errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"),
                             MatierePremiere::lastSqlError.isEmpty() ? QStringLiteral("ID indisponible.")
                                                                     : MatierePremiere::lastSqlError);
        return;
    }
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"),
                             QStringLiteral("Veuillez corriger les champs suivants :\n%1").arg(errors.join(QLatin1Char('\n'))));
        if (firstInvalid)
            firstInvalid->setFocus();
        return;
    }

    MatierePremiere toInsert(nid,
                             m.getReference(),
                             m.getNomCuir(),
                             m.getTypeCuir(),
                             m.getGamme(),
                             m.getCouleur(),
                             m.getStatut(),
                             m.getEpaisseur(),
                             m.getOrigine(),
                             m.getReserve());
    const QString confirmMsg = QStringLiteral("Confirmer l'ajout de la matiere premiere ID %1 ?").arg(nid);
    if (QMessageBox::question(this, QStringLiteral("Matieres premieres"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    if (!toInsert.ajouter()) {
        QMessageBox::critical(this, QStringLiteral("Matieres premieres"),
                              QStringLiteral("Echec ajout:\n%1").arg(MatierePremiere::lastSqlError));
        return;
    }
    refreshMatieresTable();
    clearMatiereForm();
    QMessageBox::information(this, QStringLiteral("Matieres premieres"), QStringLiteral("Ajoute (ID %1).").arg(nid));
}

void MainWindow::on_btnModifier_5_clicked()
{
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"), QStringLiteral("Base fermee."));
        return;
    }
    if (m_matiereSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"), QStringLiteral("Selectionnez une ligne."));
        return;
    }
    QStringList errors;
    QWidget *firstInvalid = nullptr;
    MatierePremiere m;
    if (!readMatiereFromForm(m)) {
        errors << QStringLiteral("- Saisie incomplete ou invalide.");
        if (!firstInvalid) firstInvalid = ui->lineEditNom_5;
    }
    bool newIdOk = false;
    const int newId = ui->lineEditCIN_5 ? ui->lineEditCIN_5->text().trimmed().toInt(&newIdOk) : m_matiereSelectedId;
    if (!newIdOk || newId <= 0) {
        errors << QStringLiteral("- ID entier > 0 requis.");
        if (!firstInvalid) firstInvalid = ui->lineEditCIN_5;
    }
    if (newId != m_matiereSelectedId && MatierePremiere::idExiste(newId))
        errors << QStringLiteral("- ID %1 deja pris.").arg(newId);
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Controle de saisie"),
                             QStringLiteral("Veuillez corriger les champs suivants :\n%1").arg(errors.join(QLatin1Char('\n'))));
        if (firstInvalid)
            firstInvalid->setFocus();
        return;
    }

    MatierePremiere upd(newId,
                        m.getReference(),
                        m.getNomCuir(),
                        m.getTypeCuir(),
                        m.getGamme(),
                        m.getCouleur(),
                        m.getStatut(),
                        m.getEpaisseur(),
                        m.getOrigine(),
                        m.getReserve());
    const QString confirmMsg = QStringLiteral("Confirmer la modification de la matiere premiere ID %1 ?").arg(m_matiereSelectedId);
    if (QMessageBox::question(this, QStringLiteral("Matieres premieres"), confirmMsg,
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    if (!upd.modifier(m_matiereSelectedId, newId)) {
        QMessageBox::critical(this, QStringLiteral("Matieres premieres"),
                              QStringLiteral("Echec modification:\n%1").arg(MatierePremiere::lastSqlError));
        return;
    }
    m_matiereSelectedId = newId;
    m_matiereEditMode = false;
    m_matiereEditingId = -1;
    if (ui->btnAjouter_7)
        ui->btnAjouter_7->setText(QStringLiteral("Ajouter"));
    if (m_matiereCancelEditButton)
        m_matiereCancelEditButton->hide();
    if (ui->employeeTable_5) {
        ui->employeeTable_5->setSelectionMode(QAbstractItemView::SingleSelection);
        for (int r = 0; r < ui->employeeTable_5->rowCount(); ++r) {
            if (QWidget *w = ui->employeeTable_5->cellWidget(r, 13)) w->setEnabled(true);
            if (QWidget *w = ui->employeeTable_5->cellWidget(r, 14)) w->setEnabled(true);
        }
    }
    if (ui->lineEditCIN_5)
        ui->lineEditCIN_5->setText(QString::number(newId));
    refreshMatieresTable();
    QMessageBox::information(this, QStringLiteral("Matieres premieres"), QStringLiteral("Modifie."));
}

void MainWindow::on_btnSupprimer_5_clicked()
{
    if (!ensureDbOpenForProduits()) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"), QStringLiteral("Base fermee."));
        return;
    }
    if (m_matiereSelectedId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Matieres premieres"), QStringLiteral("Selectionnez une ligne."));
        return;
    }
    if (QMessageBox::question(this,
                              QStringLiteral("Matieres premieres"),
                              QStringLiteral("Supprimer la matiere premiere ID %1 ?").arg(m_matiereSelectedId),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    if (!MatierePremiere::supprimer(m_matiereSelectedId)) {
        QMessageBox::critical(this, QStringLiteral("Matieres premieres"),
                              QStringLiteral("Echec suppression:\n%1").arg(MatierePremiere::lastSqlError));
        return;
    }
    refreshMatieresTable();
    clearMatiereForm();
    QMessageBox::information(this, QStringLiteral("Matieres premieres"), QStringLiteral("Supprime."));
}

void MainWindow::on_btnExportPDF_2_clicked()
{
    if (!ui->employeeTable_5)
        return;
    const QString path = QFileDialog::getSaveFileName(this,
                                                      QStringLiteral("Exporter PDF"),
                                                      QStringLiteral("stock_matieres.pdf"),
                                                      QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty())
        return;
    QString err;
    if (!MatierePremiere::exportPdfFromTable(ui->employeeTable_5, path, &err)) {
        QMessageBox::warning(this, QStringLiteral("Export"), err.isEmpty() ? QStringLiteral("Echec export.") : err);
        return;
    }
    QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Fichier enregistre :\n%1").arg(path));
}

void MainWindow::on_btnExportExcel_clicked()
{
    if (!ui->employeeTable_5)
        return;
    const QString path = QFileDialog::getSaveFileName(this,
                                                      QStringLiteral("Exporter Excel (CSV)"),
                                                      QStringLiteral("stock_matieres.csv"),
                                                      QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty())
        return;
    QString err;
    if (!MatierePremiere::exportCsvFromTable(ui->employeeTable_5, path, &err)) {
        QMessageBox::warning(this, QStringLiteral("Export"), err.isEmpty() ? QStringLiteral("Echec export.") : err);
        return;
    }
    QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Fichier enregistre :\n%1").arg(path));
}

void MainWindow::on_btnFiltreDisponible_clicked()
{
    m_mpFilterDisponible = true;
    m_mpSeuilCritique = -1;
    applyMatieresViewFilters();
    applyMatieresTableSortIfNeeded();
}

void MainWindow::on_btnFiltreSeuilCritique_clicked()
{
    bool ok = false;
    const int seuil = QInputDialog::getInt(this,
                                           QStringLiteral("Seuil critique"),
                                           QStringLiteral("Afficher les matieres avec reserve <= "),
                                           10,
                                           0,
                                           1000000,
                                           1,
                                           &ok);
    if (!ok)
        return;
    m_mpFilterDisponible = false;
    m_mpSeuilCritique = seuil;
    applyMatieresViewFilters();
    applyMatieresTableSortIfNeeded();
}

void MainWindow::on_btnAfficherTous_clicked()
{
    m_mpFilterDisponible = false;
    m_mpSeuilCritique = -1;
    if (ui->lineEditSearch_6)
        ui->lineEditSearch_6->clear();
    if (ui->comboBoxTri_mp) {
        ui->comboBoxTri_mp->blockSignals(true);
        ui->comboBoxTri_mp->setCurrentIndex(0);
        ui->comboBoxTri_mp->blockSignals(false);
    }
    refreshMatieresTable();
}

void MainWindow::onOpenBusySeasonCalendar()
{
    BusySeasonCalendarDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_comboBoxTri_mp_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!ui->comboBoxTri_mp)
        return;
    if (ui->comboBoxTri_mp->currentIndex() <= 0) {
        refreshMatieresTable();
        return;
    }
    applyMatieresTableSortIfNeeded();
}

void MainWindow::on_comboBoxOrdre_mp_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!ui->comboBoxTri_mp || ui->comboBoxTri_mp->currentIndex() <= 0)
        return;
    applyMatieresTableSortIfNeeded();
}

void MainWindow::connectToGoogle()
{
    if (m_googleCalendarService)
        m_googleCalendarService->connectToGoogle();
}

void MainWindow::syncCalendar()
{
    if (m_googleCalendarService)
        m_googleCalendarService->syncCalendar();
}

void MainWindow::onGoogleEventCreated(bool success, const QString &detail)
{
    if (success) {
        QMessageBox::information(this, QStringLiteral("Google Calendar"),
                                 QStringLiteral("Événement créé.\nID : %1").arg(detail));
        if (m_googleCalendarService)
            m_googleCalendarService->syncCalendar();
    } else {
        QMessageBox::warning(this, QStringLiteral("Google Calendar"),
                             QStringLiteral("Échec création événement.\n%1").arg(detail));
    }
}

void MainWindow::onGoogleCalendarEventsReady(const QVector<GoogleCalendarEventRow> &rows)
{
    m_googleCalendarLastRows = rows;
    if (m_googleCalendarEventsList) {
        m_googleCalendarEventsList->clear();
        for (const GoogleCalendarEventRow &row : rows) {
            auto *item = new QListWidgetItem(row.displayText);
            /// Bonus : rouge = période chargée, vert = période libre (transparency « transparent » côté Google).
            item->setForeground(row.isBusy ? QColor(196, 30, 58) : QColor(22, 101, 52));
            m_googleCalendarEventsList->addItem(item);
        }
    }
    if (m_googleCalendarWidget) {
        m_googleCalendarWidget->setSelectedDate(QDate::currentDate());
        applyGoogleCalendarMonthHighlights();
    }
}

void MainWindow::applyGoogleCalendarMonthHighlights()
{
    if (!m_googleCalendarWidget)
        return;
    const int y = m_googleCalendarWidget->yearShown();
    const int m = m_googleCalendarWidget->monthShown();
    if (m < 1 || m > 12)
        return;

    QDate first(y, m, 1);
    const int dim = first.daysInMonth();
    for (int day = 1; day <= dim; ++day) {
        const QDate d(y, m, day);
        m_googleCalendarWidget->setDateTextFormat(d, QTextCharFormat());
    }

    struct Counts {
        int busy = 0;
        int freeSlots = 0;
    };
    QHash<QDate, Counts> perDay;
    for (const GoogleCalendarEventRow &row : m_googleCalendarLastRows) {
        if (!row.startDate.isValid())
            continue;
        if (row.startDate.year() != y || row.startDate.month() != m)
            continue;
        if (row.isBusy)
            perDay[row.startDate].busy++;
        else
            perDay[row.startDate].freeSlots++;
    }

    for (auto it = perDay.constBegin(); it != perDay.constEnd(); ++it) {
        const int b = it.value().busy;
        const int f = it.value().freeSlots;
        QTextCharFormat fmt;
        if (b > 0 && f == 0) {
            fmt.setBackground(QColor(255, 235, 238));
            fmt.setForeground(QColor(196, 30, 58));
        } else if (f > 0 && b == 0) {
            fmt.setBackground(QColor(232, 245, 233));
            fmt.setForeground(QColor(22, 101, 52));
        } else {
            fmt.setBackground(QColor(255, 243, 224));
            fmt.setForeground(QColor(120, 53, 15));
        }
        fmt.setFontWeight(QFont::DemiBold);
        m_googleCalendarWidget->setDateTextFormat(it.key(), fmt);
    }
}
