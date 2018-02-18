#include <QDebug>
#include "TaskStatesManager.h"

clTaskStatesManager::clTaskStatesManager(const QMap<int, const clReminder *> *p_reminders,
                                         clFileReadWrite *p_FileReadWrite,
                                         QObject *parent)
    : QObject(parent),
      pReminders(p_reminders), pFileReadWrite(p_FileReadWrite)
{
}

void clTaskStatesManager::start_up()
{
    // get overdue reminders from file
    pFileReadWrite->read_overdue_tasks(mOverdueTasks, mLastTracedDay);
    if(mLastTracedDay.addDays(1) < QDate::currentDate())
        trace_overdue_tasks();
}

void clTaskStatesManager::trace_overdue_tasks()
//Call this if last traced day is < yesterday.
//Will read task state history and modify `mOverdueTasks[]`, `mLastTracedDay`.
{
    const QDate today = QDate::currentDate();
    if(mLastTracedDay == today.addDays(-1))
        return;

    emit to_show_status_bar_message("Trace overdue tasks.");

    // find all tasks with state "todo" and deadline within [`mLastTracedDay`+1, `today`-1]
    const QDate scan_from_date = mLastTracedDay.addDays(1),
                scan_to_date = today.addDays(-1);

    for(auto it=pReminders->cbegin(); it!=pReminders->cend(); it++)
    {
        int id = it.key();
        QList<QDate> deadlines = it.value()->get_due_dates_within(scan_from_date, scan_to_date);

        QMap<QDate,clDataElem_TaskState> deadline_states;
        pFileReadWrite->read_rem_deadlines_states_in_history(id, scan_from_date, scan_to_date,
                                                deadline_states);

        foreach(QDate deadline, deadlines)
        {
            bool is_todo = false;
            if(! deadline_states.contains(deadline))
            {             //(not found in reminder-deadline state history ==> state is "todo")
                is_todo = true;
            }
            else
            {
                if(deadline_states[deadline].get_state()
                                      == clDataElem_TaskState::Todo) //(state is "todo")
                    is_todo = true;
            }

            //
            if(is_todo)
                mOverdueTasks << clTask(id, deadline);
        }
    }

    mLastTracedDay = scan_to_date;

    // save to file
    pFileReadWrite->save_overdue_tasks(mOverdueTasks, mLastTracedDay);
}

QList<clTask> clTaskStatesManager::get_overdue_tasks() const
//get tasks that are overdue w.r.t. current date
{
    return mOverdueTasks;
}

void clTaskStatesManager::current_date_changed()
{
    trace_overdue_tasks();
}

void clTaskStatesManager::date_query(const QDate &date,
                            //
                            QList<clTask> &tasks_active_on_date,
                            QList<clTaskState> &active_tasks_states,
                            //
                            const bool on_date_only,
                            QList<clTask> &tasks_w_date_shifted, QList<QDate> &shifted_dates,
                            //
                            QList<clTask> &tasks_w_state_update_on_date,
                            QList<clTaskState> &updated_state_on_date) const
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
{
    Q_ASSERT(date.isValid());

    tasks_active_on_date.clear();
    active_tasks_states.clear();
    tasks_w_date_shifted.clear();
    shifted_dates.clear();
    tasks_w_state_update_on_date.clear();
    updated_state_on_date.clear();

    //
    for(auto it=pReminders->cbegin(); it!=pReminders->cend(); it++)
    {
        int id = it.key();
        const clReminder *reminder = it.value();

        if(! reminder->has_date_setting())
            continue;

        // get active tasks --> active_deadlines[]
        QList<QDate> active_deadlines;

        int N = reminder->get_precaution_day_counts();
        for(QDate d=date; d<=date.addDays(N); d=d.addDays(1))
        {
            if(reminder->is_due_date(d)) //`d` is a deadline
                active_deadlines << d;
        }

        // get data for reminder `id`
        QList<clTaskState> active_deadlines_states;
        QList<QDate> deadlines_w_date_shifted;
        QList<QDate> deadlines_shifted_dates;
        QList<QDate> deadlines_w_record_on_date;
        QList<clTaskState> deadlines_states_on_date;
        pFileReadWrite->read_rem_deadline_states(id,
                                 active_deadlines, active_deadlines_states,
                                 date, deadlines_w_date_shifted, deadlines_shifted_dates,
                                 date, deadlines_w_record_on_date, deadlines_states_on_date);

        //
        for(int i=0; i<active_deadlines.size(); i++)
            tasks_active_on_date << clTask(id, active_deadlines.at(i));
        active_tasks_states << active_deadlines_states;

        //
        if(on_date_only)
        {
            for(int i=0; i<deadlines_w_date_shifted.size(); i++)
            {
                if(deadlines_shifted_dates.at(i) == date)
                {
                    tasks_w_date_shifted << clTask(id, deadlines_w_date_shifted.at(i));
                    shifted_dates << date;
                }
            }

        }
        else
        {
            for(int i=0; i<deadlines_w_date_shifted.size(); i++)
                tasks_w_date_shifted << clTask(id, deadlines_w_date_shifted.at(i));
            shifted_dates << deadlines_shifted_dates;
        }

        //
        for(int i=0; i<deadlines_w_record_on_date.size(); i++)
            tasks_w_state_update_on_date << clTask(id, deadlines_w_record_on_date.at(i));
        updated_state_on_date << deadlines_states_on_date;
    }
}

void clTaskStatesManager::task_state_changed(const clTask &task, const clTaskState &new_state)
{
    const QDate today = QDate::currentDate();

    // update `mOverdueTasks[]`
    if(mOverdueTasks.contains(task))
    {
        if(new_state.get_state() != clTaskState::Todo)
            mOverdueTasks.removeOne(task);
    }
    else
    {
        if(task.mDeadline < today
           && new_state.get_state() == clTaskState::Todo)
            mOverdueTasks << task;
    }

    pFileReadWrite->save_overdue_tasks(mOverdueTasks, mLastTracedDay); //save to file

    // update task state history
    pFileReadWrite->add_task_state_update(task, new_state, today);
}

//void clTaskStatesManager::reminder_date_setting_updated(const int &rem_id)
//{
// //..........
//}
