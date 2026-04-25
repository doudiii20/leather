#include "busyseasoncalendardialog.h"

#include "busyseasonservice.h"

#include <QCalendarWidget>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTextCharFormat>
#include <QVBoxLayout>

BusySeasonCalendarDialog::BusySeasonCalendarDialog(QWidget *parent)
    : QDialog(parent)
    , m_service(new BusySeasonService(this))
{
    buildUi();
    wireSignals();
    m_service->loadBusySeasonData(QDate::currentDate().year(), QStringLiteral("TN"));
}

void BusySeasonCalendarDialog::buildUi()
{
    setWindowTitle(QStringLiteral("Smart Busy Season Calendar"));
    resize(960, 760);

    setStyleSheet(QStringLiteral(
        "QDialog { background: #f5f7fc; color: #0f172a; }"
        "QLabel { color: #334155; }"
        "QFrame#card { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; }"
        "QLabel#cardTitle { font-size: 11px; font-weight: 700; color: #64748b; text-transform: uppercase; }"
        "QLabel#cardValue { font-size: 15px; font-weight: 700; color: #0f172a; }"
        "QFrame#legendColorBox { border: 1px solid #cbd5e1; border-radius: 5px; }"
        "QPushButton#actionButton {"
        "  background: #ffffff; color: #1e293b; border: 1px solid #d1d9e6;"
        "  border-radius: 10px; font-weight: 600; padding: 8px 14px; }"
        "QPushButton#actionButton:hover { background: #eef4ff; border-color: #b7c7e6; }"
        "QPushButton#actionButton:pressed { background: #dde9fb; border-color: #9fb6de; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: #ffffff; border-bottom: 1px solid #e2e8f0; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
        "QCalendarWidget QToolButton { color: #1e293b; font-weight: 700; }"
        "QCalendarWidget QSpinBox { color: #1e293b; background: #ffffff; selection-background-color: #d6e7ff; }"
        "QCalendarWidget QAbstractItemView:enabled {"
        "  background: #ffffff; selection-background-color: #bfd6ff; selection-color: #0f172a;"
        "  alternate-background-color: #f8fbff; }"
        "QCalendarWidget QAbstractItemView:item:hover {"
        "  background: #e9f2ff; border: 1px solid #b5caf0; border-radius: 6px; }"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(14);

    auto *headerCard = new QFrame(this);
    headerCard->setObjectName(QStringLiteral("card"));
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 14, 16, 14);
    headerLayout->setSpacing(12);

    auto *titleWrap = new QVBoxLayout();
    titleWrap->setSpacing(3);
    auto *title = new QLabel(QStringLiteral("Smart Busy Season Calendar"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 800; color: #0f172a;"));
    auto *subtitle = new QLabel(QStringLiteral("Vue SaaS des pics d'activite avec couleurs pastel"), this);
    subtitle->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748b;"));
    titleWrap->addWidget(title);
    titleWrap->addWidget(subtitle);
    headerLayout->addLayout(titleWrap, 1);

    m_todayButton = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogContentsView),
                                    QStringLiteral("Calendrier"), this);
    m_todayButton->setObjectName(QStringLiteral("actionButton"));
    m_todayButton->setToolTip(QStringLiteral("Revenir a la date du jour"));
    headerLayout->addWidget(m_todayButton);

    m_refreshButton = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload),
                                      QStringLiteral("Sync"), this);
    m_refreshButton->setObjectName(QStringLiteral("actionButton"));
    m_refreshButton->setToolTip(QStringLiteral("Resynchroniser les donnees API"));
    headerLayout->addWidget(m_refreshButton);
    root->addWidget(headerCard);

    m_status = new QLabel(QStringLiteral("Loading data (Calendarific key required, Aladhan without key)..."), this);
    m_status->setStyleSheet(QStringLiteral("font-size: 12px; color: #475569;"));
    root->addWidget(m_status);

    m_calendar = new QCalendarWidget(this);
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setStyleSheet(QStringLiteral(
        "QCalendarWidget QTableView {"
        "  background: #ffffff; border: 1px solid #e2e8f0; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; }"));
    root->addWidget(m_calendar, 2);

    auto *legendRow = new QHBoxLayout();
    legendRow->setSpacing(10);
    const auto addLegend = [&](const QString &txt, const QString &bg) {
        auto *box = new QFrame(this);
        box->setObjectName(QStringLiteral("legendColorBox"));
        box->setFixedSize(18, 18);
        box->setStyleSheet(QStringLiteral("QFrame#legendColorBox { background:%1; border:1px solid #bfcadb; border-radius:5px; }").arg(bg));
        auto *lab = new QLabel(txt, this);
        lab->setStyleSheet(QStringLiteral("font-size: 11px; color: #475569;"));
        legendRow->addWidget(box);
        legendRow->addWidget(lab);
    };
    addLegend(QStringLiteral("LOW"), QStringLiteral("#dff4ea"));
    addLegend(QStringLiteral("MEDIUM"), QStringLiteral("#fff0d6"));
    addLegend(QStringLiteral("HIGH"), QStringLiteral("#ffdfe4"));
    addLegend(QStringLiteral("VERY_HIGH"), QStringLiteral("#ffc9d3"));
    legendRow->addStretch(1);
    root->addLayout(legendRow);

    auto *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(12);
    const auto buildCard = [&](const QString &titleText, QLabel **valuePtr) {
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("card"));
        auto *lay = new QVBoxLayout(card);
        lay->setContentsMargins(14, 12, 14, 12);
        lay->setSpacing(5);
        auto *ct = new QLabel(titleText, card);
        ct->setObjectName(QStringLiteral("cardTitle"));
        auto *cv = new QLabel(QStringLiteral("-"), card);
        cv->setObjectName(QStringLiteral("cardValue"));
        cv->setWordWrap(true);
        lay->addWidget(ct);
        lay->addWidget(cv);
        cardsRow->addWidget(card, 1);
        *valuePtr = cv;
    };
    buildCard(QStringLiteral("Date"), &m_dateValue);
    buildCard(QStringLiteral("Score"), &m_scoreValue);
    buildCard(QStringLiteral("Evenements"), &m_eventsValue);
    root->addLayout(cardsRow);

    m_details = new QLabel(this);
    m_details->setWordWrap(true);
    m_details->setMinimumHeight(120);
    m_details->setStyleSheet(QStringLiteral(
        "background:#ffffff; border:1px solid #e2e8f0; border-radius:12px;"
        "padding:12px; font-size:12px; color:#334155;"));
    root->addWidget(m_details);
}

