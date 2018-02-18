#ifndef UI_DAYPLAN_SCHEDULETABLE_H
#define UI_DAYPLAN_SCHEDULETABLE_H

#include <QObject>
#include <QTableWidget>
#include <QList>
#include <QMenu>
#include "DataElem_TaskDayScheduleSession.h"

/* Manage the schedule table and the scheduled sessions of tasks and temporary reminders. */

class clUI_DayPlan_ScheduleTable : public QObject
{
    Q_OBJECT

public:
    explicit clUI_DayPlan_ScheduleTable(QTableWidget *table, QWidget *parent = nullptr);

    void clear();

    void add_schedule_items(const QList<clDataElem_DayScheduleItem> &items);
                            //highligh will be removed

    void show_nontask_reminders(const bool show_reminders_w_time_trigger,
                                const bool show_reminders_w_time_range_binding);
                                //highligh is retained using `mHighlightedTask`,
                                //`mHighlightedTempRemTitle` and `mHighlightedNonTaskID`

    void update_sessions_for_task(const clUtil_Task &task,
                                  const QList<clDataElem_DayScheduleItem> &new_sessions);
                                  //highligh will be removed

    void highlight_task(const clUtil_Task &task);
    void highlight_temporary_reminder(const QString &title);
    void highlight_nontask(const int rem_id);
    void highlight_none();

    void deselect();

    //
    QList<clDataElem_DayScheduleItem> get_task_and_temporary_sessions() const;
    QList<clDataElem_DayScheduleItem> get_sessions_for_task(const clUtil_Task &task) const;
    QList<clDataElem_DayScheduleItem> get_sessions_for_temporary_reminder(
                                                                     const QString &title) const;

signals:
    void user_selected_task(const clUtil_Task &task);
    void user_selected_nontask(const int rem_id); //-2: temporary sessions
    //void no_item_selected();

    void task_sessions_modified(const clUtil_Task &task);
    void to_show_in_all_reminders(const int rem_id);

//-----------------------

private slots:
    void cell_clicked(int row, int column); //highlight the item and send signals
    void context_menu_requested(const QPoint &pos);

private:
    QTableWidget *mTable;

    // internal data
    QList<clDataElem_DayScheduleItem> mScheduleItems;
    bool mShowRem_w_Trigger;
    bool mShowRem_w_Binding;

    clUtil_Task mHighlightedTask; //has id=-1 => no task is highlighted
    QString mHighlightedTempRemTitle; //null => no temp. rem. is highlighted
    int mHighlightedNonTaskID; //-1 => no non-task (with trigger or binding) is highlighted

    // context menu
    QMenu *mContextMenu;
    QList<QAction *> mActions;

    //
    void sort_items(); //sort `mScheduleItems[]`
    void update_table(); //According to `mScheduleItems[]` (including the order),
                         //`mShowRem_w_Trigger`, and `mShowRem_w_Binding`.
                         //Highlight will be removed.
    void add_table_row(const int list_index, const clDataElem_DayScheduleItem &DSItem);
    void highlight(); //according to `mHighlightedTask`, `mHighlightedTempRemTitle`, and
                      //`mHighlightedNonTaskID`
    void to_modify_sessions(const int index); //to modify sessions for `mScheduleItems[index]`

    QSet<int> find_item_indices_for_task(const clUtil_Task &task) const;
    QSet<int> find_item_indices_for_temp_rem(const QString &title) const;
};

#endif // UI_DAYPLAN_SCHEDULETABLE_H
