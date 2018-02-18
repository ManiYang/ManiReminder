#include <QMessageBox>
#include <QDebug>
#include <QCalendarWidget>
#include "UI_DayPlan.h"
#include "ui_UI_DayPlan.h"
#include "utility_widgets.h"
#include "UI_DayPlan_Dialog_Schedule.h"
#include "UI_Dialog_CalendarDatePicker.h"

clUI_DayPlan::clUI_DayPlan(const QMap<int, const clReminder*> *p_reminders,
                           const clTaskStatesManager *p_TaskStatesManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::clUI_day_plan),
      pReminders(p_reminders), pTaskStatesManager(p_TaskStatesManager)
{
    ui->setupUi(this);

    //
    ui->dateEdit->setDisplayFormat("yyyy MM/dd ddd");
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit->setDate(QDate(2000,1,1));

    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(set_date(QDate)));

    // managers for tables
    mScheduleTableManager = new clUI_DayPlan_ScheduleTable(ui->tableWidget_schedule, this);
    mUpdateRecTableManager = new clUI_DayPlan_UpdateRecordTable(ui->tableWidget_update_record, this);
    mTaskTableManager = new clUI_DayPlan_TaskTable(ui->tableWidget_day_plan, this);

    //
    ui->splitter->setStretchFactor(0,1);
    ui->splitter->setStretchFactor(1,2);

    ui->checkBox_show_update_record->setChecked(false);
    mUpdateRecTableManager->set_table_visible(false);

    //
    ui->plainTextEdit_quick_note->installEventFilter(this);
}

clUI_DayPlan::~clUI_DayPlan()
{
    delete ui;
}

void clUI_DayPlan::start_up()
{
    QDate today = QDate::currentDate();
    ui->dateEdit->setDate(today);
}

void clUI_DayPlan::reminder_updated(int id)
{
    Q_UNUSED(id);

    QDate date = ui->dateEdit->date();
    set_date(date);
}


void clUI_DayPlan::prepare_tasks_data_for_date(const QDate &date,
                                               QMap<clTask, clTaskState> &tasks,
                                               QMap<clTask, clTaskState> &other_update_record)
{
    const QDate today = QDate::currentDate();

    tasks.clear();
    other_update_record.clear();

    // get data for `date` from `pTaskStatesManager`
    QList<clTask> active_tasks; //tasks active on `date`
    QList<clTaskState> active_tasks_states; //[i]: task states for `mActiveTasks[i]`

    QList<clTask> shifted_tasks; //tasks with date shifted to <= `date` (if `date` is today)
                                 //or to = `date` (`date` is not today)
    QList<QDate> shifted_tasks_shifted2dates; //[i]: for `mShiftedTasks[i]`

    QList<clTask> updated_tasks; //tasks with a record of state update on `date`
    QList<clTaskState> updated_tasks_states; //[i]: for `mUpdatedTasks[i]`

    pTaskStatesManager->date_query(date,
                                   active_tasks, active_tasks_states,
                                   (date!=today), shifted_tasks, shifted_tasks_shifted2dates,
                                   updated_tasks, updated_tasks_states);

    // add active tasks to `tasks[]`
    for(int i=0; i<active_tasks.size(); i++)
        tasks.insert(active_tasks.at(i), active_tasks_states.at(i));

    // add overdue tasks to `tasks[]` if `date` is today
    // (an overdue task is a task with state "Todo" and deadline < today)
    if(date == today)
    {
        QList<clTask> overdue_tasks;
        overdue_tasks = pTaskStatesManager->get_overdue_tasks();

        for(int i=0; i<overdue_tasks.size(); i++)
            tasks.insert(overdue_tasks.at(i), clTaskState()); //todo
    }

    // add to `tasks[]` tasks with date shifted to d1 <= `date` [to d1 = `date`] if `date` is
    // today [if `date` > today].
    if(date >= today)
    {
        clTaskState state;
        for(int i=0; i<shifted_tasks.size(); i++)
        {
            state.set_state(clTaskState::DateShifted, shifted_tasks_shifted2dates.at(i));
            tasks.insert(shifted_tasks.at(i), state);
        }
    }

    // if `date` is <= today, add to `other_update_record[]` tasks with a record of state
    // update on `date` that is not included in `tasks[]`
    if(date <= today)
    {
        for(int i=0; i<updated_tasks.size(); i++)
        {
            if(! tasks.contains(updated_tasks.at(i)))
                other_update_record.insert(updated_tasks.at(i), updated_tasks_states.at(i));
        }
    }
}

