#include <QMessageBox>
#include <QDebug>
#include <QCalendarWidget>
#include "UI_DayPlan.h"
#include "ui_UI_DayPlan.h"
#include "utility_widgets.h"
#include "UI_DayPlan_Dialog_Schedule.h"
#include "UI_Dialog_CalendarDatePicker.h"

clUI_DayPlan::clUI_DayPlan(const QMap<int, const clReminder*> *p_reminders, QWidget *parent)
    : QWidget(parent), ui(new Ui::clUI_day_plan),
      pReminders(p_reminders)
{
    ui->setupUi(this);

    //
    ui->tableWidget_reminder_titles->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tableWidget_reminder_titles->setSelectionBehavior(QTableWidget::SelectRows);
    ui->tableWidget_reminder_titles->setStyleSheet("Font: 11pt");
    ui->tableWidget_reminder_titles->verticalHeader()->setVisible(false);
    ui->tableWidget_reminder_titles->horizontalHeader()
                                          ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_reminder_titles->verticalHeader()
                                          ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_reminder_titles->horizontalHeader()->setHighlightSections(false);
    ui->tableWidget_reminder_titles->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_reminder_titles->setHorizontalHeaderLabels(
                                QStringList() << "task" << "days to\ndeadline" << "scheduling");

    //
    ui->plainTextEdit_reminder_detail->setReadOnly(true);
    ui->plainTextEdit_reminder_detail->setStyleSheet("Font: 10pt");

    //
    ui->dateEdit->setDisplayFormat("yyyy MM/dd ddd");
    ui->dateEdit->setCalendarPopup(true);
    ui->dateEdit->setDate(QDate(2000,1,1));

    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(set_date(QDate)));

    //
    mContextMenu = new QMenu(this);
    mAction_Schedule = mContextMenu->addAction("schedule");
    mContextMenu->addSeparator();
    mAction_Postpone = mContextMenu->addAction("postpone");
    mAction_Skip = mContextMenu->addAction("skip");
    mContextMenu->addSeparator();
    mAction_Cancel = mContextMenu->addAction("cancel/reset");
}

clUI_DayPlan::~clUI_DayPlan()
{
    delete ui;
}

void clUI_DayPlan::start_up()
{
    QDate today = QDate::currentDate();
    ui->dateEdit->setDate(today);
}

void clUI_DayPlan::reminder_updated(int id)
{
    Q_UNUSED(id);

    QDate date = ui->dateEdit->date();
    set_date(date);
}

