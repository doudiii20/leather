#include "assistantwindow.h"

#include <QCalendarWidget>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateEdit>
#include <QDateTime>
#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSplitter>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {
constexpr int kBusyThreshold = 80;
constexpr int kWarningThreshold = 65;
constexpr char kGoogleCalendarEventsBaseUrl[] =
    "https://www.googleapis.com/calendar/v3/calendars/dorrajridi8%40gmail.com/events";
constexpr char kGoogleCalendarDefaultQuery[] =
    "timeMin=2026-01-01T00:00:00Z&timeMax=2026-12-31T23:59:59Z&singleEvents=true&orderBy=startTime";

QColor busyColor(bool darkMode)
{
    return darkMode ? QColor(255, 99, 99) : QColor(196, 30, 58);
}

QString loadGoogleAccessTokenFromStorage()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("tokens.json")),
        QDir(configDir).absoluteFilePath(QStringLiteral("tokens.json")),
        QDir(configDir).absoluteFilePath(QStringLiteral("google_calendar_tokens.json")),
    };

    for (const QString &path : candidates) {
        QFile f(path);
        if (!f.exists() || !f.open(QIODevice::ReadOnly))
            continue;
        const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
        f.close();
        const QString token = obj.value(QStringLiteral("access_token")).toString().trimmed();
        if (!token.isEmpty())
            return token;
    }
    return {};
}

QUrl normalizedGoogleCalendarUrl(const QString &rawInput)
{
    const QString trimmed = rawInput.trimmed();
    if (trimmed.isEmpty()) {
        QUrl url(QString::fromLatin1(kGoogleCalendarEventsBaseUrl));
        url.setQuery(QString::fromLatin1(kGoogleCalendarDefaultQuery));
        return url;
    }

    if (!trimmed.startsWith(QStringLiteral("http://"), Qt::CaseInsensitive)
        && !trimmed.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
        QUrl url(QString::fromLatin1(kGoogleCalendarEventsBaseUrl));
        const QString q = trimmed.startsWith(QLatin1Char('?')) ? trimmed.mid(1) : trimmed;
        url.setQuery(q.isEmpty() ? QString::fromLatin1(kGoogleCalendarDefaultQuery) : q);
        return url;
    }

    QUrl url = QUrl::fromUserInput(trimmed);
    if (!url.isValid())
        return {};

    const QString path = url.path();
    if (!path.contains(QStringLiteral("/calendar/v3/calendars/"), Qt::CaseInsensitive)
        || !path.contains(QStringLiteral("/events"), Qt::CaseInsensitive)) {
        QUrl fixed(QString::fromLatin1(kGoogleCalendarEventsBaseUrl));
        fixed.setQuery(url.query());
        return fixed;
    }
    return url;
}
}

AssistantWindow::AssistantWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    m_categories = {"All", "Bags", "Shoes", "Jackets"};

    buildUi();
    initializeData();
    simulateBusySeasons(QDate::currentDate());
    refreshCalendarAppearance();
    refreshInsights();
    updateRecommendations();
    applyTheme(false);
    loadCalendarFromApi();

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, &AssistantWindow::onDateChanged);
    connect(m_calendar, &QCalendarWidget::currentPageChanged, this, [this](int year, int month) {
        simulateBusySeasons(QDate(year, month, 1));
        refreshCalendarAppearance();
        refreshInsights();
        updateRecommendations();
    });
    connect(m_categoryFilter, &QComboBox::currentIndexChanged, this, [this](int) {
        onCategoryChanged();
    });
    connect(m_themeButton, &QToolButton::clicked, this, &AssistantWindow::onThemeToggled);
    connect(m_syncApiButton, &QPushButton::clicked, this, &AssistantWindow::loadCalendarFromApi);
    connect(m_apiUrlInput, &QLineEdit::returnPressed, this, &AssistantWindow::loadCalendarFromApi);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AssistantWindow::onApiReplyFinished);
}

