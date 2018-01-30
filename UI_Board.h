#ifndef UI_BOARD_H
#define UI_BOARD_H

#include <QWidget>
#include <QMap>
#include <QListWidget>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QPlainTextEdit>
#include "Reminder.h"
#include "DataElem_GEvent.h"
#include "UI_Board_RemList.h"
#include "UI_Board_ButtonSetsView.h"

namespace Ui {
class clUI_Board;
}

class clAlternateBSInfo;

////////

class clUI_Board : public QWidget
{
    Q_OBJECT

public:
    explicit clUI_Board(const QMap<int, const clReminder*> *p_reminders,
                        const QSet<QString> *p_situations, const QSet<QString> *p_events,
                        QWidget *parent = nullptr);
    ~clUI_Board();

    void show_reminder(const int id); //activate reminder
    void hide_reminder(const int id); //deactivate reminder

public slots:
    void reminder_updated(int id);
    void situation_event_list_updated();

signals:
    void gevent_happened(const clDataElem_GEvent &gevent, const QDateTime &at);
    void to_modify_reminder(int id, int modifyFlags, const clReminder *new_data);
    void to_add_reminder_record(int id, const QDateTime &t, const QString &log_text);
//    void to_MainWindow__reminder_modified(int rem_id);

    void to_show_reminder_in_tab_all_reminders(int id);

//--------------------------

private slots:
    // situation and events
    void on_listWidget_selected_situations_customContextMenuRequested(const QPoint &pos);
    void on_listWidget_all_situations_et_events_itemDoubleClicked(QListWidgetItem *item);

    void update_history_time_display();

    // reminder list
    void On_RemList_current_reminder_changed(int new_rem_id,
                                             clUI_Board_RemList::enAlarmState alarm_state);
    void On_RemList_current_reminder_alarm_state_changed(
                                  int rem_id, clUI_Board_RemList::enAlarmState new_alarm_state);
                                  //not on changes of current reminder in the reminder list
    void go_to_tab_all_reminders();

    // detail view
    void On_user_delay_alarm();

    void On_user_clicked_a_rem_button();
    void On_user_close_reminder();
    void to_add_record(QString log_text);

    void on_pushButton_time_stamp_clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::clUI_Board *ui;

    // data (read only) //
    const QMap<int, const clReminder*> *pReminders;
    const QSet<QString> *pSituations;
    const QSet<QString> *pEvents;

    // internal data //
    QMap<int, QMultiMap<QDateTime, QString> > mRemRecentRecords; //[active_rem_id]
    QMap<int, clAlternateBSInfo*> mAlternateBSInfos; //[active_rem_id]

    // units //
    QMenu *mContextMenu_for_SelectedSituations;
    QAction *mAction_EndSituation;
    QTimer mTimer; //for updating the time display in history

    clUI_Board_RemList *mRemList; //the list widget of reminder titles
    QAction *mAction_GoToAllReminders;

    QMenu *mMenu_for_alarm_button;
    QAction *mAction_DelayAlarm;

    clUI_Board_ButtonSetsView *mButtonSetsView;

    //
    void update_detail_view(const int rem_id);
                            //Fill the detail view with the data of `rem_id`.
                            //If `rem_id` < 0, clear the detail view.

    void add_to_history(const clDataElem_GEvent &gevent, const QDateTime &t);

    void update_alarm_button(const clReminder *reminder,
                             clUI_Board_RemList::enAlarmState alarm_state);

    void deploy_button_sets(int rem_id); //deploy the button sets of `rem_id`

    void update_recent_records_view(const QMultiMap<QDateTime,QString> &recent_records);
    void add_to_recent_records_view(const QDateTime &t, const QString &log_text);

    void save_quick_note();
};

////////////////////////////////////////////////////////////////////////////////////

class clAlternateBSInfo //for one reminder
{
public:
    explicit clAlternateBSInfo(const clReminder *reminder);
    ~clAlternateBSInfo();

    void update(const clReminder *reminder);
    int *get_state_register(const int BSNo) const;
                            //return `nullptr` if `BSNo` is not an alternate button set

private:
    QMap<int,int> mBSNo_to_index; //index of `mBSData[]` and `mStateRegisters[]`
    QList<clDataElem_ButtonSet_Alternate> mBSData;
    QList<int*> mStateRegisters;

    void clear();
    void set(const clReminder *reminder);
         //Set internal data according to `reminder`. All states will be set as 0.
};

#endif // UI_BOARD_H
