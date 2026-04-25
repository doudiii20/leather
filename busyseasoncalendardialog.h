#ifndef BUSYSEASONCALENDARDIALOG_H
#define BUSYSEASONCALENDARDIALOG_H

#include "busyseasonevent.h"

#include <QDate>
#include <QDialog>
#include <QHash>
#include <QList>
#include <QStringList>

class BusySeasonService;
class QCalendarWidget;
class QLabel;
class QPushButton;

class BusySeasonCalendarDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BusySeasonCalendarDialog(QWidget *parent = nullptr);

private:
    void buildUi();
    void wireSignals();
    void applyCalendarColors();
    void updateDetailsPanel(const QDate &date);
    void refreshDayTooltips();
    QString humanLevel(const QString &level) const;
    QString levelBadge(const QString &level) const;

    BusySeasonService *m_service = nullptr;
    QCalendarWidget *m_calendar = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_details = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_todayButton = nullptr;
    QLabel *m_dateValue = nullptr;
    QLabel *m_scoreValue = nullptr;
    QLabel *m_eventsValue = nullptr;

    QList<Event> m_events;
    QHash<QDate, QString> m_dayLevels;
    QHash<QDate, QStringList> m_dayDetails;
};

#endif // BUSYSEASONCALENDARDIALOG_H
