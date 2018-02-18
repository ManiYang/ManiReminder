#ifndef UI_DAYPLAN_DIALOG_SCHEDULE_ADD_H
#define UI_DAYPLAN_DIALOG_SCHEDULE_ADD_H

#include <QDialog>
#include "Util_HrMinRange.h"

namespace Ui {
class clUI_DayPlan_Dialog_Schedule_Edit;
}

class clUI_DayPlan_Dialog_Schedule_Edit : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_DayPlan_Dialog_Schedule_Edit(const QString &prefix,
                                              const clUtil_HrMinRange &hrmin_range,
                                              QWidget *parent = 0);
    ~clUI_DayPlan_Dialog_Schedule_Edit();

    QString get_prefix() const;
    clUtil_HrMinRange get_hrmin_range() const;

public slots:
    void accept();

private slots:
    void on_spinBox_min0_valueChanged(int arg1);
    void on_spinBox_min1_valueChanged(int arg1);
    void on_spinBox_hr0_valueChanged(int arg1);
    void on_spinBox_hr1_valueChanged(int arg1);

private:
    Ui::clUI_DayPlan_Dialog_Schedule_Edit *ui;

    void check_input();
};

#endif // UI_DAYPLAN_DIALOG_SCHEDULE_ADD_H
