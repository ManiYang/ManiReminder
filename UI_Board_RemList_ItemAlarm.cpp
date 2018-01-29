#include "UI_Board_RemList_ItemAlarm.h"
using namespace nsUI_Board_RemList;

clItemAlarm::clItemAlarm(const clDataElem_ReminderAlarm &alarm_spec, QObject *parent)
    : QObject(parent), mState(STOPPED)
//`alarm_spec` should not be empty.
//Alarm will not be started.
{
    Q_ASSERT(! alarm_spec.is_empty());
    mAlarmSpec = alarm_spec;

    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(On_timer_time_out()));
}

clItemAlarm::~clItemAlarm()
{
    mTimer.stop();
}

void clItemAlarm::restart()
{
    mTimer.stop();
    mTimer.setInterval(mAlarmSpec.get_pause_after_sec()*1000);

    start_blinking_and_sound();
    mTimer.start();

    if(mState != ACTIVE)
    {
        mState = ACTIVE;
        emit state_changed(this, ACTIVE);
    }
}

void clItemAlarm::pause()
{
    mTimer.stop();
    mTimer.setInterval(mAlarmSpec.get_pause_interval_sec()*1000);

    stop_blinking_and_sound();
    mTimer.start();

    if(mState != PAUSED)
    {
        mState = PAUSED;
        emit state_changed(this, PAUSED);
    }
}

void clItemAlarm::start()
//effective on when the alarm is stopped
{
    if(mState == STOPPED)
        restart();
}

void clItemAlarm::delay()
//effective only when the alarm is active
{
    if(mState == ACTIVE)
        pause();
}

void clItemAlarm::stop()
{
    mTimer.stop();
    if(mState == ACTIVE)
        stop_blinking_and_sound();

    if(mState != STOPPED)
    {
        mState = STOPPED;
        emit state_changed(this, STOPPED);
    }
}

void clItemAlarm::On_timer_time_out()
{
    if(mState == ACTIVE)
        pause();
    else
    {
        Q_ASSERT(mState == PAUSED);
        restart();
    }
}

void clItemAlarm::start_blinking_and_sound()
{
    emit start_blinking(this);
    if(mAlarmSpec.play_sound())
        emit start_sound(this);
}

void clItemAlarm::stop_blinking_and_sound()
{
    emit stop_blinking(this);
    if(mAlarmSpec.play_sound())
        emit stop_sound(this);
}
