#ifndef UI_ALL_REMINDERS_H
#define UI_ALL_REMINDERS_H

#include <QWidget>
#include <QMap>
#include <QListWidget>
#include <QMenu>
#include "Reminder.h"

namespace Ui {
class clUI_AllReminders;
}

class clUI_AllReminders : public QWidget
{
    Q_OBJECT

public:
    explicit clUI_AllReminders(const QMap<int, const clReminder*> *p_reminders,
                               const QSet<QString> *p_situations,
                               const QSet<QString> *p_events,
                               QWidget *parent = nullptr);
    ~clUI_AllReminders();

    void update_all_reminders_view(); //reload all reminders
    void update_reminder_view(const int id);
                              //call this when the data of reminder `id` has updated
    void add_reminder(const int id); //and select it

    int get_current_reminder_id() const; //return -1 if no reminder is selected in the title list

signals:
    void to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void to_modify_reminder_spec(int id, const clReminderSpec *new_spec);

//--------------------

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void on_listWidget_reminders_currentItemChanged(QListWidgetItem *current,
                                                    QListWidgetItem *previous);
    void on_listWidget_reminders_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_edit_tags_clicked();
    void on_pushButton_edit_detail_clicked();
    void on_pushButton_edit_quickNote_clicked();
    void on_pushButton_edit_spec_alarm_buttons_clicked();

private:
    Ui::clUI_AllReminders *ui;

    // data (read only) //
    const QMap<int, const clReminder*> *pReminders;
    const QSet<QString> *pSituations;
    const QSet<QString> *pEvents;

    //
    QMap<int, QListWidgetItem*> mReminderID_to_Item; //items of `ui->listWidget_reminders`

    QMenu *mContextMenu_RemTitles;
    QAction *mAction_EditRemTitle;

    //
    void set_detail_view_contents(const clReminder *reminder);
    void clear_detail_view_contents();
    void set_records_table(const QMultiMap<QDateTime, QString> &records);

    void finish_editing_detail();
    void finish_editing_quickNote();
};

#endif // UI_ALL_REMINDERS_H