void AssistantWindow::buildUi()
{
    resize(1180, 760);
    setWindowTitle("Smart Calendar - Matiere Premieres");

    QWidget *central = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    QFrame *headerFrame = new QFrame(central);
    headerFrame->setObjectName("headerFrame");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(16, 10, 16, 10);

    QLabel *titleLabel = new QLabel("Smart Demand Calendar", headerFrame);
    titleLabel->setObjectName("titleLabel");
    QLabel *subtitleLabel = new QLabel("Interface matiere premieres - previsions et alertes", headerFrame);
    subtitleLabel->setObjectName("subtitleLabel");

    QVBoxLayout *titlesLayout = new QVBoxLayout;
    titlesLayout->setSpacing(2);
    titlesLayout->addWidget(titleLabel);
    titlesLayout->addWidget(subtitleLabel);

    m_themeButton = new QToolButton(headerFrame);
    m_themeButton->setText("Switch Theme");
    m_themeButton->setCursor(Qt::PointingHandCursor);

    headerLayout->addLayout(titlesLayout, 1);
    headerLayout->addWidget(m_themeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, central);

    QWidget *mainPanel = new QWidget(splitter);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPanel);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    m_calendar = new QCalendarWidget(mainPanel);
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setNavigationBarVisible(true);

    QWidget *filterRow = new QWidget(mainPanel);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterRow);
    filterLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *categoryLabel = new QLabel("Category:", filterRow);
    m_categoryFilter = new QComboBox(filterRow);
    m_categoryFilter->addItems(m_categories);

    filterLayout->addWidget(categoryLabel);
    filterLayout->addWidget(m_categoryFilter);
    filterLayout->addStretch();

    QWidget *apiRow = new QWidget(mainPanel);
    QHBoxLayout *apiLayout = new QHBoxLayout(apiRow);
    apiLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *apiLabel = new QLabel("Google Calendar API:", apiRow);
    m_apiUrlInput = new QLineEdit(apiRow);
    m_apiUrlInput->setPlaceholderText(QString::fromLatin1(kGoogleCalendarEventsBaseUrl) + QStringLiteral("?")
                                      + QString::fromLatin1(kGoogleCalendarDefaultQuery));
    m_apiUrlInput->setText(QString::fromLatin1(kGoogleCalendarEventsBaseUrl) + QStringLiteral("?")
                           + QString::fromLatin1(kGoogleCalendarDefaultQuery));
    m_syncApiButton = new QPushButton("Sync", apiRow);
    m_syncApiButton->setCursor(Qt::PointingHandCursor);
    m_apiStatusLabel = new QLabel("API status: idle", apiRow);
    m_apiStatusLabel->setObjectName("apiStatusLabel");

    apiLayout->addWidget(apiLabel);
    apiLayout->addWidget(m_apiUrlInput, 1);
    apiLayout->addWidget(m_syncApiButton);

    m_monthSummaryLabel = new QLabel(mainPanel);
    m_monthSummaryLabel->setWordWrap(true);
    m_monthSummaryLabel->setObjectName("cardLabel");

    m_notificationsLabel = new QLabel(mainPanel);
    m_notificationsLabel->setWordWrap(true);
    m_notificationsLabel->setObjectName("cardWarning");

    m_predictedSalesLabel = new QLabel(mainPanel);
    m_predictedSalesLabel->setWordWrap(true);
    m_predictedSalesLabel->setObjectName("cardLabel");

    mainLayout->addWidget(m_calendar);
    mainLayout->addWidget(filterRow);
    mainLayout->addWidget(apiRow);
    mainLayout->addWidget(m_apiStatusLabel);
    mainLayout->addWidget(m_monthSummaryLabel);
    mainLayout->addWidget(m_notificationsLabel);
    mainLayout->addWidget(m_predictedSalesLabel);

    QWidget *sidePanel = new QWidget(splitter);
    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(10);

    QLabel *recommendationTitle = new QLabel("Recommendations", sidePanel);
    recommendationTitle->setObjectName("sideTitle");

    m_recommendationsList = new QListWidget(sidePanel);
    m_recommendationsList->setObjectName("recommendationsList");

    sideLayout->addWidget(recommendationTitle);
    sideLayout->addWidget(m_recommendationsList, 1);

    splitter->addWidget(mainPanel);
    splitter->addWidget(sidePanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    rootLayout->addWidget(headerFrame);
    rootLayout->addWidget(splitter, 1);

    setCentralWidget(central);
}

void AssistantWindow::initializeData()
{
    const QDate today = QDate::currentDate();
    const QStringList baseCategories = {"Bags", "Shoes", "Jackets"};

    for (int i = 0; i < 180; ++i) {
        const QDate date = today.addDays(-i);
        const int monthBias = (date.month() == 11 || date.month() == 12) ? 30 : 0;
        const double baseSales = 240.0 + monthBias + QRandomGenerator::global()->bounded(120);
        m_pastSalesByDate[date] = baseSales;

        for (const QString &category : baseCategories) {
            CalendarEvent event;
            event.date = date;
            event.category = category;
            event.title = category + " demand";
            event.demandScore = 45 + QRandomGenerator::global()->bounded(35) + (monthBias / 3);
            event.expectedSales = baseSales + QRandomGenerator::global()->bounded(80);
            m_events.append(event);
        }
    }
}

