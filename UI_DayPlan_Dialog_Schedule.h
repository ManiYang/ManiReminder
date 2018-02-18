#ifndef UI_DAY_PLAN_SCHEDULE_DIALOG_H
#define UI_DAY_PLAN_SCHEDULE_DIALOG_H

#include <QDialog>
#include "DataElem_TaskDayScheduleSession.h"

namespace Ui {
class clUI_DayPlan_ScheduleDialog;
}

class clUI_DayPlan_ScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    clUI_DayPlan_ScheduleDialog(const clUtil_Task &task, const QString &title,
                                QWidget *parent = nullptr); //for adding sessions for a task

    explicit clUI_DayPlan_ScheduleDialog(QWidget *parent = nullptr);
                                         //for adding temporary sessions

    explicit clUI_DayPlan_ScheduleDialog(const QList<clDataElem_DayScheduleItem> &sessions,
                                         QWidget *parent = nullptr);
                                         //`sessions` cannot be empty.
                                         //`sessions[i]` must be all for the same task or the
                                         //same temporary reminder.

    ~clUI_DayPlan_ScheduleDialog();

//  QList< std::pair<clUtil_HrMinRange,bool> > get_hrmin_ranges() const { return mHrMinRanges; }
//    QList<clDataElem_TaskDayScheduleSession> get_sessions() const { return mSessions; }
    QList<clDataElem_DayScheduleItem> get_sessions() const { return mSessions; }

private slots:
    void on_pushButton_edit_title_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_edit_clicked();
    void on_pushButton_remove_clicked();
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn,
                                           int previousRow, int previousColumn);

private:
    Ui::clUI_DayPlan_ScheduleDialog *ui;

    QList<clDataElem_DayScheduleItem> mSessions;
    clUtil_Task mTask; //id = -2 => temporary reminder

    //
    void update_table(); //update table according to `mSessions[]`
};

#endif // UI_DAY_PLAN_SCHEDULE_DIALOG_H
