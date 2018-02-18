#ifndef UI_DAY_PLAN_H
#define UI_DAY_PLAN_H

#include <QWidget>
#include <QDate>
#include <QMenu>
#include <QTableWidget>
#include "Reminder.h"
#include "DataElem_TaskState.h"
#include "TaskStatesManager.h"
#include "DataElem_TaskDayScheduleSession.h"
#include "UI_DayPlan_TaskTable.h"
#include "UI_DayPlan_ScheduleTable.h"
#include "UI_DayPlan_UpdateRecordTable.h"
typedef clDataElem_DayScheduleItem clDayScheduleItem;
typedef clDataElem_TaskDayScheduleSession clTaskDayScheduleSession;

namespace Ui {
class clUI_day_plan;
}

class clUI_DayPlan : public QWidget
{
    Q_OBJECT

public:
    explicit clUI_DayPlan(const QMap<int, const clReminder*> *p_reminders,
                          const clTaskStatesManager *p_TaskStatesManager,
                          QWidget *parent = 0);
    ~clUI_DayPlan();

    void start_up();

public slots:
    void reminder_updated(int id);

signals:
    void to_update_task_state(const clUtil_Task &task, const clDataElem_TaskState &new_state);
    void to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void to_show_reminder_in_tab_all_reminders(int id);

//    void get_scheduled_sessions(const QDate &date,
//                                QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);
//    void scheduled_sessions_updated(
//                           const QDate& date,
//                           const QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);

//------------------

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void set_date(const QDate &date); //update view according to `pTaskStatesManager`

    void on_pushButton_today_clicked();
    void on_checkBox_show_update_record_toggled(bool checked);
    void on_comboBox_nontask_option_currentIndexChanged(int index);
    void on_pushButton_add_temp_rem_clicked();

//    void on_tableWidget_day_plan_customContextMenuRequested(const QPoint &pos);
//    void on_tableWidget_update_record_customContextMenuRequested(const QPoint &pos);

//    void on_tableWidget_day_plan_currentCellChanged(int currentRow, int currentColumn,
//                                                    int previousRow, int previousColumn);
//    void on_tableWidget_update_record_currentCellChanged(int currentRow, int currentColumn,
//                                                        int previousRow, int previousColumn);

private:
    Ui::clUI_day_plan *ui;

    // data (read only) //
    const QMap<int, const clReminder*> *pReminders;

    // other units //
    const clTaskStatesManager *pTaskStatesManager;

    // managers for tables //
    clUI_DayPlan_TaskTable *mTaskTableManager;              //for `ui->tableWidget_day_plan`
    clUI_DayPlan_UpdateRecordTable *mUpdateRecTableManager; //for `ui->tableWidget_update_record`
    clUI_DayPlan_ScheduleTable *mScheduleTableManager;      //for `ui->tableWidget_schedule`

    // internal data //
    clTask mCurrentTask;

    //
    void prepare_tasks_data_for_date(const QDate &date,
                                     QMap<clTask, clTaskState> &tasks,
                                     QMap<clTask, clTaskState> &other_update_record);
    void prepare_scheduled_session_data_for_date(const QDate &date,
                                                 QList<clDayScheduleItem> &sessions);
                                                 //get scheduled task sessions and temporary
                                                 //sessions on `date`
    void get_reminders_w_trigger_or_binding_for_date(const QDate &date,
                                                     QList<clDayScheduleItem> &items);
                                                     //find reminders with triggering times or
                                                     //binding time ranges on `date`
    void show_reminder_detail_and_quick_note(const int id);
};

#endif // UI_DAY_PLAN_H