void clUI_DayPlan::set_date(const QDate &date)
{
    Q_ASSERT(date.isValid());

    // get all reminders that has date-setting including `date`
    // --> `mInvolvedReminderIDs[]`, `mDayCountsToDue[]`
    mInvolvedReminderIDs.clear();
    mDayCountsToDue.clear();

    for(auto it=pReminders->begin(); it!=pReminders->end(); it++)
    {
        const clReminder *reminder = it.value();
        int Ndays_to_due;
        if(reminder->date_setting_includes(date, &Ndays_to_due))
        {
            mInvolvedReminderIDs << reminder->get_id();
            mDayCountsToDue << Ndays_to_due;
        }
    }

    // get planning status --> `mDayPlanningStatus`
    emit to_get_day_planning_status(date, &mDayPlanningStatus);

    foreach(int id, mInvolvedReminderIDs)
    {
        // add the unscheduled ones
        if(! mDayPlanningStatus.contains(id)) //not included => unscheduled
            mDayPlanningStatus.insert(id, clDataElem_RemDayStatus());
    }

    // remove `mDayPlanningStatus[id]` if reminder `id` is not involved (this can happen when
    // the reminder's spec is modified)
    for(auto it=mDayPlanningStatus.begin(); it!=mDayPlanningStatus.end(); )
    {
        if(! mInvolvedReminderIDs.contains(it.key()))
            it = mDayPlanningStatus.erase(it);
        else
            it++;
    }

    //
    build_table();
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

void clUI_DayPlan::build_table()
//Build table according to `mDayPlanningStatus`, 'mInvolvedReminderIDs', and 'mDayCountsToDue'.
{
    // determine contents of columns
    QList<QByteArray> sort_keys;
    QList<int> rem_ids;
    QList<int> due_day_counts;
    QList<QString> str_scheduling_status;
    QList<int> schedule_Nos;

    for(int i=0; i<mInvolvedReminderIDs.size(); i++)
    {
        const int id = mInvolvedReminderIDs.at(i);

        Q_ASSERT(mDayPlanningStatus.contains(id));
        clDataElem_RemDayStatus SS = mDayPlanningStatus[id];
        switch(SS.get_status())
        {
        case clDataElem_RemDayStatus::Unscheduled :
            rem_ids << id;
            due_day_counts << mDayCountsToDue.at(i);
            sort_keys << "a0";
            str_scheduling_status << "unscheduled";
            schedule_Nos << -1;
            break;

        case clDataElem_RemDayStatus::Scheduled :
        {
            QList<std::pair<clUtil_HrMinRange,bool> > times = SS.get_scheduled_times();
            for(int j=0; j<times.size(); j++)
            {
                clUtil_HrMinRange hrmin_rng = std::get<0>(times.at(j));
                bool b_try = std::get<1>(times.at(j));

                QString str_start, str_end;
                print_hrmin_range(hrmin_rng, str_start, str_end);

                rem_ids << id;
                due_day_counts << mDayCountsToDue.at(i);
                sort_keys << ("t"+str_start).toLatin1();
                if(b_try)
                    str_scheduling_status << ("try "+str_start+" - "+str_end);
                else
                    str_scheduling_status << (str_start+" - "+str_end);
                schedule_Nos << j;
            }
            break;
        }

        case clDataElem_RemDayStatus::Postponed :
        {
            QDate d = SS.get_date_postponed_to();
            rem_ids << id;
            due_day_counts << mDayCountsToDue.at(i);
            sort_keys << "z0";
            str_scheduling_status << ("postpone to "+d.toString("yyyy/M/d"));
            schedule_Nos << -1;
            break;
        }

        case clDataElem_RemDayStatus::Skipped :
            rem_ids << id;
            due_day_counts << mDayCountsToDue.at(i);
            sort_keys << "z1";
            str_scheduling_status << "skip";
            schedule_Nos << -1;
            break;
        }
    }

    //
    QList<int> ind;
    {
        QMultiMap<QByteArray, int> map;
        for(int i=0; i<sort_keys.size(); i++)
            map.insert(sort_keys.at(i), i);
        ind = map.values();
    }
    //(now sort_keys[ind[]] is in ascending order)

    // build table
    //   column 0: title
    //   column 1: days to deadline
    //   column 2: scheduling status
    Q_ASSERT(ui->tableWidget_reminder_titles->columnCount() == 3);

    ui->tableWidget_reminder_titles->setRowCount(0);

    int Nrows = 0;
    foreach(int i, ind)
    {
        ui->tableWidget_reminder_titles->setRowCount(Nrows+1);

        //
        const int id = rem_ids.at(i);
        Q_ASSERT(pReminders->contains(id));
        const QString title = (*pReminders)[id]->get_title();

        const int due_day_count = due_day_counts.at(i);
        const QString str_status = str_scheduling_status.at(i);
        const int schedule_No = schedule_Nos.at(i);

        // column 0: title
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, title);
            item->setData(Qt::UserRole, id);
            ui->tableWidget_reminder_titles->setItem(Nrows, 0, item);
        }

        // column 1: days to deadline
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, due_day_count);
            item->setData(Qt::TextAlignmentRole, int(Qt::AlignHCenter | Qt::AlignVCenter));
            if(due_day_count == 0)
                item->setData(Qt::ForegroundRole, QBrush(QColor(Qt::red)));
            else if(due_day_count == 1)
                item->setData(Qt::ForegroundRole, QBrush(QColor("#dd6600")));
            ui->tableWidget_reminder_titles->setItem(Nrows, 1, item);
        }

        // column 2: scheduling status
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, str_status);
            if(str_status == "unscheduled")
                item->setData(Qt::ForegroundRole, QBrush(QColor(Qt::darkBlue)));
            item->setData(Qt::UserRole, schedule_No);
            ui->tableWidget_reminder_titles->setItem(Nrows, 2, item);
        }

        //
        Nrows++;
    }
}

void clUI_DayPlan::on_pushButton_today_clicked()
{
    QDate today = QDate::currentDate();
    ui->dateEdit->setDate(today);
}

void clUI_DayPlan::on_tableWidget_reminder_titles_currentCellChanged(
                        int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousColumn);
    Q_UNUSED(currentColumn);

    if(currentRow == previousRow)
        return;

    if(currentRow < 0)
    {
        ui->plainTextEdit_reminder_detail->clear();
        return;
    }

    QTableWidgetItem *item = ui->tableWidget_reminder_titles->item(currentRow, 0);
    if(item == Q_NULLPTR)
        return;

    bool ok;
    int rem_id = item->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);
    QString detail = (*pReminders)[rem_id]->get_detail();
    ui->plainTextEdit_reminder_detail->setPlainText(detail);
}

