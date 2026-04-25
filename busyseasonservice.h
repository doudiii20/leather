#ifndef BUSYSEASONSERVICE_H
#define BUSYSEASONSERVICE_H

#include "busyseasonevent.h"

#include <QDate>
#include <QHash>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class BusySeasonService : public QObject
{
    Q_OBJECT
public:
    explicit BusySeasonService(QObject *parent = nullptr);

    void loadBusySeasonData(int year, const QString &countryCode = QStringLiteral("TN"));

signals:
    void busySeasonReady(const QList<Event> &events,
                         const QHash<QDate, QString> &dayLevels,
                         const QHash<QDate, QStringList> &dayDetails);
    void loadFailed(const QString &errorMessage);

private:
    struct PendingRequestState {
        int expectedReplies = 0;
        int completedReplies = 0;
        bool hadAnySuccess = false;
        QStringList errors;
        QList<Event> parsedEvents;
    };

    QNetworkRequest makeCalendarificRequest(int year, const QString &countryCode) const;
    QNetworkRequest makeAladhanRequest(int month, int year) const;

    void onApiReplyFinished(QNetworkReply *reply);
    void parseCalendarific(const QByteArray &payload, PendingRequestState &state) const;
    void parseAladhan(const QByteArray &payload, PendingRequestState &state) const;
    void enrichRules(PendingRequestState &state, int year) const;
    void finalizeAndEmit(PendingRequestState &state);

    static QString normalizeHolidayName(const QString &value);
    static QString normalizeToken(const QString &value);
    static bool isRamadanMonthToken(const QString &value);
    static bool isEidHolidayToken(const QString &value);
    static QString levelFromRank(int rank);
    static int rankFromLevel(const QString &level);
    static QList<Event> buildRamadanRanges(const QList<QDate> &ramadanDays);

    QNetworkAccessManager m_network;
    bool m_isLoading = false;
    PendingRequestState m_state;
    int m_currentYear = 0;
};

#endif // BUSYSEASONSERVICE_H