void AssistantWindow::simulateBusySeasons(const QDate &monthDate)
{
    m_busyScoreByDate.clear();

    const QDate first(monthDate.year(), monthDate.month(), 1);
    const QDate last = first.addMonths(1).addDays(-1);

    for (QDate day = first; day <= last; day = day.addDays(1)) {
        int score = 30 + QRandomGenerator::global()->bounded(30);

        if (day.month() == 11 || day.month() == 12) {
            score += 35;
        }
        if (day.month() == 6 || day.month() == 7) {
            score += 18;
        }
        if (day.dayOfWeek() == Qt::Saturday || day.dayOfWeek() == Qt::Sunday) {
            score += 8;
        }

        m_busyScoreByDate[day] = qBound(0, score, 100);
    }
}

void AssistantWindow::refreshCalendarAppearance()
{
    if (!m_calendar) {
        return;
    }

    QTextCharFormat defaultFmt;
    defaultFmt.setForeground(m_isDarkMode ? QColor(230, 232, 240) : QColor(30, 32, 45));
    defaultFmt.setBackground(Qt::transparent);

    const QDate shown = m_calendar->monthShown() > 0
                            ? QDate(m_calendar->yearShown(), m_calendar->monthShown(), 1)
                            : QDate::currentDate();
    const QDate first(shown.year(), shown.month(), 1);
    const QDate last = first.addMonths(1).addDays(-1);

    for (QDate day = first; day <= last; day = day.addDays(1)) {
        m_calendar->setDateTextFormat(day, defaultFmt);
    }

    for (auto it = m_busyScoreByDate.constBegin(); it != m_busyScoreByDate.constEnd(); ++it) {
        if (it.value() < kWarningThreshold) {
            continue;
        }

        QTextCharFormat fmt = defaultFmt;
        const bool highDemand = it.value() >= kBusyThreshold;
        const QColor mark = busyColor(m_isDarkMode);

        if (highDemand) {
            fmt.setBackground(mark);
            fmt.setForeground(Qt::white);
            fmt.setFontWeight(QFont::Bold);
        } else {
            QColor soft = mark;
            soft.setAlpha(95);
            fmt.setBackground(soft);
            fmt.setFontWeight(QFont::DemiBold);
        }
        m_calendar->setDateTextFormat(it.key(), fmt);
    }
}

void AssistantWindow::refreshInsights()
{
    const QDate selected = m_calendar->selectedDate();
    const QString category = m_categoryFilter->currentText();
    const int score = m_busyScoreByDate.value(selected, 45);
    const double predicted = predictSales(selected, category);

    m_monthSummaryLabel->setText(
        QString("Date: %1 | Category: %2 | Demand score: %3/100")
            .arg(selected.toString("dddd dd MMM yyyy"))
            .arg(category)
            .arg(score));

    m_predictedSalesLabel->setText(
        QString("Predicted sales: %1 units (based on seasonal trend + history)")
            .arg(QString::number(predicted, 'f', 1)));

    updateNotifications();
}

void AssistantWindow::updateRecommendations()
{
    m_recommendationsList->clear();

    const QDate selected = m_calendar->selectedDate();
    const QString category = m_categoryFilter->currentText();
    const int score = m_busyScoreByDate.value(selected, 50);
    const double predicted = predictSales(selected, category);
    const bool allCategories = (category.compare("All", Qt::CaseInsensitive) == 0);

    if (score >= kBusyThreshold) {
        m_recommendationsList->addItem("Increase raw material stock by 25-30% before this period.");
        m_recommendationsList->addItem("Prioritize supplier lead-time confirmation this week.");
    } else if (score >= kWarningThreshold) {
        m_recommendationsList->addItem("Prepare safety stock (+15%) for medium-high demand.");
        m_recommendationsList->addItem("Launch targeted promotions 5 days in advance.");
    } else {
        m_recommendationsList->addItem("Demand is stable: keep baseline procurement levels.");
        m_recommendationsList->addItem("Focus budget on awareness campaigns.");
    }

    if (predicted > 360.0) {
        m_recommendationsList->addItem("Schedule extra warehouse staffing for expected order volume.");
    }

    if (allCategories) {
        m_recommendationsList->addItem("Top opportunities: Bags and Jackets for seasonal campaigns.");
    } else if (category == "Bags") {
        m_recommendationsList->addItem("Bags: push premium bundles and cross-sell maintenance kits.");
    } else if (category == "Shoes") {
        m_recommendationsList->addItem("Shoes: emphasize fast-moving sizes and replenishment cadence.");
    } else if (category == "Jackets") {
        m_recommendationsList->addItem("Jackets: anticipate weather peaks and highlight new arrivals.");
    }
}

