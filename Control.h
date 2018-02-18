#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "UI_MainWindow.h"
#include "FileReadWrite.h"
#include "AlarmClockService.h"
#include "TaskStatesManager.h"

class clControl : public QObject
{
    Q_OBJECT
public:
    explicit clControl(QObject *parent = nullptr);
    ~clControl();

    void start_up();

private slots:
    void AlarmClockService_time_reached(const QDateTime &t, int rem_id);
    void reminder_becomes_active(int id);
    void reminder_becomes_inactive(int id);

    void MainWindow_gevent_happened(const clDataElem_GEvent &gevent, const QDateTime &at);

    void MainWindow_to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void MainWindow_to_modify_reminder_spec(int id, const clReminderSpec *new_spec);
    void MainWindow_to_add_reminder_record(int id, const QDateTime &t, const QString &log_text);
    void MainWindow_to_create_new_reminder(const QString &title);

    void MainWindow_to_update_task_state(const clUtil_Task &task,
                                         const clDataElem_TaskState &new_state);

    void MainWindow_get_scheduled_sessions(
                                  const QDate &date,
                                  QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);
    void MainWindow_scheduled_sessions_updated(
                            const QDate& date,
                            const QMap<clTask, QList<clTaskDayScheduleSession> > *TaskSessions);

    void show_status_bar_message(const QString &msg);

    void On_message_clock_notify(QDateTime t, QString message);
    void On_crossing_day();

private:
    // data //
    QMap<int, clReminder*> mReminders; //mReminders[id]
    QMap<int, const clReminder*> mReminders_ReadOnly; //must be sync'ed to `mReminders`

    QSet<QString> mSituations; //can include unused ones
    QSet<QString> mEvents; //can include unused ones

    QSet<QString> mSelectedSituations;
    QSet<QString> mActiveSituations; //selected and all induced situations (including unused)

    QMultiMap<int,QDate> mOverdueRemDeadlines; //overdue reminders for current day
    QDate mLastTracedDay; //for tracing overdue reminders

    // units //
    clUI_MainWindow *uMainWindow;
    clFileReadWrite uFileReadWrite;
    clAlarmClockService *uAlarmClockService;
    clUtil_MessageClock mMessageClock; //For dectecting day-crossing. Started in `start_up()`.
    clTaskStatesManager *uTaskStatesManager;

    //
    void save_reminder_data(const int id);

    bool is_induced_by_selected_situation(const QString &sit) const;
         //Returns whether `sit` is induced by any currently selected situation.
    QSet<QString> get_induced_names(const QString &name) const;
         //Example: If `name` is "A:B:C", the returned set will be {"A", "A:B", "A:B:C"}.
    void update_active_situations();
         //Set `mActiveSituations` according to `mSelectedSituations`.
};

#endif // CONTROL_H
