#include "busyseasonservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <cmath>

namespace {
constexpr int kLevelLow = 1;
constexpr int kLevelMedium = 2;
constexpr int kLevelHigh = 3;
constexpr int kLevelVeryHigh = 4;

QDate estimatedIslamicHolidayDate(int year, const QDate &referenceDate, int referenceYear)
{
    if (!referenceDate.isValid())
        return QDate();

    const int yearDelta = year - referenceYear;
    // Hijri year is ~10.875 days shorter than Gregorian year.
    const int daysShift = static_cast<int>(std::lround(10.875 * static_cast<double>(yearDelta)));
    return referenceDate.addDays(-daysShift);
}

QDate estimatedEidAlFitrDate(int year)
{
    // Reference anchored on 2026 (can drift by ~1 day depending on moon sighting).
    return estimatedIslamicHolidayDate(year, QDate(2026, 3, 20), 2026);
}

QDate estimatedEidAlAdhaDate(int year)
{
    // Reference anchored on 2026 (can drift by ~1 day depending on moon sighting).
    return estimatedIslamicHolidayDate(year, QDate(2026, 5, 27), 2026);
}

void addPreEidWindow(QList<Event> *events, const QDate &eidDate, const QString &label)
{
    if (!events || !eidDate.isValid())
        return;
    Event preEid;
    preEid.start = eidDate.addDays(-10);
    preEid.end = eidDate.addDays(-1);
    preEid.level = QStringLiteral("HIGH");
    preEid.source = QStringLiteral("Internal Rule: 10 days before %1 (estimated)").arg(label);
    events->push_back(preEid);
}
}

BusySeasonService::BusySeasonService(QObject *parent)
    : QObject(parent)
{
}

void BusySeasonService::loadBusySeasonData(int year, const QString &countryCode)
{
    if (m_isLoading)
        return;

    qDebug() << "=== BusySeasonService::loadBusySeasonData ===";
    qDebug() << "Year:" << year << "Country:" << countryCode;

    m_isLoading = true;
    m_currentYear = year;
    m_state = PendingRequestState();

    const QString calendarificKey = qEnvironmentVariable("CALENDARIFIC_API_KEY").trimmed();
    const bool useCalendarific = !calendarificKey.isEmpty();
    m_state.expectedReplies = 12 + (useCalendarific ? 1 : 0);

    if (useCalendarific) {
        const QNetworkRequest calReq = makeCalendarificRequest(year, countryCode);
        qDebug() << "=== CALLING CALENDARIFIC ===";
        qDebug() << "[Calendarific] Final URL:" << calReq.url().toString();
        QNetworkReply *calReply = m_network.get(calReq);
        connect(calReply, &QNetworkReply::finished, this, [this, calReply]() {
            onApiReplyFinished(calReply);
        });
    } else {
        qDebug() << "[Calendarific] Skipped: CALENDARIFIC_API_KEY is not set.";
    }

    for (int month = 1; month <= 12; ++month) {
        const QNetworkRequest aladhanReq = makeAladhanRequest(month, year);
        qDebug() << "=== CALLING ALADHAN ===";
        qDebug() << "[Aladhan] Final URL:" << aladhanReq.url().toString();
        QNetworkReply *reply = m_network.get(aladhanReq);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            onApiReplyFinished(reply);
        });
    }
}

QNetworkRequest BusySeasonService::makeCalendarificRequest(int year, const QString &countryCode) const
{
    // Official Calendarific endpoint (api.calendarific.com may fail DNS in some environments).
    QUrl url(QStringLiteral("https://calendarific.com/api/v2/holidays"));
    QUrlQuery query;

    // Calendarific key must come from environment variable CALENDARIFIC_API_KEY.
    const QString apiKey = qEnvironmentVariable("CALENDARIFIC_API_KEY").trimmed();
    if (apiKey.isEmpty())
        qDebug() << "[Calendarific] CALENDARIFIC_API_KEY is empty. Request may fail with 401/403.";
    query.addQueryItem(QStringLiteral("api_key"), apiKey);
    query.addQueryItem(QStringLiteral("country"), countryCode);
    query.addQueryItem(QStringLiteral("year"), QString::number(year));

    url.setQuery(query);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    return req;
}

