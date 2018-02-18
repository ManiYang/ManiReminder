#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMainWindow>
#include "Reminder.h"
#include "UI_AllReminders.h"
#include "UI_Board.h"
#include "UI_DayPlan.h"
#include "TaskStatesManager.h"

namespace Ui {
class clUI_MainWindow;
}

class clUI_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit clUI_MainWindow(const QMap<int, const clReminder*> *p_reminders,
                             const QSet<QString> *p_situations, const QSet<QString> *p_events,
                             const clTaskStatesManager *p_TaskStatesManager,
                             QWidget *parent = nullptr);
    ~clUI_MainWindow();

    void start_up();

public slots:
    void reminder_created(int id); //Reminder `id` must be empty, having only title.
    void reminder_updated(int id);
    void situation_event_list_updated();

    void reminder_becomes_active(int id);
    void reminder_becomes_inactive(int id);

    void current_date_changed();

signals:
    void gevent_happened(const clDataElem_GEvent &gevent, const QDateTime &at);

    void to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void to_modify_reminder_spec(int id, const clReminderSpec *new_spec);
    void to_add_reminder_record(int id, const QDateTime &t, const QString &log_text);
    void to_create_new_reminder(const QString &title);

    void to_update_task_state(const clUtil_Task &task, const clDataElem_TaskState &new_state);

    void get_scheduled_sessions(const QDate &date,
                                QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);
    void scheduled_sessions_updated(
                           const QDate& date,
                           const QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);

//    void to_get_day_planning_status(const QDate &date,
//                                    QMap<int,clDataElem_TaskState> *status);
//    void day_planning_status_modified(const QDate &date,
//                                      const QMap<int,clDataElem_TaskState> &status);

//----------------------

private slots:
    void on_actionQuit_triggered();
    void on_actionNew_Reminder_triggered();
    void on_actionScheduled_Actions_triggered();

    void to_show_reminder_in_tab_all_reminders(int id);

private:
    Ui::clUI_MainWindow *ui;

    // data access (read only) //
    const QMap<int, const clReminder*> *pReminders;
    const QSet<QString> *pSituations;
    const QSet<QString> *pEvents;

    // other units //
    const clTaskStatesManager *pTaskStatesManager;

    // child UI components //
    clUI_Board *mUI_Board;
    clUI_DayPlan *mUI_DayPlan;
    clUI_AllReminders *mUI_AllReminders;
};

#endif // UI_MAINWINDOW_H
