#ifndef UI_DAY_PLAN_SCHEDULE_DIALOG_H
#define UI_DAY_PLAN_SCHEDULE_DIALOG_H

#include <QDialog>
#include "Util_HrMinRange.h"

namespace Ui {
class clUI_DayPlan_ScheduleDialog;
}

class clUI_DayPlan_ScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_DayPlan_ScheduleDialog(
                              const QList<std::pair<clUtil_HrMinRange,bool> > &hrmin_ranges,
                              QWidget *parent = 0);
    ~clUI_DayPlan_ScheduleDialog();

    QList< std::pair<clUtil_HrMinRange,bool> > get_hrmin_ranges() const {return mHrMinRanges; }

private slots:
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn,
                                           int previousRow, int previousColumn);
    void on_pushButton_add_clicked();
    void on_pushButton_remove_clicked();

    void on_spinBox_start_hr_valueChanged(int arg1);
    void on_spinBox_start_min_valueChanged(int arg1);
    void on_spinBox_end_hr_valueChanged(int arg1);
    void on_spinBox_end_min_valueChanged(int arg1);

private:
    Ui::clUI_DayPlan_ScheduleDialog *ui;

    QList< std::pair<clUtil_HrMinRange,bool> > mHrMinRanges;

    //
    void update_table(); //update table according to `mHrMinRanges[]`
    void enable_editing_frame(const bool enable);
    void check_input();
};

#endif // UI_DAY_PLAN_SCHEDULE_DIALOG_H
