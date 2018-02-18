#include <QHeaderView>
#include <QStringList>
#include <QDebug>
#include "UI_DayPlan_ScheduleTable.h"
#include "UI_DayPlan_Dialog_Schedule.h"
#include "utility_general.h"

const QColor HighlightBackgroundColor("#ccc");

clUI_DayPlan_ScheduleTable::clUI_DayPlan_ScheduleTable(QTableWidget *table, QWidget *parent)
    : QObject(parent), mTable(table),
      mShowRem_w_Trigger(true), mShowRem_w_Binding(true),
      mHighlightedNonTaskID(-1)
{
    // table set up
    mTable->setRowCount(0);
    mTable->setColumnCount(2);

    mTable->setHorizontalHeaderLabels(QStringList() << "title" << "time" );
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
    mActions << mContextMenu->addAction("Schedule/Edit Sessions")   //[0]
             << mContextMenu->addSeparator() //[1]
             << mContextMenu->addAction("Show in \"All Reminders\""); //[2]

    //
    connect(mTable, SIGNAL(cellClicked(int,int)), this, SLOT(cell_clicked(int,int)));
    connect(mTable, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(context_menu_requested(QPoint)));
}

void clUI_DayPlan_ScheduleTable::add_schedule_items(
                                                const QList<clDataElem_DayScheduleItem> &items)
//highligh will be removed
{
    mScheduleItems << items;
    sort_items();
    update_table();
}

void clUI_DayPlan_ScheduleTable::sort_items()
//sort `mScheduleItems[]`
{
    QMultiMap<QTime, int> sort_key;
    for(int i=mScheduleItems.size()-1; i>=0; i--)
    {
        QTime t = mScheduleItems.at(i).get_start_or_triggering_time();
        sort_key.insert(t, i);
    }

    QList<clDataElem_DayScheduleItem> new_list;
    for(auto it=sort_key.begin(); it!=sort_key.end(); it++)
    {
        int i = it.value();
        new_list << mScheduleItems.at(i);
    }

    mScheduleItems = new_list;
}

void clUI_DayPlan_ScheduleTable::update_table()
//According to `mScheduleItems[]` (including the order), `mShowRem_w_Trigger`, and
//`mShowRem_w_Binding`.
//Highlight will be removed.
{
    // remove all current table items and highlight
    mTable->setRowCount(0);

    // add items
    for(int i=0; i<mScheduleItems.size(); i++)
    {
        bool add = true;
        if(mScheduleItems.at(i).is_nontask_w_time_trigger())
        {
            if(! mShowRem_w_Trigger)
                add = false;
        }
        else if(mScheduleItems.at(i).is_nontask_w_time_range_binding())
        {
            if(! mShowRem_w_Binding)
                add = false;
        }

        //
        if(add)
            add_table_row(i, mScheduleItems.at(i));
    }  
}

void clUI_DayPlan_ScheduleTable::add_table_row(const int list_index,
                                               const clDataElem_DayScheduleItem &DSItem)
{
    const int row = mTable->rowCount();
    mTable->setRowCount(row+1);

    ///
    QTableWidgetItem *item;

    // column 0: title, list index
    item = new QTableWidgetItem(DSItem.get_title());
    item->setData(Qt::UserRole, list_index);
    mTable->setItem(row, 0, item);

    // column 1: prefix, time
    item = new QTableWidgetItem(DSItem.print_time());
    mTable->setItem(row, 1, item);
}

void clUI_DayPlan_ScheduleTable::highlight_task(const clUtil_Task &task)
{
    Q_ASSERT(task.mRemID >= 0);

    mHighlightedTask = task;
    mHighlightedTempRemTitle = QString();
    mHighlightedNonTaskID = -1;

    highlight();
}

void clUI_DayPlan_ScheduleTable::highlight_temporary_reminder(const QString &title)
{
    Q_ASSERT(! title.isNull());

    mHighlightedTask.mRemID = -1;
    mHighlightedTempRemTitle = title;
    mHighlightedNonTaskID = -1;

    highlight();
}

void clUI_DayPlan_ScheduleTable::highlight_nontask(const int rem_id)
{
    Q_ASSERT(rem_id >= 0);

    mHighlightedTask.mRemID = -1;
    mHighlightedTempRemTitle = QString();
    mHighlightedNonTaskID = rem_id;

    highlight();
}

