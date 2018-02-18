#include <QDebug>
#include <QInputDialog>
#include "UI_DayPlan_Dialog_Schedule.h"
#include "ui_UI_DayPlan_Dialog_Schedule.h"
#include "utility_widgets.h"
#include "UI_DayPlan_Dialog_Schedule_Edit.h"

static void prepare_table(QTableWidget *table)
{
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

clUI_DayPlan_ScheduleDialog::clUI_DayPlan_ScheduleDialog(
                                              const clUtil_Task &task, const QString &title,
                                              QWidget *parent)
  : QDialog(parent), ui(new Ui::clUI_DayPlan_ScheduleDialog)
//for adding sessions for a task
{
    ui->setupUi(this);

    //
    Q_ASSERT(task.mRemID >= 0 && task.mDeadline.isValid());
    mTask = task;

    //
    setWindowTitle("Schedule Sessions");

    ui->lineEdit_title->setText(title);
    ui->pushButton_edit_title->setVisible(false);

    prepare_table(ui->tableWidget);

    ui->pushButton_edit->setEnabled(false);
    ui->pushButton_remove->setEnabled(false);
}

clUI_DayPlan_ScheduleDialog::clUI_DayPlan_ScheduleDialog(QWidget *parent)
 : QDialog(parent), ui(new Ui::clUI_DayPlan_ScheduleDialog)
//for adding temporary sessions
{
    ui->setupUi(this);

    //
    mTask.mRemID = -2;

    ui->lineEdit_title->setPlaceholderText("No Title");

    prepare_table(ui->tableWidget);

    ui->pushButton_edit->setEnabled(false);
    ui->pushButton_remove->setEnabled(false);
}

clUI_DayPlan_ScheduleDialog::clUI_DayPlan_ScheduleDialog(
                                              const QList<clDataElem_DayScheduleItem> &sessions,
                                              QWidget *parent)
    : QDialog(parent), ui(new Ui::clUI_DayPlan_ScheduleDialog)
//`sessions` cannot be empty.
//`sessions[i]` must be all for the same task or the same temporary reminder.
{
    ui->setupUi(this);

    //
    Q_ASSERT(! sessions.isEmpty());

    if(sessions.at(0).is_task())
    {
        mTask = sessions.at(0).get_task();
        for(int i=0; i<sessions.size(); i++)
            Q_ASSERT(sessions.at(i).is_task());
    }
    else
    {
        mTask.mRemID = -2;
        for(int i=0; i<sessions.size(); i++)
            Q_ASSERT(sessions.at(i).is_temperary_reminder());
    }

    //
    if(mTask.mRemID == -2)
        ui->lineEdit_title->setPlaceholderText("No Title");
    else
        ui->pushButton_edit_title->setVisible(false);
    ui->lineEdit_title->setText(sessions.at(0).get_title());

    prepare_table(ui->tableWidget);

    //
    mSessions = sessions;
    update_table();
    ui->tableWidget->setCurrentCell(-1, -1);

    //
    ui->pushButton_edit->setEnabled(false);
    ui->pushButton_remove->setEnabled(false);
}

clUI_DayPlan_ScheduleDialog::~clUI_DayPlan_ScheduleDialog()
{
    delete ui;
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
//update table according to `mSessions[]`
{
    ui->tableWidget->setRowCount(0);
    int Nrow = 0;
    for(int i=0; i<mSessions.size(); i++)
    {
        ui->tableWidget->setRowCount(Nrow+1);

        // prefix
        QTableWidgetItem *item = new QTableWidgetItem(mSessions.at(i).get_prefix());
        item->setData(Qt::UserRole, i);
        ui->tableWidget->setItem(Nrow, 0, item);

        // start time & end time
        QString str_start_time, str_end_time;
        clUtil_HrMinRange hrmin_range = mSessions.at(i).get_time_range();
        print_hrmin_range(hrmin_range, str_start_time, str_end_time);

        item = new QTableWidgetItem(str_start_time);
        ui->tableWidget->setItem(Nrow, 1, item);

        item = new QTableWidgetItem(str_end_time);
        ui->tableWidget->setItem(Nrow, 2, item);

        // state
        item = new QTableWidgetItem(mSessions.at(i).get_state());
        ui->tableWidget->setItem(Nrow, 3, item);

        //
        Nrow++;
    }

    //
    ui->tableWidget->sortByColumn(1, Qt::AscendingOrder);
}

/*
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
    for(int i=0; i<mSessions.size(); i++)
    {
        clUtil_HrMinRange range1 = mSessions.at(i).get_hrmin_range();
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
}*/

void clUI_DayPlan_ScheduleDialog::on_pushButton_add_clicked()
{
    clUI_DayPlan_Dialog_Schedule_Edit dialog("", clUtil_HrMinRange(12,0,12,30));
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        QString prefix = dialog.get_prefix();
        clUtil_HrMinRange hrmin_range = dialog.get_hrmin_range();

        clDataElem_DayScheduleItem DSItem;
        if(mTask.mRemID == -2)
            DSItem.set_temporary_reminder(ui->lineEdit_title->text(), prefix, hrmin_range, "");
        else
            DSItem.set_task(mTask, ui->lineEdit_title->text(), prefix, hrmin_range, "");

        mSessions << DSItem;

        //
        update_table();
    }

}

void clUI_DayPlan_ScheduleDialog::on_pushButton_edit_title_clicked()
{
    Q_ASSERT(mTask.mRemID == -2);

    QInputDialog dialog;
    dialog.setLabelText("Enter title:");
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(ui->lineEdit_title->text());
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        ui->lineEdit_title->setText(dialog.textValue());

        for(int i=0; i<mSessions.size(); i++)
            mSessions[i].set_title(dialog.textValue());

        update_table();
    }
}

void clUI_DayPlan_ScheduleDialog::on_pushButton_edit_clicked()
{
    if(ui->tableWidget->currentColumn() < 0)
        return;
    const int row = ui->tableWidget->currentRow();
    if(row < 0)
        return;

    int index;
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        Q_ASSERT(item != nullptr);
        bool ok;
        index = item->data(Qt::UserRole).toInt(&ok);
        Q_ASSERT(ok);
    }
    Q_ASSERT(0 <= index && index < mSessions.size());

    //
    clUI_DayPlan_Dialog_Schedule_Edit dialog(mSessions.at(index).get_prefix(),
                                             mSessions.at(index).get_time_range());
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        QString new_prefix = dialog.get_prefix();
        clUtil_HrMinRange new_hrmin_range = dialog.get_hrmin_range();

        mSessions[index].set_prefix(new_prefix);
        mSessions[index].set_time_range(new_hrmin_range);

        //
        update_table();
    }
}

void clUI_DayPlan_ScheduleDialog::on_pushButton_remove_clicked()
{
    if(ui->tableWidget->currentColumn() < 0)
        return;
    const int row = ui->tableWidget->currentRow();
    if(row < 0)
        return;

    int index;
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        Q_ASSERT(item != nullptr);
        bool ok;
        index = item->data(Qt::UserRole).toInt(&ok);
        Q_ASSERT(ok);
    }
    Q_ASSERT(0 <= index && index < mSessions.size());

    //
    mSessions.removeAt(index);
    update_table();
    ui->tableWidget->setCurrentCell(-1, -1);
}


void clUI_DayPlan_ScheduleDialog::on_tableWidget_currentCellChanged(
                         int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousColumn);
    Q_UNUSED(previousRow);
    ui->pushButton_remove->setEnabled(currentRow >= 0 && currentColumn >= 0);
    ui->pushButton_edit->setEnabled(currentRow >= 0 && currentColumn >= 0);
}