QNetworkRequest BusySeasonService::makeAladhanRequest(int month, int year) const
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("api.aladhan.com"));
    url.setPath(QStringLiteral("/v1/calendarByCity/%1/%2").arg(year).arg(month));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("city"), QStringLiteral("Tunis"));
    query.addQueryItem(QStringLiteral("country"), QStringLiteral("Tunisia"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    return req;
}

void BusySeasonService::onApiReplyFinished(QNetworkReply *reply)
{
    if (!reply)
        return;

    m_state.completedReplies++;
    const QByteArray payload = reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString host = reply->request().url().host().toLower();
    const QString finalUrl = reply->request().url().toString();

    qDebug() << "=== API REPLY FINISHED ===";
    qDebug() << "URL:" << finalUrl;
    qDebug() << "HTTP Status:" << status;
    qDebug() << "Response:" << payload;

    if (reply->error() == QNetworkReply::NoError && status >= 200 && status < 300) {
        m_state.hadAnySuccess = true;
        if (host.contains(QStringLiteral("calendarific"))) {
            parseCalendarific(payload, m_state);
        } else if (host.contains(QStringLiteral("aladhan"))) {
            parseAladhan(payload, m_state);
        }
    } else {
        qDebug() << "Network Error:" << reply->errorString();
        m_state.errors << QStringLiteral("%1 -> %2")
                              .arg(reply->request().url().host(), reply->errorString());
    }

    reply->deleteLater();

    if (m_state.completedReplies >= m_state.expectedReplies) {
        enrichRules(m_state, m_currentYear);
        finalizeAndEmit(m_state);
        m_isLoading = false;
    }
}

void BusySeasonService::parseCalendarific(const QByteArray &payload, PendingRequestState &state) const
{
    const int before = state.parsedEvents.size();
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject())
        return;

    const QJsonObject root = doc.object();
    const QJsonArray holidays = root.value(QStringLiteral("response"))
                                    .toObject()
                                    .value(QStringLiteral("holidays"))
                                    .toArray();

    for (const QJsonValue &value : holidays) {
        const QJsonObject obj = value.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString().trimmed();
        const QString iso = obj.value(QStringLiteral("date"))
                                .toObject()
                                .value(QStringLiteral("iso"))
                                .toString();
        const QDate day = QDate::fromString(iso.left(10), Qt::ISODate);
        if (!day.isValid())
            continue;

        Event event;
        event.start = day;
        event.end = day;
        event.level = QStringLiteral("LOW");
        event.source = QStringLiteral("Calendarific: %1").arg(name.isEmpty() ? QStringLiteral("General event") : name);
        state.parsedEvents.push_back(event);
    }
    qDebug() << "[Calendarific] Parsed events:" << (state.parsedEvents.size() - before);
}

