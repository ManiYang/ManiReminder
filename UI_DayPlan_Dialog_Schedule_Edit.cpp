#include "UI_DayPlan_Dialog_Schedule_Edit.h"
#include "ui_UI_DayPlan_Dialog_Schedule_Edit.h"

clUI_DayPlan_Dialog_Schedule_Edit::clUI_DayPlan_Dialog_Schedule_Edit(
                                                         const QString &prefix,
                                                         const clUtil_HrMinRange &hrmin_range,
                                                         QWidget *parent)
    : QDialog(parent), ui(new Ui::clUI_DayPlan_Dialog_Schedule_Edit)
{
    ui->setupUi(this);

    //
    setWindowTitle("Edit Session");

    ui->comboBox_prefix->setCurrentText(prefix);

    Q_ASSERT(! hrmin_range.is_empty());
    ui->spinBox_hr0->setValue(hrmin_range.get_start_hr());
    ui->spinBox_min0->setValue(hrmin_range.get_start_min());
    ui->spinBox_hr1->setValue(hrmin_range.get_end_hr());
    ui->spinBox_min1->setValue(hrmin_range.get_end_min());

    ui->label_msg->setText("");
}

clUI_DayPlan_Dialog_Schedule_Edit::~clUI_DayPlan_Dialog_Schedule_Edit()
{
    delete ui;
}

void clUI_DayPlan_Dialog_Schedule_Edit::on_spinBox_min0_valueChanged(int arg1)
{
    if(arg1 == -1)
    {
        if(ui->spinBox_hr0->value() > 0)
        {
            ui->spinBox_min0->setValue(59);
            ui->spinBox_hr0->stepDown();
        }
        else
            ui->spinBox_min0->setValue(0);
    }
    else if(arg1 == 60)
    {
        if(ui->spinBox_hr0->value() < 23)
        {
            ui->spinBox_min0->setValue(0);
            ui->spinBox_hr0->stepUp();
        }
        else
            ui->spinBox_min0->setValue(59);
    }

    check_input();
}

void clUI_DayPlan_Dialog_Schedule_Edit::on_spinBox_min1_valueChanged(int arg1)
{
    if(arg1 == -1)
    {
        if(ui->spinBox_hr1->value() > 0)
        {
            ui->spinBox_min1->setValue(59);
            ui->spinBox_hr1->stepDown();
        }
        else
            ui->spinBox_min1->setValue(0);
    }
    else if(arg1 == 60)
    {
        ui->spinBox_min1->setValue(0);
        ui->spinBox_hr1->stepUp();
    }

    check_input();
}

void clUI_DayPlan_Dialog_Schedule_Edit::on_spinBox_hr0_valueChanged(int arg1)
{
    if(ui->spinBox_hr1->value() < arg1)
        ui->spinBox_hr1->setValue(arg1);

    check_input();
}

void clUI_DayPlan_Dialog_Schedule_Edit::on_spinBox_hr1_valueChanged(int arg1)
{
    if(ui->spinBox_hr0->value() > arg1)
        ui->spinBox_hr0->setValue(arg1);

    check_input();
}

clUtil_HrMinRange clUI_DayPlan_Dialog_Schedule_Edit::get_hrmin_range() const
{
    return clUtil_HrMinRange(ui->spinBox_hr0->value(), ui->spinBox_min0->value(),
                             ui->spinBox_hr1->value(), ui->spinBox_min1->value());
}

void clUI_DayPlan_Dialog_Schedule_Edit::accept()
{
    if(    ui->spinBox_hr1->value() > ui->spinBox_hr0->value()
        || (    ui->spinBox_hr1->value() == ui->spinBox_hr0->value()
             && ui->spinBox_min1->value() > ui->spinBox_min0->value()) )
    {
        QDialog::accept();
    }
}

QString clUI_DayPlan_Dialog_Schedule_Edit::get_prefix() const
{
    return ui->comboBox_prefix->currentText();
}

void clUI_DayPlan_Dialog_Schedule_Edit::check_input()
{
    if(    ui->spinBox_hr1->value() < ui->spinBox_hr0->value()
        || (    ui->spinBox_hr1->value() == ui->spinBox_hr0->value()
             && ui->spinBox_min1->value() <= ui->spinBox_min0->value()) )
    {
        ui->label_msg->setText("End time must be later than start time.");
    }
    else
        ui->label_msg->setText("");
}