void clUI_DayPlan::on_tableWidget_reminder_titles_customContextMenuRequested(const QPoint &pos)
{
    if(ui->tableWidget_reminder_titles->itemAt(pos) == Q_NULLPTR)
        return;

    // get reminder id
    int row = ui->tableWidget_reminder_titles->rowAt(pos.y());
    QTableWidgetItem *item_title = ui->tableWidget_reminder_titles->item(row, 0);
    int rem_id = item_title->data(Qt::UserRole).toInt();

    // get day count before deadline
    QTableWidgetItem *item1 = ui->tableWidget_reminder_titles->item(row, 1);
    int days_to_due = item1->data(Qt::DisplayRole).toString().toInt();

    // get schedule No
    QTableWidgetItem *item2 = ui->tableWidget_reminder_titles->item(row, 2);
    int schedule_No = item2->data(Qt::UserRole).toInt();

    // get scheduling status of the reminder --> `status0`
    Q_ASSERT(mDayPlanningStatus.contains(rem_id));
    clDataElem_RemDayStatus status0 = mDayPlanningStatus[rem_id];

    //
    switch(status0.get_status())
    {
    case clDataElem_RemDayStatus::Unscheduled :
        mAction_Schedule->setText("schedule");
        mAction_Postpone->setEnabled(days_to_due == 0);
        mAction_Skip->setEnabled(true);
        mAction_Cancel->setEnabled(false);
        break;

    case clDataElem_RemDayStatus::Scheduled :
        mAction_Schedule->setText("edit schedule");
        mAction_Postpone->setEnabled(false);
        mAction_Skip->setEnabled(false);
        mAction_Cancel->setEnabled(true);
        break;

    case clDataElem_RemDayStatus::Postponed :
        mAction_Schedule->setText("schedule on today");
        mAction_Postpone->setEnabled(false);
        mAction_Skip->setEnabled(true);
        mAction_Cancel->setEnabled(true);
        break;

    case clDataElem_RemDayStatus::Skipped :
        mAction_Schedule->setText("schedule");
        mAction_Postpone->setEnabled(days_to_due == 0);
        mAction_Skip->setEnabled(false);
        mAction_Cancel->setEnabled(true);
        break;
    }

    mAction_Skip->setText((days_to_due==0) ? "skip" : "skip for today");

    //
    QAction *action_triggered
        = mContextMenu->exec(ui->tableWidget_reminder_titles->viewport()->mapToGlobal(pos));
    if(action_triggered == Q_NULLPTR)
        return;

    if(action_triggered == mAction_Schedule)
    {
        clUI_DayPlan_ScheduleDialog dialog(status0.get_scheduled_times());
        int r = dialog.exec();
        if(r == QDialog::Accepted)
        {
            auto ranges = dialog.get_hrmin_ranges();
            if(ranges.isEmpty())
                mDayPlanningStatus[rem_id].set_unscheduled();
            else
                mDayPlanningStatus[rem_id].set_scheduled(ranges);
        }
    }
    else if(action_triggered == mAction_Postpone)
    {
        QDate date0 = ui->dateEdit->date();

        clUI_Dialog_CalendarDatePicker dialog;
        dialog.set_label_text("Postpone to which date?");
        dialog.set_minimum_date(date0.addDays(1));
        dialog.set_date(date0.addDays(1));
        int r = dialog.exec();
        if(r == QDialog::Accepted)
            mDayPlanningStatus[rem_id].set_postponed(dialog.get_selected_date());
    }
    else if(action_triggered == mAction_Skip)
    {
        if(days_to_due == 0)
        {
            int reply = QMessageBox::warning(this, "confirm",
                                             "The task is due today. Really skip it?",
                                             QMessageBox::Yes | QMessageBox::No,
                                             QMessageBox::No);
            if(reply == QMessageBox::No)
                return;
        }

        mDayPlanningStatus[rem_id].set_skipped();
    }
    else if(action_triggered == mAction_Cancel)
    {
        if(status0.get_status() == clDataElem_RemDayStatus::Scheduled)
        {
            Q_ASSERT(schedule_No >= 0);
            status0.remove_ith_schedule(schedule_No);
            mDayPlanningStatus[rem_id] = status0;
        }
        else
            mDayPlanningStatus[rem_id].set_unscheduled();
    }

    // rebuild table
    build_table();

    // save changes
    emit day_planning_status_modified(ui->dateEdit->date(), mDayPlanningStatus);
}
