#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <QString>
#include <QList>
#include <QSet>
#include <QMultiMap>
#include <QDomElement>
#include "Reminder.h"
#include "DataElem_GEvent.h"
#include "DataElem_TaskState.h"
#include "DataElem_TaskState.h"
#include "Util_Task.h"
#include "DataElem_TaskDayScheduleSession.h"
typedef clUtil_Task clTask;
typedef clDataElem_GEvent clGEvent;
typedef clDataElem_TaskDayScheduleSession clTaskDayScheduleSession;

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
    void read_overdue_tasks(QList<clUtil_Task> &overdue_reminders, QDate &last_traced_day);
    bool save_overdue_tasks(const QList<clUtil_Task> &overdue_tasks,
                            const QDate &last_traced_day);

    //
    void read_rem_deadlines_states_in_history(const int id,
                                         const QDate &deadline_min, const QDate &deadline_max,
                                         QMap<QDate,clDataElem_TaskState> &deadline_states);
                    //Read the reminder-deadline state history to get the recorded tasks (and
                    //their latest states) involving reminder `id` with deadline within
                    //[`deadline_min`, `deadline_max`].

    void read_rem_deadline_states(const int id,
                                  //
                                  const QList<QDate> &query_deadlines,
                                  QList<clDataElem_TaskState> &state_of_query_deadlines,
                                  //
                                  const QDate &max_shfited_date,
                                  QList<QDate> &deadlines_w_date_shifted,
                                  QList<QDate> &shifted_dates,
                                  //
                                  const QDate &record_date,
                                  QList<QDate> &deadlines_w_record_on_date,
                                  QList<clDataElem_TaskState> &states_on_date);
                                  //see .cpp for desciption

    void add_task_state_update(const clUtil_Task &task, const clDataElem_TaskState &new_state,
                               const QDate &update_date);

    //
    void read_task_day_scheduling(const QDate &date,
                                  QMap<clTask, QList<clTaskDayScheduleSession> > &TaskSessions);
    void save_task_day_scheduling(
                            const QDate &date,
                            const QMap<clTask, QList<clTaskDayScheduleSession> > &TaskSessions);

private:
    QString mDataDir;
};

#endif // DATA_STORAGE_H