void clUI_DayPlan_ScheduleTable::highlight()
//according to `mHighlightedTask`, `mHighlightedTempRemTitle`, and `mHighlightedNonTaskID`
{
    for(int r=0; r<mTable->rowCount(); r++)
    {
        bool ok;
        int index = mTable->item(r, 0)->data(Qt::UserRole).toInt(&ok);
        Q_ASSERT(ok);
        Q_ASSERT(index>=0 && index<mScheduleItems.size());

        //
        bool to_highlight = false;
        if(mHighlightedTask.mRemID != -1)
        {
            if(mScheduleItems.at(index).is_task())
            {
                if(mScheduleItems.at(index).get_task() == mHighlightedTask)
                    to_highlight = true;
            }
        }
        else if(! mHighlightedTempRemTitle.isNull())
        {
            if(mScheduleItems.at(index).is_temperary_reminder())
            {
                if(mScheduleItems.at(index).get_title() == mHighlightedTempRemTitle)
                    to_highlight = true;
            }
        }
        else if(mHighlightedNonTaskID != -1)
        {
            if(mScheduleItems.at(index).get_reminder_ID() == mHighlightedNonTaskID)
                to_highlight = true;
        }

        //
        QBrush brush; //default: no fill
        if(to_highlight)
        {
            brush.setStyle(Qt::SolidPattern);
            brush.setColor(HighlightBackgroundColor);
        }
        mTable->item(r, 0)->setData(Qt::BackgroundRole, brush);
    }
}

void clUI_DayPlan_ScheduleTable::highlight_none()
{
    mHighlightedTask.mRemID = -1;
    mHighlightedTempRemTitle = QString();
    mHighlightedNonTaskID = -1;

    for(int r=0; r<mTable->rowCount(); r++)
        mTable->item(r, 0)->setData(Qt::BackgroundRole, QBrush()); //default no-fill brush
}

void clUI_DayPlan_ScheduleTable::deselect()
{
    mTable->setCurrentCell(-1, -1);
}

void clUI_DayPlan_ScheduleTable::show_nontask_reminders(
                                               const bool show_reminders_w_time_trigger,
                                               const bool show_reminders_w_time_range_binding)
//highligh is retained using `mHighlightedTask`, `mHighlightedTempRemTitle` and
//`mHighlightedNonTaskID`
{
    mShowRem_w_Trigger = show_reminders_w_time_trigger;
    mShowRem_w_Binding = show_reminders_w_time_range_binding;
    update_table();

    // recover original highlight
    highlight();
}

QList<clDataElem_DayScheduleItem> clUI_DayPlan_ScheduleTable::
                                                        get_task_and_temporary_sessions() const
{
    QList<clDataElem_DayScheduleItem> items;
    for(auto it=mScheduleItems.cbegin(); it!=mScheduleItems.cend(); it++)
    {
        if(it->is_task() || it->is_temperary_reminder() )
            items << *it;
    }
    return items;
}

QList<clDataElem_DayScheduleItem> clUI_DayPlan_ScheduleTable::get_sessions_for_task(
                                                                 const clUtil_Task &task) const
{
    QSet<int> indices = find_item_indices_for_task(task);

    QList<clDataElem_DayScheduleItem> items;
    foreach(int i, indices)
        items << mScheduleItems.at(i);
    return items;
}

QList<clDataElem_DayScheduleItem> clUI_DayPlan_ScheduleTable::
                                           get_sessions_for_temporary_reminder(
                                                                   const QString &title) const
{
    QSet<int> indices = find_item_indices_for_temp_rem(title);

    QList<clDataElem_DayScheduleItem> items;
    foreach(int i, indices)
        items << mScheduleItems.at(i);
    return items;
}

void clUI_DayPlan_ScheduleTable::cell_clicked(int row, int column)
//highlight the item and send signals
{
    if(row<0 || column<0)
        return;

    bool ok;
    const int index = mTable->item(row, 0)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    if(mScheduleItems.at(index).is_task())
    {
        highlight_task(mScheduleItems.at(index).get_task());
        emit user_selected_task(mScheduleItems.at(index).get_task());
    }
    else if(mScheduleItems.at(index).is_temperary_reminder())
    {
        highlight_temporary_reminder(mScheduleItems.at(index).get_title());
        emit user_selected_nontask(mScheduleItems.at(index).get_reminder_ID());
    }
    else //(is reminder with time trigger or time-range binding)
    {
        highlight_nontask(mScheduleItems.at(index).get_reminder_ID());
        emit user_selected_nontask(mScheduleItems.at(index).get_reminder_ID());
    }
}

