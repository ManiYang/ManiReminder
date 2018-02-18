#ifndef TASKSTATESMANAGER_H
#define TASKSTATESMANAGER_H

#include <QObject>
#include "Reminder.h"
#include "DataElem_TaskState.h"
#include "Util_Task.h"
#include "FileReadWrite.h"
typedef clUtil_Task clTask;
typedef clDataElem_TaskState clTaskState;

class clTaskStatesManager : public QObject
{
    Q_OBJECT
public:
    explicit clTaskStatesManager(const QMap<int, const clReminder*> *p_reminders,
                                 clFileReadWrite *p_FileReadWrite,
                                 QObject *parent = nullptr);

    void start_up();

    //
    QList<clTask> get_overdue_tasks() const; //get tasks that are overdue w.r.t. current date

    //
    void date_query(const QDate &date,
                    //
                    QList<clTask> &tasks_active_on_date,
                    QList<clTaskState> &active_tasks_states,
                    //
                    const bool on_date_only,
                    QList<clTask> &tasks_w_date_shifted, QList<QDate> &shifted_dates,
                    //
                    QList<clTask> &tasks_w_state_update_on_date,
                    QList<clTaskState> &updated_state) const;
                    //`tasks_active_on_date[]` will be the tasks with deadline within
                    //[`date`, `date`+N], where N is the precaution day count of the
                    //involved reminder.
                    //`active_tasks_states[i]` will be the latest state of the task
                    //`tasks_active_on_date[i]`.
                    //
                    //`tasks_w_date_shifted[]` will be the tasks with state = `DateShifted`
                    //and shifted-date <= `date` (if `on_date_only` = false) or
                    //shifted-date = `date` (if `on_date_only` = true).
                    //`shifted_dates[i]` will be the shifted date for
                    //`tasks_w_date_shifted[i]`.
                    //
                    //`tasks_w_state_update_on_date[]` will be the tasks with a state update
                    //on `date`. `updated_state[i]` will be the updated state of
                    //`tasks_w_state_update_on_date[i]' on `date`.

public slots:
    void task_state_changed(const clTask &task, const clTaskState &new_state);
    void current_date_changed();
//    void reminder_date_setting_updated(const int &rem_id); //.........

signals:
    void to_show_status_bar_message(const QString &msg);

//--------------

private:
    // data (read-only) //
    const QMap<int, const clReminder*> *pReminders;

    // units //
    clFileReadWrite *pFileReadWrite;

    // internal data
    QList<clTask> mOverdueTasks; //tasks with state "Todo" and deadline < current day
    QDate mLastTracedDay;

    //
    void trace_overdue_tasks();
         //Call this if last traced day is < yesterday.
         //Will read task state history and modify `mOverdueTasks[]`, `mLastTracedDay`.
};

#endif // TASKSTATESMANAGER_H
