#ifndef UI_DAYPLAN_TASKTABLE_H
#define UI_DAYPLAN_TASKTABLE_H

#include <QObject>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include "Util_Task.h"
#include "DataElem_TaskState.h"
#include "DataElem_TaskDayScheduleSession.h"
typedef clUtil_Task clTask;
typedef clDataElem_TaskState clTaskState;

class clUI_DayPlan_TaskTable : public QObject
{
    Q_OBJECT
public:
    explicit clUI_DayPlan_TaskTable(QTableWidget *table, QWidget *parent = nullptr);

    void clear();
    void set_relative_to_date(const QDate &date);
    void add_task(const clTask &task, const QString &title, const bool deadline_verbose,
                  const clTaskState &state, const bool scheduled);

    void highlight_task(const clTask &task);
    void highlight_none();

    void deselect();

signals:
    void user_selected_task(const clUtil_Task &task);
    void task_state_changed(const clTask &task, const clTaskState &new_state);
    void to_show_in_all_reminders(const int rem_id);

    void get_scheduled_sessions_for_task(const clUtil_Task &task,
                                         QList<clDataElem_DayScheduleItem> *sessions);
    void task_sessions_modified(const clUtil_Task &task,
                                const QList<clDataElem_DayScheduleItem> &new_sessions);

//----------------------

private slots:
    void cell_clicked(int row, int column); //highlight and emit signal
    void context_menu_requested(const QPoint &pos);

private:
    QTableWidget *mTable;

    QDate mRelativeToDate;

    // context menu
    QMenu *mContextMenu;
    QList<QAction *> mActions;

    //
    clTask get_task_of_row(const int row) const;
    void highlight_row(const int row); //`row` can be outside the normal range
    void to_schedule_sessions(const int row, const clTask &task, const QString &title);
    void to_change_task_state(const int row, const clTask &task,
                              const clTaskState::enState new_state_code);
    void update_state_of_row(const int row, const clTaskState &state);
                             //column 2 of row `row` must already have a table-widget item
};

#endif // UI_DAYPLAN_TASKTABLE_H