void AssistantWindow::updateNotifications()
{
    const QDate selected = m_calendar->selectedDate();
    const int score = m_busyScoreByDate.value(selected, 45);

    if (score >= kBusyThreshold) {
        m_notificationsLabel->setText("High demand alert: risk of stock pressure. Trigger replenishment workflow.");
    } else if (score >= kWarningThreshold) {
        m_notificationsLabel->setText("Demand warning: monitor stock turns and supplier response times.");
    } else {
        m_notificationsLabel->setText("Normal demand period: no urgent action required.");
    }
}

void AssistantWindow::applyTheme(bool darkMode)
{
    m_isDarkMode = darkMode;
    m_themeButton->setText(darkMode ? "Light Mode" : "Dark Mode");

    const QString bg = darkMode ? "#12151C" : "#F7F8FC";
    const QString panel = darkMode ? "#1D2330" : "#FFFFFF";
    const QString text = darkMode ? "#E6E8F0" : "#1E202D";
    const QString sub = darkMode ? "#A8AEC0" : "#5C6070";
    const QString border = darkMode ? "#333D55" : "#D7DBEA";
    const QString accent = darkMode ? "#7C9DFF" : "#4C6FFF";
    const QString cardWarning = darkMode ? "#402127" : "#FFE8EB";

    setStyleSheet(QString(
                      "QMainWindow, QWidget { background:%1; color:%2; }"
                      "#headerFrame { background:%3; border:1px solid %4; border-radius:12px; }"
                      "#titleLabel { font-size:20px; font-weight:700; color:%2; }"
                      "#subtitleLabel { color:%5; }"
                      "QToolButton { background:%6; color:white; border:none; border-radius:8px; padding:8px 12px; font-weight:600; }"
                      "QToolButton:hover { background:#6A88FF; }"
                      "QCalendarWidget QWidget { alternate-background-color:%3; }"
                      "QCalendarWidget QToolButton { color:%2; background:%3; border:1px solid %4; border-radius:6px; padding:4px 8px; }"
                      "QCalendarWidget QMenu { background:%3; color:%2; }"
                      "QCalendarWidget QSpinBox { background:%3; color:%2; selection-background-color:%6; }"
                      "QCalendarWidget QAbstractItemView:enabled { color:%2; background:%1; selection-background-color:%6; selection-color:white; }"
                      "QLineEdit, QPushButton { background:%3; color:%2; border:1px solid %4; border-radius:8px; padding:6px; }"
                      "QPushButton { background:%6; color:white; border:none; font-weight:600; padding:7px 12px; }"
                      "QPushButton:hover { background:#6A88FF; }"
                      "QComboBox, QListWidget { background:%3; color:%2; border:1px solid %4; border-radius:8px; padding:6px; }"
                      "#cardLabel { background:%3; border:1px solid %4; border-radius:10px; padding:10px; }"
                      "#cardWarning { background:%7; border:1px solid %4; border-radius:10px; padding:10px; font-weight:600; }"
                      "#apiStatusLabel { color:%5; font-weight:600; }"
                      "#sideTitle { font-size:16px; font-weight:700; color:%2; }")
                      .arg(bg, text, panel, border, sub, accent, cardWarning));

    refreshCalendarAppearance();
}

double AssistantWindow::predictSales(const QDate &date, const QString &category) const
{
    const bool allCategories = (category.compare("All", Qt::CaseInsensitive) == 0);
    const double base = m_pastSalesByDate.value(date.addYears(-1), 250.0);
    const int seasonalScore = m_busyScoreByDate.value(date, 50);
    const double seasonalBoost = 1.0 + (static_cast<double>(seasonalScore) / 250.0);

    double categoryMultiplier = 1.0;
    if (!allCategories) {
        if (category == "Bags") {
            categoryMultiplier = 1.08;
        } else if (category == "Shoes") {
            categoryMultiplier = 1.15;
        } else if (category == "Jackets") {
            categoryMultiplier = 1.22;
        }
    }
    return base * seasonalBoost * categoryMultiplier;
}

