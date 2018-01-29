#ifndef UI_BOARD_REM_LIST_H
#define UI_BOARD_REM_LIST_H

#include <QListWidget>
#include <QColor>
#include <QSet>
#include <QTimer>
#include <QFont>
#include <QSoundEffect>
#include "DataElem_Alarm.h"
#include "UI_Board_RemList_ItemAlarm.h"

namespace nsUI_Board_RemList {
class clItemAlarm;
class clSoundPlayer;
}

class clUI_Board_RemList : public QListWidget
{
    Q_OBJECT
public:
    enum enAlarmState { NoAlarm, AlarmStopped, AlarmActive, AlarmPaused };

    explicit clUI_Board_RemList(QWidget *parent = nullptr);
    ~clUI_Board_RemList();

    void add_reminder_title(const int rem_id,
                            const QString &rem_title, const clDataElem_ReminderAlarm &alarm_spec);
                            //Add the reminder if it is not already added.
                            //Also select the reminder.
                            //Alarm will not be started.
    void remove_reminder_title(const int rem_id);
    bool contains_reminder_title(const int rem_id) const;
    void update_reminder_title(const int rem_id, const QString &new_title);

    int get_current_reminder_id() const; //returns -1 if not found

    //
    void start_alarm_on_reminder(const int rem_id);
                                 //If the alarm is already started, restart it.
    void delay_alarm_on_reminder(const int rem_id); //effective only if the alarm is active
    void stop_alarm_on_reminder(const int rem_id);

    bool alarm_running_on_reminder(const int rem_id) const; //?????
         //Return true iff `rem_id` has alarm and the alarm is active or paused (not stopped).

signals:
    void current_reminder_changed(int new_rem_id, clUI_Board_RemList::enAlarmState alarm_state);
    void current_reminder_alarm_state_changed(int rem_id,
                                              clUI_Board_RemList::enAlarmState new_alarm_state);
                                              //this will NOT be emitted when current reminder
                                              //changes

//---------------

private slots:
    void On_current_item_changed(QListWidgetItem *item_current, QListWidgetItem *item_prev);

    void On_timer_blink_time_out();

    void start_blinking_item(nsUI_Board_RemList::clItemAlarm *by_item_alarm);
    void stop_blinking_item(nsUI_Board_RemList::clItemAlarm *by_item_alarm);
    void start_playing_sound(nsUI_Board_RemList::clItemAlarm *by_item_alarm);
    void stop_playing_sound(nsUI_Board_RemList::clItemAlarm *by_item_alarm);
    void On_item_alarm_state_changed(nsUI_Board_RemList::clItemAlarm *by_item_alarm,
                                     nsUI_Board_RemList::clItemAlarm::enState new_state);

private:
    QMap<int, QListWidgetItem*> mReminderIDtoItem;
    QMap<QListWidgetItem*, nsUI_Board_RemList::clItemAlarm*> mItemsWithAlarm;

    QSet<QListWidgetItem*> mBlinkingItems;
    QSet<QListWidgetItem*> mSoundPlayingItems;
      // These are modified only by the private slots `start_blinking_item()`,
      // `stop_blinking_item()`, `start_playing_sound()`, and `stop_playing_sound()`.

    //
    QTimer mTimer_blink; //controls blinking

    QColor mNormalTextColor;
    QString mAlternateTextColor;
    QFont mNormalFont;
    bool m_blink_state;
    bool m_blinks_selected_item; //true => current selected item blinks
    void set_highlighted_text_color(bool alternate_color);

    //
    nsUI_Board_RemList::clSoundPlayer *mSoundPlayer;
};


///////////////////////////////////////////////////////////////////////////////////////////////

namespace nsUI_Board_RemList {

class clSoundPlayer : public QObject
{
    Q_OBJECT
public:
    explicit clSoundPlayer(QObject *parent = nullptr);

    void play(); //restart if already playing
    void stop();

    bool is_playing() const;

private slots:
    void On_timer_time_out();
    void On_sound_playing_changed();

private:
    QTimer mTimer;
    QSoundEffect mSound;
    int mGap; //(msec) pause interval between repeats
};

} // nsUI_Board_RemList

#endif // UI_BOARD_REM_LIST_H