void clUI_DayPlan::prepare_scheduled_session_data_for_date(const QDate &date,
                                                           QList<clDayScheduleItem> &sessions)
//get scheduled task sessions and temporary sessions on `date`
{
    // ......

}

void clUI_DayPlan::get_reminders_w_trigger_or_binding_for_date(const QDate &date,
                                                               QList<clDayScheduleItem> &items)
//find reminders with triggering times or binding time ranges on `date`
{
    items.clear();

    for(auto it=pReminders->cbegin(); it!=pReminders->cend(); it++)
    {
        int id = it.key();
        const clReminder *reminder = it.value();

        QList<QTime> triggering_times = reminder->get_triggering_times_on_date(date);
        foreach(QTime t, triggering_times)
        {
            clDayScheduleItem item;
            item.set_nontask_w_time_trigger(id, reminder->get_title(), "", t);
            items << item;
        }

        QList<clUtil_HrMinRange> binding_time_ranges
                = reminder->get_binding_hrmin_ranges_on_date(date);
        foreach(clUtil_HrMinRange t_rng, binding_time_ranges)
        {
            clDayScheduleItem item;
            item.set_nontask_w_time_range_binding(id, reminder->get_title(), "", t_rng);
            items << item;
        }
    }
}

void clUI_DayPlan::set_date(const QDate &date)
//update view according to `pTaskStatesManager`
{
    Q_ASSERT(date.isValid());

    const QDate today = QDate::currentDate();

    // prepare data
    QMap<clTask, clTaskState> tasks;
    QMap<clTask, clTaskState> other_updated_tasks;
    prepare_tasks_data_for_date(date, tasks, other_updated_tasks);

    QList<clDayScheduleItem> scheduled_sessions; //task and temp. sessions
    prepare_scheduled_session_data_for_date(date, scheduled_sessions);

    // find scheduled tasks from `scheduled_sessions[]` --> `scheduled_tasks`
    QMap<clTask,bool> scheduled_tasks; //the key is not used
    for(int i=0; i<scheduled_sessions.size(); i++)
    {
        if(scheduled_sessions.at(i).is_task())
            scheduled_tasks.insert(scheduled_sessions.at(i).get_task(), false);
    }

    // build table 1 -- tasks
    mTaskTableManager->clear();
    mTaskTableManager->set_relative_to_date(date);

    for(auto it=tasks.begin(); it!=tasks.end(); it++)
    {
        const clTask &task = it.key();
        const clTaskState &state = it.value();

        const int id = task.mRemID;
        Q_ASSERT(pReminders->contains(id));

        bool scheduled = scheduled_tasks.contains(task);
        mTaskTableManager->add_task(task, (*pReminders)[id]->get_title(),
                                    (date==today), state, scheduled);
    }

    // build table 2 -- state update records
    mUpdateRecTableManager->clear();

    for(auto it=other_updated_tasks.begin(); it!=other_updated_tasks.end(); it++)
    {
        const clTask &task = it.key();
        const clTaskState &state = it.value();

        const int id = task.mRemID;
        Q_ASSERT(pReminders->contains(id));

        mUpdateRecTableManager->add_task(task, (*pReminders)[id]->get_title(), state);
    }

    // find reminders with triggering times or binding time ranges on `date`
    QList<clDayScheduleItem> reminders_w_trigger_or_binding;
    get_reminders_w_trigger_or_binding_for_date(date, reminders_w_trigger_or_binding);

    // build table 3 -- schedule
    QList<clDayScheduleItem> day_schedule_items;
    day_schedule_items << scheduled_sessions;
    day_schedule_items << reminders_w_trigger_or_binding;

    mScheduleTableManager->clear();
    mScheduleTableManager->add_schedule_items(day_schedule_items);

    //
    mTaskTableManager->deselect();
    mUpdateRecTableManager->deselect();
    mScheduleTableManager->deselect();

    //
    ui->plainTextEdit_detail->clear();
    ui->plainTextEdit_quick_note->clear();
}