bool AssistantWindow::eventMatchesCurrentCategory(const CalendarEvent &event) const
{
    const QString current = m_categoryFilter->currentText();
    return current.compare("All", Qt::CaseInsensitive) == 0
           || event.category.compare(current, Qt::CaseInsensitive) == 0;
}

void AssistantWindow::onDateChanged()
{
    refreshInsights();
    updateRecommendations();
}

void AssistantWindow::onCategoryChanged()
{
    refreshInsights();
    updateRecommendations();
}

void AssistantWindow::onThemeToggled()
{
    applyTheme(!m_isDarkMode);
}

void AssistantWindow::onApiReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_apiStatusLabel->setText("API status: failed - " + reply->errorString());
        reply->deleteLater();
        return;
    }

    const QByteArray body = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject root = doc.object();
    QJsonArray items = root.value("events").toArray();
    const bool isGoogleCalendar = items.isEmpty() && root.contains("items");
    if (isGoogleCalendar) {
        items = root.value("items").toArray();
    }

    int loaded = 0;
    for (const QJsonValue &item : items) {
        const QJsonObject obj = item.toObject();
        QDate date;
        QString title;
        QString category;
        int demandScore = 70;
        double expectedSales = 320.0;

        if (isGoogleCalendar) {
            title = obj.value("summary").toString("Google Event");
            const QJsonObject startObj = obj.value("start").toObject();
            const QString rawDate = startObj.value("date").toString();
            const QString rawDateTime = startObj.value("dateTime").toString();
            date = QDate::fromString(rawDate, Qt::ISODate);
            if (!date.isValid() && !rawDateTime.isEmpty()) {
                date = QDateTime::fromString(rawDateTime, Qt::ISODate).date();
            }

            const QString summaryLower = title.toLower();
            if (summaryLower.contains("shoe")) {
                category = "Shoes";
            } else if (summaryLower.contains("jacket")) {
                category = "Jackets";
            } else {
                category = "Bags";
            }

            demandScore = 68 + QRandomGenerator::global()->bounded(24);
            expectedSales = 300.0 + QRandomGenerator::global()->bounded(140);
        } else {
            date = QDate::fromString(obj.value("date").toString(), Qt::ISODate);
            title = obj.value("title").toString("API Event");
            category = obj.value("category").toString("Bags");
            demandScore = obj.value("demandScore").toInt(70);
            expectedSales = obj.value("expectedSales").toDouble(320.0);
        }

        if (!date.isValid()) {
            continue;
        }

        CalendarEvent ev;
        ev.date = date;
        ev.title = title;
        ev.category = category;
        ev.demandScore = demandScore;
        ev.expectedSales = expectedSales;
        m_events.append(ev);
        ++loaded;

        const int existing = m_busyScoreByDate.value(date, 50);
        m_busyScoreByDate[date] = qBound(0, qMax(existing, ev.demandScore), 100);
    }

    refreshCalendarAppearance();
    refreshInsights();
    updateRecommendations();
    m_apiStatusLabel->setText(QString("API status: synced %1 events").arg(loaded));
}

void AssistantWindow::loadCalendarFromApi()
{
    QUrl endpoint = normalizedGoogleCalendarUrl(m_apiUrlInput->text());
    if (!endpoint.isValid()) {
        m_apiStatusLabel->setText("API status: invalid URL");
        return;
    }

    QUrlQuery query(endpoint);
    if (query.hasQueryItem(QStringLiteral("key"))) {
        query.removeAllQueryItems(QStringLiteral("key"));
        endpoint.setQuery(query);
    }
    if (m_apiUrlInput)
        m_apiUrlInput->setText(endpoint.toString(QUrl::FullyEncoded));

    QNetworkRequest request(endpoint);
    request.setRawHeader("Accept", "application/json");

    if (endpoint.host().contains(QStringLiteral("googleapis.com"), Qt::CaseInsensitive)) {
        const QString token = loadGoogleAccessTokenFromStorage();
        if (token.isEmpty()) {
            m_apiStatusLabel->setText("API status: failed (OAuth token missing)");
            return;
        }
        request.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + token.toUtf8());
    }

    m_networkManager->get(request);
    m_apiStatusLabel->setText("API status: loading...");
}
