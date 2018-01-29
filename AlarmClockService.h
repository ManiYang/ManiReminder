#ifndef CLALARMCLOCKSERVICE_H
#define CLALARMCLOCKSERVICE_H

#include <QObject>
#include <QDateTime>
#include <QMultiMap>
#include <QStringList>
#include "Util_MessageClock.h"

class clAlarmClockService : public QObject
{
    Q_OBJECT

public:
    explicit clAlarmClockService(QObject *parent = nullptr);

    QMultiMap<int,QDateTime> get_all_scheduled_alarms() const { return mScheduledTimes; }

public slots:
    void schedule_alarm(int rem_id, const QDateTime &at);
         // Schedule an alarm for 'rem_id' at time 'at', so that when 'at' is reached,
         // signal `time_reached(at, rem_id)` will be emitted.

signals:
    void time_reached(const QDateTime &t, int rem_id);

private slots:
    void On_time_reached(const QDateTime &t, const QString &message);

private:
    clUtil_MessageClock mMessageClock;
    QMultiMap<int,QDateTime> mScheduledTimes; //mScheduledTimes[rem_id]
};

#endif // CLALARMCLOCKSERVICE_H
