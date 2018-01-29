#include <QDebug>
#include "AlarmClockService.h"

clAlarmClockService::clAlarmClockService(QObject *parent)
    : QObject(parent)
{
    connect(&mMessageClock, SIGNAL(notify(QDateTime,QString)),
            this, SLOT(On_time_reached(QDateTime,QString)));
}

void clAlarmClockService::schedule_alarm(int rem_id, const QDateTime &at)
// Schedule an alarm for 'rem_id' at time 'at', so that when 'at' is reached,
// signal `time_reached(at, rem_id)` will be emitted.
{
    if(mScheduledTimes.contains(rem_id, at))
        return;

    mMessageClock.schedule(at, QString::number(rem_id));
    mScheduledTimes.insert(rem_id, at);
}

void clAlarmClockService::On_time_reached(const QDateTime &t, const QString &message)
{
    // get reminder ID from 'message'
    bool ok;
    int rem_id = message.toInt(&ok);
    Q_ASSERT(ok);

    //
    emit time_reached(t, rem_id);
    mScheduledTimes.remove(rem_id, t);
}

/*
void clAlarmClockService::get_all_scheduled_alarms(QList<QDateTime> &times,
                                                   QStringList &reminder_titles)
{
    times.clear();
    reminder_titles.clear();

    QMultiMap<QDateTime, QString> scheduled_messages = mMessageClock.get_all_scheduled_messages();
    for(auto it=scheduled_messages.begin(); it!=scheduled_messages.end(); it++)
    {
        bool ok;
        int msg_id = it.value().toInt(&ok);
        Q_ASSERT(ok);

        Q_ASSERT(mSchedulers.contains(msg_id));
        clReminderSpec *rem_spec = mSchedulers.value(msg_id);
        QString rem_title = rem_spec->get_reminder_title();

        //
        times << it.key();
        reminder_titles << rem_title;
    }
}
    */