void BusySeasonService::parseAladhan(const QByteArray &payload, PendingRequestState &state) const
{
    const int before = state.parsedEvents.size();
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject())
        return;

    const QJsonArray days = doc.object().value(QStringLiteral("data")).toArray();
    QList<QDate> ramadanDays;
    QList<QDate> eidDays;
    QStringList eidLabels;

    for (const QJsonValue &value : days) {
        const QJsonObject obj = value.toObject();
        const QString gregDate = obj.value(QStringLiteral("gregorian"))
                                     .toObject()
                                     .value(QStringLiteral("date"))
                                     .toString();
        const QDate gDate = QDate::fromString(gregDate, QStringLiteral("dd-MM-yyyy"));
        if (!gDate.isValid())
            continue;

        const QJsonObject hijri = obj.value(QStringLiteral("hijri")).toObject();
        const QString hijriMonth = hijri.value(QStringLiteral("month"))
                                        .toObject()
                                        .value(QStringLiteral("en"))
                                        .toString();
        if (isRamadanMonthToken(hijriMonth))
            ramadanDays.push_back(gDate);

        const QJsonArray holidays = hijri.value(QStringLiteral("holidays")).toArray();
        for (const QJsonValue &h : holidays) {
            const QString holiday = h.toString().trimmed();
            if (isEidHolidayToken(holiday)) {
                eidDays.push_back(gDate);
                eidLabels.push_back(holiday);
            }
        }
    }

    const QList<Event> ramadanRanges = buildRamadanRanges(ramadanDays);
    for (const Event &e : ramadanRanges)
        state.parsedEvents.push_back(e);

    for (int i = 0; i < eidDays.size(); ++i) {
        const QDate eidDate = eidDays.at(i);
        const QDate start = eidDate.addDays(-10);
        const QDate end = eidDate.addDays(-1);
        if (!start.isValid() || !end.isValid() || end < start)
            continue;

        Event preEid;
        preEid.start = start;
        preEid.end = end;
        preEid.level = QStringLiteral("HIGH");
        const QString label = i < eidLabels.size() ? eidLabels.at(i) : QStringLiteral("Eid");
        preEid.source = QStringLiteral("Aladhan: 10 days before %1").arg(label);
        state.parsedEvents.push_back(preEid);
    }
    qDebug() << "[Aladhan] Parsed events:" << (state.parsedEvents.size() - before);
}

void BusySeasonService::enrichRules(PendingRequestState &state, int year) const
{
    Event summer;
    summer.start = QDate(year, 6, 1);
    summer.end = QDate(year, 8, 31);
    summer.level = QStringLiteral("MEDIUM");
    summer.source = QStringLiteral("Internal Rule: Summer season (June-August)");
    state.parsedEvents.push_back(summer);

    // Tunisia school start typically drives demand from late August through September.
    Event schoolStart;
    schoolStart.start = QDate(year, 8, 25);
    schoolStart.end = QDate(year, 9, 30);
    schoolStart.level = QStringLiteral("HIGH");
    schoolStart.source = QStringLiteral("Internal Rule: Back-to-school period (late Aug-Sep)");
    state.parsedEvents.push_back(schoolStart);

    // Fallback windows to keep Eid demand visible when APIs fail.
    addPreEidWindow(&state.parsedEvents, estimatedEidAlFitrDate(year), QStringLiteral("Eid al-Fitr"));
    addPreEidWindow(&state.parsedEvents, estimatedEidAlAdhaDate(year), QStringLiteral("Eid al-Adha"));
}

void BusySeasonService::finalizeAndEmit(PendingRequestState &state)
{
    QHash<QDate, int> rankPerDay;
    QHash<QDate, int> overlapCount;
    QHash<QDate, QStringList> detailsPerDay;

    for (const Event &e : state.parsedEvents) {
        if (!e.start.isValid() || !e.end.isValid() || e.end < e.start)
            continue;

        const int r = rankFromLevel(e.level);
        for (QDate d = e.start; d <= e.end; d = d.addDays(1)) {
            rankPerDay[d] = std::max(rankPerDay.value(d, 0), r);
            overlapCount[d] = overlapCount.value(d, 0) + 1;
            detailsPerDay[d].append(QStringLiteral("%1 [%2]").arg(e.source, e.level));
        }
    }

    QHash<QDate, QString> levels;
    for (auto it = rankPerDay.constBegin(); it != rankPerDay.constEnd(); ++it) {
        const QDate date = it.key();
        int rank = it.value();
        if (overlapCount.value(date, 0) >= 2)
            rank = kLevelVeryHigh;
        levels.insert(date, levelFromRank(rank));
    }

    if (!state.hadAnySuccess && state.parsedEvents.isEmpty()) {
        emit loadFailed(state.errors.join(QStringLiteral("\n")));
        return;
    }

    emit busySeasonReady(state.parsedEvents, levels, detailsPerDay);
}