void BusySeasonCalendarDialog::wireSignals()
{
    connect(m_service, &BusySeasonService::busySeasonReady, this,
            [this](const QList<Event> &events,
                   const QHash<QDate, QString> &dayLevels,
                   const QHash<QDate, QStringList> &dayDetails) {
                m_events = events;
                m_dayLevels = dayLevels;
                m_dayDetails = dayDetails;
                m_status->setText(QStringLiteral("Loaded %1 event blocks. Click a date to inspect details.")
                                      .arg(m_events.size()));
                applyCalendarColors();
                refreshDayTooltips();
                updateDetailsPanel(m_calendar->selectedDate());
            });

    connect(m_service, &BusySeasonService::loadFailed, this, [this](const QString &err) {
        m_status->setText(QStringLiteral("Failed to load API data."));
        QMessageBox::warning(this, QStringLiteral("Busy Season Calendar"),
                             QStringLiteral("Could not load data from APIs.\n%1").arg(err));
    });

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        updateDetailsPanel(m_calendar->selectedDate());
    });
    connect(m_calendar, &QCalendarWidget::currentPageChanged, this, [this](int, int) {
        applyCalendarColors();
        refreshDayTooltips();
    });

    connect(m_calendar, &QCalendarWidget::clicked, this, [this](const QDate &date) {
        const QString level = m_dayLevels.value(date, QStringLiteral("LOW"));
        const QStringList details = m_dayDetails.value(date);
        QString msg = QStringLiteral("Date: %1\nLoad level: %2")
                          .arg(date.toString(QStringLiteral("dddd dd MMMM yyyy")))
                          .arg(humanLevel(level));
        if (!details.isEmpty())
            msg += QStringLiteral("\n\nEvents:\n- ") + details.join(QStringLiteral("\n- "));
        else
            msg += QStringLiteral("\n\nNo major events on this day.");

        QMessageBox::information(this, QStringLiteral("Busy Season Details"), msg);
    });

    connect(m_todayButton, &QPushButton::clicked, this, [this]() {
        const QDate today = QDate::currentDate();
        m_calendar->setSelectedDate(today);
        m_calendar->setCurrentPage(today.year(), today.month());
        updateDetailsPanel(today);
    });

    connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
        m_status->setText(QStringLiteral("Synchronisation des donnees en cours..."));
        m_service->loadBusySeasonData(m_calendar->yearShown(), QStringLiteral("TN"));
    });
}

