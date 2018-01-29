#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <QString>
#include <QList>
#include <QSet>
#include <QMultiMap>
#include <QDomElement>
#include "Reminder.h"
#include "DataElem_GEvent.h"
#include "DataElem_ScheduleStatus.h"
typedef clDataElem_GEvent clGEvent;

class clFileReadWrite
{
public:
    clFileReadWrite();

    //
    QList<clReminder *> read_all_reminders();
                        //The returned `clReminder` objects are created and are not delete here.
    bool save_reminder(const clReminder &reminder, QString &error_msg);
    bool delete_reminder(const int id); //return true iff done

    //
    QSet<QString> read_all_situations();
    QSet<QString> read_all_events();
    bool save_situations(const QSet<QString> &situations);
    bool save_events(const QSet<QString> &events);

    //
    QMultiMap<QDateTime, clGEvent> read_gevent_history();
    bool save_gevent_history(const QMultiMap<QDateTime, clGEvent> &history);

    //
    QMap<int,clDataElem_ScheduleStatus> read_day_planning_status(const QDate &date);
                                           //Return empty map if not found or there's error.
    bool save_day_planning_status(const QDate &date,
                                  const QMap<int,clDataElem_ScheduleStatus> &status);
         //Unscheduled items will be ignored.
         //Remove the file if `status` is empty or every `status[]` is "unscheduled".

private:
    QString mDataDir;
};

#endif // DATA_STORAGE_H
