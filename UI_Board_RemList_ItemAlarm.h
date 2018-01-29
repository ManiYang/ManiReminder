#ifndef UI_BOARD_REM_LIST_ITEM_ALARM_H
#define UI_BOARD_REM_LIST_ITEM_ALARM_H

#include <QObject>
#include <QTimer>
#include "DataElem_Alarm.h"

namespace nsUI_Board_RemList {

class clItemAlarm : public QObject
{
    Q_OBJECT
public:
    enum enState { STOPPED, ACTIVE, PAUSED };

    explicit clItemAlarm(const clDataElem_ReminderAlarm &alarm_spec, QObject *parent = nullptr);
                         //`alarm_spec` should not be empty.
                         //Alarm will not be started.
    ~clItemAlarm();

    void start();   // becomes ACTIVE, effective on when the alarm is stopped
    void restart(); // becomes ACTIVE
    void delay();   // becomes PAUSED, effective only when the alarm is active
    void stop();    // becomes STOPPED

    enState get_state() const { return mState; }
    bool is_running() const { return mState==ACTIVE || mState==PAUSED; }
    bool is_active() const { return mState==ACTIVE; }

signals:
    void start_blinking(nsUI_Board_RemList::clItemAlarm *);
    void stop_blinking(nsUI_Board_RemList::clItemAlarm *);
    void start_sound(nsUI_Board_RemList::clItemAlarm *);
    void stop_sound(nsUI_Board_RemList::clItemAlarm *);

    void state_changed(nsUI_Board_RemList::clItemAlarm *,
                       nsUI_Board_RemList::clItemAlarm::enState new_state);

private slots:
    void On_timer_time_out();

private:
    clDataElem_ReminderAlarm mAlarmSpec;
    QTimer mTimer;

    enState mState; //stopped
                    //active -- blinking (and playing sound)
                    //paused -- not blinking or playing sound, but the timer is running

    void pause();
    void start_blinking_and_sound();
    void stop_blinking_and_sound();
};

} // namespace nsUI_Board_RemList

#endif // UI_BOARD_REM_LIST_ITEM_ALARM_H
