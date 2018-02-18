#include <QHeaderView>
#include <QColor>
#include <QMessageBox>
#include "UI_DayPlan_UpdateRecordTable.h"
#include "UI_Dialog_CalendarDatePicker.h"

const QColor StateTodoColor(Qt::blue),
             StateDoneColor(Qt::darkGreen),
             StateSkippedColor(Qt::magenta),
             StateDateShiftedColor(Qt::black);
const QColor HighlightBackgroundColor("#ccc");

clUI_DayPlan_UpdateRecordTable::clUI_DayPlan_UpdateRecordTable(QTableWidget *table,
                                                               QWidget *parent)
    : QObject(parent), mTable(table)
{
    // table set up
    mTable->setRowCount(0);
    mTable->setColumnCount(3);

    mTable->setStyleSheet("font: 11pt");
    mTable->setHorizontalHeaderLabels(QStringList() << "task" << "deadline" << "state");
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
    mActions << mContextMenu->addAction("Todo")  //[0]
             << mContextMenu->addAction("Done")  //[1]
             << mContextMenu->addAction("Postpone/Advance") //[2]
             << mContextMenu->addAction("Skip")  //[3]
             << mContextMenu->addSeparator() //[4]
             << mContextMenu->addAction("Show in \"All Reminders\""); //[5]

    //
    connect(mTable, SIGNAL(cellClicked(int,int)), this, SLOT(cell_clicked(int,int)));
    connect(mTable, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(context_menu_requested(QPoint)));
}

void clUI_DayPlan_UpdateRecordTable::set_table_visible(const bool visible)
{
    mTable->setVisible(visible);
}

void clUI_DayPlan_UpdateRecordTable::clear()
{
    mTable->setRowCount(0);
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

void clUI_DayPlan_UpdateRecordTable::add_task(const clTask &task, const QString &title,
                                              const clTaskState &state)
{
    int row = mTable->rowCount();
    mTable->setRowCount(row+1);

    QTableWidgetItem *item;

    // title & id
    item = new QTableWidgetItem(title);
    item->setData(Qt::UserRole, task.mRemID);
    mTable->setItem(row, 0, item);

    // deadline
    item = new QTableWidgetItem(task.mDeadline.toString("yyyy/M/d"));
    mTable->setItem(row, 1, item);

    // state
    item = new QTableWidgetItem;
    mTable->setItem(row, 2, item);
    update_state_of_row(row, state);
}

void clUI_DayPlan_UpdateRecordTable::add_tasks(const QList<clTask> &tasks, const QString &titles,
                                               const QList<clTaskState> &states)
{
    const int n = tasks.size();
    Q_ASSERT(states.size() == n);
    Q_ASSERT(titles.size() == n);

    for(int i=0; i<n; i++)
        add_task(tasks.at(i), titles.at(i), states.at(i));
}

void clUI_DayPlan_UpdateRecordTable::deselect()
{
    mTable->setCurrentCell(-1, -1);
}

void clUI_DayPlan_UpdateRecordTable::highlight_row(const int row)
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

void clUI_DayPlan_UpdateRecordTable::highlight_task(const clTask &task)
{
    int row_to_highlight = -1;
    for(int r=0; r<mTable->rowCount(); r++)
    {
        if(get_task_of_row(r) == task)
        {
            row_to_highlight = r;
            break;
        }
    }

    highlight_row(row_to_highlight);
}

void clUI_DayPlan_UpdateRecordTable::highlight_none()
{
    highlight_row(-1);
}

void clUI_DayPlan_UpdateRecordTable::cell_clicked(int row, int column)
//highlight and emit signal
{
    if(row<0 || column<0)
        return;

    clTask task = get_task_of_row(row);
    highlight_task(task);
    emit user_selected_task(task);
}

clTask clUI_DayPlan_UpdateRecordTable::get_task_of_row(const int row) const
{
    Q_ASSERT(row >= 0 && row < mTable->rowCount());

    bool ok;
    int id = mTable->item(row, 0)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    QDate deadline = QDate::fromString(mTable->item(row, 1)->text(), "yyyy/M/d");
    Q_ASSERT(deadline.isValid());

    return clTask(id, deadline);
}

void clUI_DayPlan_UpdateRecordTable::context_menu_requested(const QPoint &pos)
{
    QTableWidgetItem *item = mTable->itemAt(pos);
    if(item == nullptr)
        return;

    const int row = item->row();
    const clTask task = get_task_of_row(row);

    // highlight and emit signal
    highlight_row(row);
    emit user_selected_task(task);

    // set allowed actions
    bool ok;
    int state_code = mTable->item(row, 2)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    mActions[0]->setVisible(state_code != clTaskState::Todo);
    mActions[1]->setVisible(state_code != clTaskState::Done);
    mActions[2]->setVisible(state_code != clTaskState::DateShifted);
    mActions[3]->setVisible(state_code != clTaskState::Skipped);

    // show menu
    QAction *selected_action = mContextMenu->exec(mTable->viewport()->mapToGlobal(pos));

    // perform selected action
    if(selected_action == mActions[5]) //(show in "All Reminders")
    {
        emit to_show_in_all_reminders(task.mRemID);
    }
    else //(change task state)
    {
        // set new state
        clTaskState new_state; //defaut: Todo
        if(selected_action == mActions.at(1)) //(Done)
            new_state.set_state(clTaskState::Done);
        else if(selected_action == mActions.at(2)) //(Postpone/Advance)
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
        else if(selected_action == mActions.at(3)) //(Skip)
        {
            if(QMessageBox::question(nullptr, "warning", "Are you sure to skip the task?")
               != QMessageBox::Yes)
                return;
            new_state.set_state(clTaskState::Skipped);
        }

        //
        update_state_of_row(row, new_state);
        emit task_state_changed(task, new_state);
    }
}

void clUI_DayPlan_UpdateRecordTable::update_state_of_row(const int row, const clTaskState &state)
//column 2 of row `row` must already have a widget item
{
    QTableWidgetItem *item = mTable->item(row, 2);
    Q_ASSERT(item != nullptr);

    if(state.is_date_shifted())
    {
        QString text = QString("date shifted to \n%1")
                       .arg(state.get_shifted_date().toString("yyyy/M/d"));
        item->setText(text);
    }
    else
        item->setText(state.print());

    item->setData(Qt::UserRole, int(state.get_state()));
    item->setData(Qt::ForegroundRole, QBrush(get_state_color(state)));
}
