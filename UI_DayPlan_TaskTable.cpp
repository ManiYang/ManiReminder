#include <QHeaderView>
#include <QMessageBox>
#include "UI_DayPlan_TaskTable.h"
#include "UI_Dialog_CalendarDatePicker.h"
#include "UI_DayPlan_Dialog_Schedule.h"

const QColor StateTodoColor(Qt::blue),
             StateDoneColor(Qt::darkGreen),
             StateSkippedColor(Qt::magenta),
             StateDateShiftedColor(Qt::black);
const QColor HighlightBackgroundColor("#ccc");

clUI_DayPlan_TaskTable::clUI_DayPlan_TaskTable(QTableWidget *table, QWidget *parent)
    : QObject(parent), mTable(table)
{
    // table set up
    mTable->setRowCount(0);
    mTable->setColumnCount(4);

    mTable->setHorizontalHeaderLabels(QStringList()
                                      << "task" << "deadline" << "state" << "schedule");
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mTable->horizontalHeader()->setStyleSheet("color: #555555");
    mTable->horizontalHeader()->setHighlightSections(false);

    mTable->verticalHeader()->setVisible(false);
    mTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);

    mTable->setEditTriggers(QTableWidget::NoEditTriggers);
    mTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // context menu
    mContextMenu = new QMenu(parent);
    mActions << mContextMenu->addAction("Schedule/Edit Sessions") //[0]
             << mContextMenu->addSeparator() //[1]
             << mContextMenu->addAction("Todo")             //[2]
             << mContextMenu->addAction("Done")             //[3]
             << mContextMenu->addAction("Postpone/Advance") //[4]
             << mContextMenu->addAction("Skip")             //[5]
             << mContextMenu->addSeparator() //[6]
             << mContextMenu->addAction("Show in \"All Reminders\""); //[7]

    //
    connect(mTable, SIGNAL(cellClicked(int,int)), this, SLOT(cell_clicked(int,int)));
    connect(mTable, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(context_menu_requested(QPoint)));
}

void clUI_DayPlan_TaskTable::clear()
{
    mTable->setRowCount(0);
    mRelativeToDate = QDate();
}

void clUI_DayPlan_TaskTable::set_relative_to_date(const QDate &date)
{
    Q_ASSERT(date.isValid());
    mRelativeToDate = date;
}

void clUI_DayPlan_TaskTable::deselect()
{
    mTable->setCurrentCell(-1, -1);
}

static QColor get_state_color(const clTaskState &state)
{
    switch(state.get_state())
    {
    case clTaskState::Done :
        return StateDoneColor;
    case clTaskState::Todo :
        return StateTodoColor;
    case clTaskState::Skipped :
        return StateSkippedColor;
    case clTaskState::DateShifted :
        return StateDateShiftedColor;
    default:
        return StateDoneColor;
    }
}

void clUI_DayPlan_TaskTable::add_task(const clTask &task, const QString &title,
                                      const bool deadline_verbose,
                                      const clTaskState &state, const bool scheduled)
{
    Q_ASSERT(mRelativeToDate.isValid());

    int row = mTable->rowCount();
    mTable->setRowCount(row+1);

    QTableWidgetItem *item;

    // title & id
    item = new QTableWidgetItem(title);
    item->setData(Qt::UserRole, task.mRemID);
    mTable->setItem(row, 0, item);

    // deadline
    int day_diff = mRelativeToDate.daysTo(task.mDeadline);
    QString str;
    if(deadline_verbose)
    {
        if(day_diff == 0)
            str = "due today";
        else if(day_diff > 0)
            str = QString("due in %1 d").arg(day_diff);
        else //(day_diff < 0)
            str = QString("overdue for %1 d").arg(-day_diff);
    }
    else
        str = QString::asprintf("%+d d", day_diff);

    item = new QTableWidgetItem(str);
    item->setData(Qt::ToolTipRole, task.mDeadline.toString("yyyy/M/d"));
    mTable->setItem(row, 1, item);

    // state
    item = new QTableWidgetItem;
    mTable->setItem(row, 2, item);
    update_state_of_row(row, state);

    // scheduled
    item = new QTableWidgetItem;
    item->setText(scheduled ? "scheduled" : "");
    mTable->setItem(row, 3, item);
}

void clUI_DayPlan_TaskTable::highlight_task(const clTask &task)
{
    int row = -1;
    for(int r=0; r<mTable->rowCount(); r++)
    {
        if(get_task_of_row(r) == task)
        {
            row = r;
            break;
        }
    }

    highlight_row(row);
}

void clUI_DayPlan_TaskTable::highlight_none()
{
    highlight_row(-1);
}

clTask clUI_DayPlan_TaskTable::get_task_of_row(const int row) const
{
    Q_ASSERT(row >= 0 && row < mTable->rowCount());

    bool ok;
    int id = mTable->item(row, 0)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    QString str = mTable->item(row, 1)->data(Qt::ToolTipRole).toString();
    QDate deadline = QDate::fromString(str, "yyyy/M/d");
    Q_ASSERT(deadline.isValid());

    return clTask(id, deadline);
}