void clUI_DayPlan::on_pushButton_today_clicked()
{
    QDate today = QDate::currentDate();
    ui->dateEdit->setDate(today);
}

void clUI_DayPlan::on_checkBox_show_update_record_toggled(bool checked)
{
    mUpdateRecTableManager->set_table_visible(checked);
}
/*
void clUI_DayPlan::on_tableWidget_day_plan_currentCellChanged(
                       int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if(currentRow < 0 || currentColumn < 0)
        return;

    // de-select the other tables
    mUpdateRecTableManager->deselect();
    mScheduleTableManager->deselect();

    // get selected task --> `mCurrentTask`
    mCurrentTask = table_row_to_task(ui->tableWidget_day_plan, currentRow);

    //
    highlight_current_task_in_tables();

    //
    show_reminder_detail_and_quick_note(mCurrentTask.mRemID);
}*/

/*
void clUI_DayPlan::on_tableWidget_update_record_currentCellChanged(
                       int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if(currentRow < 0 || currentColumn < 0)
        return;

    // de-select the other tables
    ui->tableWidget_day_plan->setCurrentCell(-1, -1);
    mScheduleTableManager->deselect();

    // get selected task --> `mCurrentTask`
    mCurrentTask = table_row_to_task(ui->tableWidget_update_record, currentRow);

    //
    highlight_current_task_in_tables();

    //
    show_reminder_detail_and_quick_note(mCurrentTask.mRemID);
}
*/

void clUI_DayPlan::show_reminder_detail_and_quick_note(const int id)
{
    Q_ASSERT(pReminders->contains(id));
    const clReminder *reminder = (*pReminders)[id];

    ui->plainTextEdit_detail->clear();
    ui->plainTextEdit_detail->appendHtml(reminder->get_detail());

    ui->plainTextEdit_quick_note->setPlainText(reminder->get_quick_note());
}

bool clUI_DayPlan::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->plainTextEdit_quick_note)
    {
        if(event->type() == QEvent::FocusOut)
        {
            int id = mCurrentTask.mRemID;
            if(id >= 0)
            {
                if(ui->plainTextEdit_quick_note->document()->isModified())
                {
                    QString new_quick_note = ui->plainTextEdit_quick_note->toPlainText();
                    clReminder new_data(-1);
                    new_data.set_quick_note(new_quick_note);
                    emit to_modify_reminder(id, clReminder::QuickNote, &new_data);

                    ui->plainTextEdit_quick_note->document()->setModified(false);
                }
            }
        }
    }

    //
    return QWidget::eventFilter(watched, event);
}

void clUI_DayPlan::on_comboBox_nontask_option_currentIndexChanged(int index)
{
    if(index == 0)
        mScheduleTableManager->show_nontask_reminders(true, true);
    else if(index == 1)
        mScheduleTableManager->show_nontask_reminders(true, false);
    else if(index == 2)
        mScheduleTableManager->show_nontask_reminders(false, true);
    else
        mScheduleTableManager->show_nontask_reminders(false, false);
}

void clUI_DayPlan::on_pushButton_add_temp_rem_clicked()
{
    clUI_DayPlan_ScheduleDialog dialog;
    int r = dialog.exec();
    if(r != QDialog::Accepted)
        return;

    QList<clDataElem_DayScheduleItem> new_sessions = dialog.get_sessions();
    if(! new_sessions.isEmpty())
    {
        mScheduleTableManager->add_schedule_items(new_sessions);
        mScheduleTableManager->highlight_temporary_reminder(new_sessions.at(0).get_title());
    }
}
