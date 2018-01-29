#include <QDebug>
#include "UI_DayPlan_Dialog_Schedule.h"
#include "ui_UI_DayPlan_Dialog_Schedule.h"
#include "utility_widgets.h"

clUI_DayPlan_ScheduleDialog::clUI_DayPlan_ScheduleDialog(
                             const QList<std::pair<clUtil_HrMinRange, bool> > &hrmin_ranges,
                             QWidget *parent)
    : QDialog(parent), ui(new Ui::clUI_DayPlan_ScheduleDialog)
{
    ui->setupUi(this);

    //
    mHrMinRanges = hrmin_ranges;
    update_table();

    //
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << " " << "start" << "end");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //
    ui->label_error_message->clear();

    //
    ui->pushButton_remove->setEnabled(false);

    //
    ui->spinBox_start_hr->setValue(13);
    ui->spinBox_start_min->setValue(30);
    ui->spinBox_end_hr->setValue(14);
    ui->spinBox_end_min->setValue(30);
}

clUI_DayPlan_ScheduleDialog::~clUI_DayPlan_ScheduleDialog()
{
    delete ui;
}

void clUI_DayPlan_ScheduleDialog::enable_editing_frame(const bool enable)
{
    ui->checkBox_try->setEnabled(enable);
    ui->spinBox_start_hr->setEnabled(enable);
    ui->spinBox_start_min->setEnabled(enable);
    ui->spinBox_end_hr->setEnabled(enable);
    ui->spinBox_end_min->setEnabled(enable);
    ui->pushButton_add->setEnabled(enable);
}

static void print_hrmin_range(const clUtil_HrMinRange &hrmin_rng, QString &start, QString &end)
{
    Q_ASSERT(! hrmin_rng.is_empty());
    start = QString("%1:%2")
            .arg(hrmin_rng.get_start_hr(), 2,10,QChar('0'))
            .arg(hrmin_rng.get_start_min(),2,10,QChar('0'));
    end = QString("%1:%2")
          .arg(hrmin_rng.get_end_hr(), 2,10,QChar('0'))
          .arg(hrmin_rng.get_end_min(),2,10,QChar('0'));
}

void clUI_DayPlan_ScheduleDialog::update_table()
//update table according to `mHrMinRanges[]`
{
    QStringList col_try, col_start, col_end;
    for(int i=0; i<mHrMinRanges.size(); i++)
    {
        clUtil_HrMinRange range = std::get<0>(mHrMinRanges.at(i));
        bool b_try = std::get<1>(mHrMinRanges.at(i));

        col_try << (b_try ? "try" : "");

        QString str_start, str_end;
        print_hrmin_range(range, str_start, str_end);
        col_start << str_start;
        col_end << str_end;
    }

    fill_table_widget(*ui->tableWidget,
                      QList<QStringList>() << col_try << col_start << col_end,
                      QStringList() << "" << "start" << "end" );
}

void clUI_DayPlan_ScheduleDialog::on_tableWidget_currentCellChanged(
                         int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(currentColumn);

    ui->pushButton_remove->setEnabled(currentRow >= 0);
}

void clUI_DayPlan_ScheduleDialog::on_pushButton_add_clicked()
{
    clUtil_HrMinRange range(ui->spinBox_start_hr->value(), ui->spinBox_start_min->value(),
                            ui->spinBox_end_hr->value(), ui->spinBox_end_min->value());

    int start_min_number = ui->spinBox_start_hr->value()*60 + ui->spinBox_start_min->value();

    int insert_at = mHrMinRanges.size();
    for(int i=0; i<mHrMinRanges.size(); i++)
    {
        clUtil_HrMinRange range1 = std::get<0>(mHrMinRanges.at(i));
        int start_min_number1 = range1.get_start_hr()*60 + range1.get_start_min();
        if(start_min_number1 > start_min_number)
        {
            insert_at = i;
            break;
        }
    }

    mHrMinRanges.insert(insert_at, std::make_pair(range, ui->checkBox_try->isChecked()));
    update_table();
    ui->pushButton_add->setEnabled(false);
}

void clUI_DayPlan_ScheduleDialog::on_pushButton_remove_clicked()
{
    int row = ui->tableWidget->currentRow();
    Q_ASSERT(row >= 0);

    mHrMinRanges.removeAt(row);
    update_table();
}

void clUI_DayPlan_ScheduleDialog::on_spinBox_start_hr_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    check_input();
}

void clUI_DayPlan_ScheduleDialog::on_spinBox_start_min_valueChanged(int arg1)
{
    if(arg1 == -1)
    {
        ui->spinBox_start_min->setValue(59);
        if(ui->spinBox_start_hr->value() > 0)
            ui->spinBox_start_hr->stepDown();
    }
    else if(arg1 == 60)
    {
        ui->spinBox_start_min->setValue(0);
        if(ui->spinBox_start_hr->value() < 23)
            ui->spinBox_start_hr->stepUp();
    }
    check_input();
}

void clUI_DayPlan_ScheduleDialog::on_spinBox_end_hr_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    check_input();
}

void clUI_DayPlan_ScheduleDialog::on_spinBox_end_min_valueChanged(int arg1)
{
    if(arg1 == -1)
    {
        ui->spinBox_end_min->setValue(59);
        if(ui->spinBox_end_hr->value() > 0)
            ui->spinBox_end_hr->stepDown();
    }
    else if(arg1 == 60)
    {
        ui->spinBox_end_min->setValue(0);
        ui->spinBox_end_hr->stepUp();
    }
    check_input();
}

void clUI_DayPlan_ScheduleDialog::check_input()
{
    const int hr0  = ui->spinBox_start_hr->value();
    const int min0 = ui->spinBox_start_min->value();
    const int hr1  = ui->spinBox_end_hr->value();
    const int min1 = ui->spinBox_end_min->value();

    if((hr1-hr0)*60+(min1-min0) <= 0)
    {
        ui->label_error_message->setText("End time must be later than start time.");
        ui->pushButton_add->setEnabled(false);
        return;
    }

    //
    clUtil_HrMinRange range(hr0, min0, hr1, min1);
    bool overlap = false;
    for(int i=0; i<mHrMinRanges.size(); i++)
    {
        clUtil_HrMinRange range1 = std::get<0>(mHrMinRanges.at(i));
        if(range.overlaps_or_is_adjacent_to(range1))
        {
            overlap = true;
            break;
        }
    }

    if(overlap)
    {
        ui->label_error_message->setText("The time range overlaps an existing time range.");
        ui->pushButton_add->setEnabled(false);
        return;
    }

    //
    ui->label_error_message->setText("");
    ui->pushButton_add->setEnabled(true);
}
