#ifndef UI_DAYPLAN_UPDATERECORDTABLE_H
#define UI_DAYPLAN_UPDATERECORDTABLE_H

#include <QObject>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include "Util_Task.h"
#include "DataElem_TaskState.h"
typedef clUtil_Task clTask;
typedef clDataElem_TaskState clTaskState;

class clUI_DayPlan_UpdateRecordTable : public QObject
{
    Q_OBJECT
public:
    explicit clUI_DayPlan_UpdateRecordTable(QTableWidget *table,
                                            QWidget *parent = nullptr);

    void set_table_visible(const bool visible);

    void clear();
    void add_task(const clTask &task, const QString &title, const clTaskState &state);
    void add_tasks(const QList<clTask> &tasks, const QString &titles,
                   const QList<clTaskState> &states);

    void highlight_task(const clTask &task);
    void highlight_none();

    void deselect();

signals:
    void user_selected_task(const clUtil_Task &task);
    void task_state_changed(const clTask &task, const clTaskState &new_state);
    void to_show_in_all_reminders(const int rem_id);

//-----------------------------

private slots:
    void cell_clicked(int row, int column); //highlight and emit signal
    void context_menu_requested(const QPoint &pos);

private:
    QTableWidget *mTable;

    // context menu
    QMenu *mContextMenu;
    QList<QAction *> mActions;

    //
    clTask get_task_of_row(const int row) const;
    void highlight_row(const int row);
    void update_state_of_row(const int row, const clTaskState &state);
                             //column 2 of row `row` must already have a widget item
};

#endif // UI_DAYPLAN_UPDATERECORDTABLE_H
