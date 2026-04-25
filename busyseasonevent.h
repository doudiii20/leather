#ifndef BUSYSEASONEVENT_H
#define BUSYSEASONEVENT_H

#include <QDate>
#include <QString>

struct Event {
    QDate start;
    QDate end;
    QString level; // LOW, MEDIUM, HIGH, VERY_HIGH
    QString source;
};

#endif // BUSYSEASONEVENT_H