void clUI_DayPlan_ScheduleTable::context_menu_requested(const QPoint &pos)
{
    QTableWidgetItem *item = mTable->itemAt(pos);
    if(item == nullptr)
        return;

    const int row = item->row();
    bool ok;
    const int index = mTable->item(row, 0)->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);

    //
    cell_clicked(row, item->column()); //highlight the item and send signals

    // set allowed actions
    const clDataElem_DayScheduleItem &DSItem = mScheduleItems.at(index);
    mActions[0]->setVisible(DSItem.is_task() || DSItem.is_temperary_reminder());
    mActions[2]->setVisible(! DSItem.is_temperary_reminder());

    // show menu
    QAction *selected_action = mContextMenu->exec(mTable->viewport()->mapToGlobal(pos));

    // perform action
    if(selected_action == mActions.at(0)) //schedule/modify sessions
    {
        to_modify_sessions(index);
    }
    else if(selected_action == mActions.at(2)) //show in "All Reminders"
    {
        emit to_show_in_all_reminders(DSItem.get_reminder_ID());
    }
}

void clUI_DayPlan_ScheduleTable::to_modify_sessions(const int index)
//to modify sessions for `mScheduleItems[index]`
{
    const clDataElem_DayScheduleItem DSItem = mScheduleItems.at(index);
    Q_ASSERT(DSItem.is_task() || DSItem.is_temperary_reminder());

    QSet<int> indices;
    QList<clDataElem_DayScheduleItem> sessions;
    clUtil_Task task; //set only if `DSItem` is a task
    if(DSItem.is_task())
    {
        task = DSItem.get_task();
        indices = find_item_indices_for_task(task);
        sessions = get_sessions_for_task(task);
    }
    else
    {
        QString title = DSItem.get_title();
        indices = find_item_indices_for_temp_rem(title);
        sessions = get_sessions_for_temporary_reminder(title);
    }

    // show dialog
    clUI_DayPlan_ScheduleDialog dialog(sessions);
    int r = dialog.exec();
    if(r != QDialog::Accepted)
        return;

    // remove `mScheduleItems[indices[]]`
    remove_elements_from_list(mScheduleItems, indices);

    // add new sessions
    const QList<clDataElem_DayScheduleItem> new_sessions = dialog.get_sessions();
    mScheduleItems << new_sessions;

    //
    sort_items();
    update_table(); //highlight is removed
    deselect();

    // highlight
    if(! new_sessions.isEmpty())
    {
        if(DSItem.is_task())
            highlight_task(task);
        else
            highlight_temporary_reminder(new_sessions.at(0).get_title());
    }

    //
    if(DSItem.is_task())
        emit task_sessions_modified(task);
}

void clUI_DayPlan_ScheduleTable::update_sessions_for_task(
                                       const clUtil_Task &task,
                                       const QList<clDataElem_DayScheduleItem> &new_sessions)
//highligh will be removed
{
    // remove all items for task `task`
    QList<clDataElem_DayScheduleItem>::iterator it = mScheduleItems.begin();
    while(it != mScheduleItems.end())
    {
        if(it->is_task())
        {
            if(it->get_task() == task)
            {
                it = mScheduleItems.erase(it);
                continue;
            }
        }

        it++;
    }

    // add new items
    mScheduleItems << new_sessions;

    //
    sort_items();
    update_table();
}

QSet<int> clUI_DayPlan_ScheduleTable::find_item_indices_for_task(const clUtil_Task &task) const
{
    QSet<int> indices;
    for(int i=0; i<mScheduleItems.size(); i++)
    {
        if(mScheduleItems.at(i).is_task())
        {
            if(mScheduleItems.at(i).get_task() == task)
                indices << i;
        }
    }
    return indices;
}

QSet<int> clUI_DayPlan_ScheduleTable::find_item_indices_for_temp_rem(const QString &title) const
{
    QSet<int> indices;
    for(int i=0; i<mScheduleItems.size(); i++)
    {
        if(mScheduleItems.at(i).is_temperary_reminder())
        {
            if(mScheduleItems.at(i).get_title() == title)
                indices << i;
        }
    }
    return indices;
}

void clUI_DayPlan_ScheduleTable::clear()
{
    mScheduleItems.clear();
    update_table();
}
