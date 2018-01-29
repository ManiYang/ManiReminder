#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMainWindow>
#include "Reminder.h"
#include "UI_AllReminders.h"
#include "UI_Board.h"
#include "UI_DayPlan.h"

namespace Ui {
class clUI_MainWindow;
}

class clUI_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit clUI_MainWindow(const QMap<int, const clReminder*> *p_reminders,
                             const QSet<QString> *p_situations, const QSet<QString> *p_events,
                             QWidget *parent = nullptr);
    ~clUI_MainWindow();

    void start_up();

public slots:
    void reminder_created(int id); //Reminder `id` must be empty, having only title.
    void reminder_updated(int id);
    void situation_event_list_updated();

    void reminder_becomes_active(int id);
    void reminder_becomes_inactive(int id);

signals:
    void gevent_happened(const clDataElem_GEvent &gevent, const QDateTime &at);

    void to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void to_modify_reminder_spec(int id, const clReminderSpec *new_spec);
    void to_add_reminder_record(int id, const QDateTime &t, const QString &log_text);
    void to_create_new_reminder(const QString &title);

    void to_get_day_planning_status(const QDate &date,
                                    QMap<int,clDataElem_ScheduleStatus> *status);
    void day_planning_status_modified(const QDate &date,
                                      const QMap<int,clDataElem_ScheduleStatus> &status);

//----------------------

private slots:
    void on_actionQuit_triggered();
    void on_actionNew_Reminder_triggered();
    void on_actionScheduled_Actions_triggered();

private:
    Ui::clUI_MainWindow *ui;

    // data access (read only) //
    const QMap<int, const clReminder*> *pReminders;
    const QSet<QString> *pSituations;
    const QSet<QString> *pEvents;

    // child UI components //
    clUI_Board *mUI_Board;
    clUI_DayPlan *mUI_DayPlan;
    clUI_AllReminders *mUI_AllReminders;
};

#endif // UI_MAINWINDOW_H