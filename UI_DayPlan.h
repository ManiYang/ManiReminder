#ifndef UI_DAY_PLAN_H
#define UI_DAY_PLAN_H

#include <QWidget>
#include <QDate>
#include <QMenu>
#include "Reminder.h"
#include "DataElem_ScheduleStatus.h"
#include "FileReadWrite.h"

namespace Ui {
class clUI_day_plan;
}

class clUI_DayPlan : public QWidget
{
    Q_OBJECT

public:
    explicit clUI_DayPlan(const QMap<int, const clReminder*> *p_reminders,
                          QWidget *parent = 0);
    ~clUI_DayPlan();

    void start_up();

public slots:
    void reminder_updated(int id);

signals:
    void to_get_day_planning_status(const QDate &date,
                                    QMap<int,clDataElem_ScheduleStatus> *status);
    void day_planning_status_modified(const QDate &date,
                                      const QMap<int,clDataElem_ScheduleStatus> &status);

//------------------

private slots:
    void set_date(const QDate &date);

    void on_pushButton_today_clicked();
    void on_tableWidget_reminder_titles_currentCellChanged(int currentRow, int currentColumn,
                                                           int previousRow, int previousColumn);
    void on_tableWidget_reminder_titles_customContextMenuRequested(const QPoint &pos);

private:
    Ui::clUI_day_plan *ui;

    // data (read only) //
    const QMap<int, const clReminder*> *pReminders;

    //
    QMap<int,clDataElem_ScheduleStatus> mDayPlanningStatus; //[rem_id], for the selected date
    QList<int> mInvolvedReminderIDs; //reminders that have date-settings including
                                     //current selected date
    QList<int> mDayCountsToDue; //[i] is for `mInvolvedReminderIDs[i]`

    //
    QMenu *mContextMenu;
    QAction *mAction_Schedule;
    QAction *mAction_Postpone;
    QAction *mAction_Skip;
    QAction *mAction_Cancel;

    //
    void build_table(); //Build table according to `mDayPlanningStatus`,
                        //'mInvolvedReminderIDs', and 'mDayCountsToDue'.
};

#endif // UI_DAY_PLAN_H