QString BusySeasonService::normalizeHolidayName(const QString &value)
{
    QString out = value.toLower().trimmed();
    out.replace(QChar(0x2019), QChar('\''));
    out.replace(QChar(0x2013), QChar('-'));
    out.replace(QChar(0x2014), QChar('-'));
    return out;
}

QString BusySeasonService::normalizeToken(const QString &value)
{
    QString out = normalizeHolidayName(value);
    out.replace(QStringLiteral("ḍ"), QStringLiteral("d"));
    out.replace(QStringLiteral("ḥ"), QStringLiteral("h"));
    out.replace(QStringLiteral("ṭ"), QStringLiteral("t"));
    out.replace(QStringLiteral("ṣ"), QStringLiteral("s"));
    out.replace(QStringLiteral("ā"), QStringLiteral("a"));
    out.replace(QStringLiteral("ī"), QStringLiteral("i"));
    out.replace(QStringLiteral("ū"), QStringLiteral("u"));
    out.replace(QStringLiteral("â"), QStringLiteral("a"));
    out.replace(QStringLiteral("î"), QStringLiteral("i"));
    out.replace(QStringLiteral("û"), QStringLiteral("u"));
    out.replace(QRegularExpression(QStringLiteral("[^a-z0-9]")), QStringLiteral(""));
    return out;
}

bool BusySeasonService::isRamadanMonthToken(const QString &value)
{
    const QString token = normalizeToken(value);
    return token == QStringLiteral("ramadan") ||
           token == QStringLiteral("ramadhan") ||
           token.contains(QStringLiteral("ramadan")) ||
           token.contains(QStringLiteral("ramadhan"));
}

bool BusySeasonService::isEidHolidayToken(const QString &value)
{
    const QString token = normalizeToken(value);
    if (!token.contains(QStringLiteral("eid")))
        return false;
    return token.contains(QStringLiteral("fitr")) ||
           token.contains(QStringLiteral("adha"));
}

QString BusySeasonService::levelFromRank(int rank)
{
    switch (rank) {
    case kLevelVeryHigh:
        return QStringLiteral("VERY_HIGH");
    case kLevelHigh:
        return QStringLiteral("HIGH");
    case kLevelMedium:
        return QStringLiteral("MEDIUM");
    default:
        return QStringLiteral("LOW");
    }
}

int BusySeasonService::rankFromLevel(const QString &level)
{
    if (level == QStringLiteral("VERY_HIGH"))
        return kLevelVeryHigh;
    if (level == QStringLiteral("HIGH"))
        return kLevelHigh;
    if (level == QStringLiteral("MEDIUM"))
        return kLevelMedium;
    return kLevelLow;
}

QList<Event> BusySeasonService::buildRamadanRanges(const QList<QDate> &ramadanDays)
{
    if (ramadanDays.isEmpty())
        return {};

    QList<QDate> sorted = ramadanDays;
    std::sort(sorted.begin(), sorted.end());

    QList<Event> ranges;
    QDate start = sorted.first();
    QDate prev = sorted.first();

    for (int i = 1; i < sorted.size(); ++i) {
        const QDate current = sorted.at(i);
        if (prev.addDays(1) == current) {
            prev = current;
            continue;
        }

        Event e;
        e.start = start;
        e.end = prev;
        e.level = QStringLiteral("MEDIUM");
        e.source = QStringLiteral("Aladhan: Ramadan");
        ranges.push_back(e);

        start = current;
        prev = current;
    }

    Event last;
    last.start = start;
    last.end = prev;
    last.level = QStringLiteral("MEDIUM");
    last.source = QStringLiteral("Aladhan: Ramadan");
    ranges.push_back(last);
    return ranges;
}