void BusySeasonCalendarDialog::applyCalendarColors()
{
    if (!m_calendar)
        return;

    const int y = m_calendar->yearShown();
    const int m = m_calendar->monthShown();
    const QDate first(y, m, 1);
    if (!first.isValid())
        return;

    const int dim = first.daysInMonth();
    for (int day = 1; day <= dim; ++day)
        m_calendar->setDateTextFormat(QDate(y, m, day), QTextCharFormat());

    for (auto it = m_dayLevels.constBegin(); it != m_dayLevels.constEnd(); ++it) {
        QTextCharFormat fmt = m_calendar->dateTextFormat(it.key());
        const QString level = it.value();

        if (level == QStringLiteral("HIGH")) {
            fmt.setBackground(QColor(QStringLiteral("#ffdfe4")));
            fmt.setForeground(QColor(QStringLiteral("#9f1239")));
            fmt.setFontWeight(QFont::DemiBold);
        } else if (level == QStringLiteral("VERY_HIGH")) {
            fmt.setBackground(QColor(QStringLiteral("#ffc9d3")));
            fmt.setForeground(QColor(QStringLiteral("#831843")));
            fmt.setFontWeight(QFont::Bold);
        } else if (level == QStringLiteral("MEDIUM")) {
            fmt.setBackground(QColor(QStringLiteral("#fff0d6")));
            fmt.setForeground(QColor(QStringLiteral("#9a3412")));
        } else {
            fmt.setBackground(QColor(QStringLiteral("#dff4ea")));
            fmt.setForeground(QColor(QStringLiteral("#166534")));
        }
        m_calendar->setDateTextFormat(it.key(), fmt);
    }
}

void BusySeasonCalendarDialog::refreshDayTooltips()
{
    if (!m_calendar)
        return;

    const int y = m_calendar->yearShown();
    const int m = m_calendar->monthShown();
    const QDate first(y, m, 1);
    if (!first.isValid())
        return;

    const int dim = first.daysInMonth();
    for (int day = 1; day <= dim; ++day) {
        const QDate date(y, m, day);
        QTextCharFormat fmt = m_calendar->dateTextFormat(date);
        const QString level = m_dayLevels.value(date, QStringLiteral("LOW"));
        const QStringList details = m_dayDetails.value(date);
        QString tip = QStringLiteral("%1\nScore: %2")
                          .arg(date.toString(QStringLiteral("dddd dd MMMM yyyy")))
                          .arg(levelBadge(level));
        if (!details.isEmpty())
            tip += QStringLiteral("\nEvents:\n- ") + details.join(QStringLiteral("\n- "));
        else
            tip += QStringLiteral("\nNo major events");
        fmt.setToolTip(tip);
        m_calendar->setDateTextFormat(date, fmt);
    }
}

void BusySeasonCalendarDialog::updateDetailsPanel(const QDate &date)
{
    const QString level = m_dayLevels.value(date, QStringLiteral("LOW"));
    const QStringList details = m_dayDetails.value(date);
    m_dateValue->setText(date.toString(QStringLiteral("ddd dd MMM yyyy")));
    m_scoreValue->setText(levelBadge(level));
    m_eventsValue->setText(details.isEmpty() ? QStringLiteral("0 event") : QStringLiteral("%1 events").arg(details.size()));

    QString text = QStringLiteral("Date: %1\nLoad level: %2")
                       .arg(date.toString(QStringLiteral("dddd dd MMMM yyyy")))
                       .arg(humanLevel(level));

    if (!details.isEmpty()) {
        text += QStringLiteral("\n\nEvent details:\n- ") + details.join(QStringLiteral("\n- "));
    } else {
        text += QStringLiteral("\n\nNo major events detected for this day.");
    }
    m_details->setText(text);
}

QString BusySeasonCalendarDialog::humanLevel(const QString &level) const
{
    if (level == QStringLiteral("VERY_HIGH"))
        return QStringLiteral("VERY_HIGH (overlap detected)");
    return level;
}

QString BusySeasonCalendarDialog::levelBadge(const QString &level) const
{
    if (level == QStringLiteral("VERY_HIGH"))
        return QStringLiteral("9 / 10");
    if (level == QStringLiteral("HIGH"))
        return QStringLiteral("7 / 10");
    if (level == QStringLiteral("MEDIUM"))
        return QStringLiteral("5 / 10");
    return QStringLiteral("3 / 10");
}