void clUI_DayPlan_TaskTable::highlight_row(const int row)
//`row` can be outside the normal range
{
    for(int r=0; r<mTable->rowCount(); r++)
    {
        QBrush brush; //default: no fill
        if(r == row)
        {
            brush.setStyle(Qt::SolidPattern);
            brush.setColor(HighlightBackgroundColor);
        }
        mTable->item(r, 0)->setData(Qt::BackgroundRole, brush);
        mTable->item(r, 1)->setData(Qt::BackgroundRole, brush);
    }
}

void clUI_DayPlan_TaskTable::cell_clicked(int row, int column)
//highlight and emit signal
{
    if(row<0 || column<0)
        return;

    clTask task = get_task_of_row(row);
    highlight_task(task);
    emit user_selected_task(task);
}

void clUI_DayPlan_TaskTable::context_menu_requested(const QPoint &pos)
{
    const int col = mTable->columnAt(pos.x());
    const int row = mTable->rowAt(pos.y());
    if(col < 0 || row < 0)
        return;

    const clTask task = get_task_of_row(row);

    // highlight and emit signal
    highlight_row(row);
    emit user_selected_task(task);

    // set allowed actions
    bool ok;
    int state_code = mTable->item(row, 2)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    mActions[2]->setVisible(state_code != clTaskState::Todo);
    mActions[3]->setVisible(state_code != clTaskState::Done);
    mActions[4]->setVisible(state_code != clTaskState::DateShifted);
    mActions[5]->setVisible(state_code != clTaskState::Skipped);

    // show menu
    QAction *selected_action = mContextMenu->exec(mTable->viewport()->mapToGlobal(pos));

    // perform selected action
    if(selected_action == mActions.at(0)) //(schedule/edit sessions)
    {
        QString title = mTable->item(row, 0)->text();
        to_schedule_sessions(row, task, title);
    }
    else if(selected_action == mActions.at(7)) //(show in "All Reminders")
    {
        emit to_show_in_all_reminders(task.mRemID);
    }
    else //(change state)
    {
        clTaskState::enState new_state_code;
        if(selected_action == mActions.at(2))
            new_state_code = clTaskState::Todo;
        else if(selected_action == mActions.at(3))
            new_state_code = clTaskState::Done;
        else if(selected_action == mActions.at(4))
            new_state_code = clTaskState::DateShifted;
        else
            new_state_code = clTaskState::Skipped;

        to_change_task_state(row, task, new_state_code);
    }
}

void clUI_DayPlan_TaskTable::to_schedule_sessions(const int row, const clTask &task,
                                                  const QString &title)
{
    QList<clDataElem_DayScheduleItem> sessions;
    get_scheduled_sessions_for_task(task, &sessions);

    clUI_DayPlan_ScheduleDialog *dialog;
    if(sessions.isEmpty())
        dialog = new clUI_DayPlan_ScheduleDialog(task, title);
    else
        dialog = new clUI_DayPlan_ScheduleDialog(sessions);

    int r = dialog->exec();
    if(r == QDialog::Accepted)
    {
        QList<clDataElem_DayScheduleItem> new_sessions = dialog->get_sessions();
        emit task_sessions_modified(task, new_sessions);

        //
        QString str = new_sessions.isEmpty() ? "" : "scheduled";
        mTable->item(row, 3)->setText(str);
    }

    dialog->deleteLater();
}

void clUI_DayPlan_TaskTable::to_change_task_state(const int row, const clTask &task,
                                                  const clTaskState::enState new_state_code)
{
    clTaskState new_state;
    if(new_state_code == clTaskState::DateShifted)
    {
        clUI_Dialog_CalendarDatePicker dialog;
        const QDate today = QDate::currentDate();
        dialog.set_minimum_date(today);
        QDate d = task.mDeadline;
        if(d < today)
            d = today;
        dialog.set_date(d);
        dialog.set_label_text("Postpone/advance to which day?");
        int r = dialog.exec();
        if(r != QDialog::Accepted)
            return;
        d = dialog.get_selected_date();
        if(d == task.mDeadline)
            return;
        new_state.set_state(clTaskState::DateShifted, d);
    }
    else if(new_state_code == clTaskState::Skipped)
    {
        if(QMessageBox::question(nullptr, "warning", "Are you sure to skip the task?")
           != QMessageBox::Yes)
            return;
        new_state.set_state(clTaskState::Skipped);
    }
    else
        new_state.set_state(new_state_code);

    // update table
    update_state_of_row(row, new_state);
    emit task_state_changed(task, new_state);
}

void clUI_DayPlan_TaskTable::update_state_of_row(const int row, const clTaskState &state)
//column 2 of row `row` must already have a table-widget item
{
    QTableWidgetItem *item = mTable->item(row, 2);
    Q_ASSERT(item != nullptr);

    Q_ASSERT(mRelativeToDate.isValid());

    if(state.is_date_shifted())
    {
        int dd = mRelativeToDate.daysTo(state.get_shifted_date());
        if(dd == 0)
            item->setText("date shifted to\ntoday");
        else if(dd > 0)
            item->setText(QString("date shifted to\n%1 d after").arg(dd));
        else //(dd < 0)
            item->setText(QString("date shifted to\n%1 d before").arg(-dd));
    }
    else
        item->setText(state.print());

    item->setData(Qt::UserRole, int(state.get_state()));
    item->setData(Qt::ForegroundRole, QBrush(get_state_color(state)));
}
