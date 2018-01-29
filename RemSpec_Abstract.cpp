#include <QDebug>
#include "RemSpec_Abstract.h"

clRemSpec_Abstract::clRemSpec_Abstract(const int forth_scheduling_duration_minutes,
                                       QObject *parent)
    : QObject(parent),
      mForthSchedulingDuration(forth_scheduling_duration_minutes)
{
    Q_ASSERT(forth_scheduling_duration_minutes >= 1);
}

void clRemSpec_Abstract::clear()
{
    mActions.clear();
    mNextForthSchedulingTime = QDateTime();
}

void clRemSpec_Abstract::On_time_reached(const QDateTime &t)
{
    if(is_empty())
        return;

    QMultiMap<QDateTime,clAction> actions_to_perform;
    for(auto it=mActions.begin(); it!=mActions.end(); )
    {
        QDateTime t1 = it.key();
        if(t1 > t)
            break;

        actions_to_perform.insert(t1, it.value());

        //
        it = mActions.erase(it);
    }

    if(! actions_to_perform.isEmpty())
        perform_actions(actions_to_perform);

    //
    if(mNextForthSchedulingTime.isValid() && mNextForthSchedulingTime <= t)
    {
        QDateTime from = mNextForthSchedulingTime;
        mNextForthSchedulingTime = from.addSecs(mForthSchedulingDuration*60);
        forth_schedule(from);
    }
}

void clRemSpec_Abstract::start_forth_scheduling()
{
    if(is_forth_scheduling_started()) //(already started)
        return;

    //
    const QDateTime now = QDateTime::currentDateTime();
    mNextForthSchedulingTime = now.addSecs(mForthSchedulingDuration*60);
    forth_schedule(now);
}

void clRemSpec_Abstract::stop_forth_scheduling()
//remaining scheduled actions are NOT canceled
{
    if(! is_forth_scheduling_started()) //(already stopped)
        return;

    //
    mNextForthSchedulingTime = QDateTime();
}

bool clRemSpec_Abstract::is_forth_scheduling_started() const
{
    return mNextForthSchedulingTime.isValid();
}

QDateTime clRemSpec_Abstract::get_next_forth_scheduling_time() const
//forth-scheduling must have been started
{
    Q_ASSERT(mNextForthSchedulingTime.isValid());
    return mNextForthSchedulingTime;
}

void clRemSpec_Abstract::forth_schedule(const QDateTime &from)
//within (`from`, `mNextForthSchedulingTime`]
{
    Q_ASSERT(from < mNextForthSchedulingTime);

    mActions = get_actions_within_time_range(from, mNextForthSchedulingTime);
    foreach(QDateTime t, mActions.uniqueKeys())
    {
        Q_ASSERT(t.isValid());
        emit to_set_alarm(t);
    }

    if(mNextForthSchedulingTime.isValid())
    {
        if(! mActions.contains(mNextForthSchedulingTime))
            emit to_set_alarm(mNextForthSchedulingTime);
    }
}

void clRemSpec_Abstract::insert_actions_up_to_next_forth_scheduling_time(
                                                  const QMultiMap<QDateTime,clAction> &actions)
//Add actions from `actions` that is no later than `mNextForthSchedulingTime`.
//Forth-scheduling must have been started.
{
    Q_ASSERT(mNextForthSchedulingTime.isValid());
    for(auto it=actions.constBegin(); it!=actions.constEnd(); it++)
    {
        QDateTime t = it.key();
        Q_ASSERT(t.isValid());
        if(t > mNextForthSchedulingTime)
            break;

        if(! mActions.contains(t))
            emit to_set_alarm(t);
        mActions.insert(t, it.value());
    }
}

void clRemSpec_Abstract::cancel_actions()
//Cancel all currently scheduled actions, but forth-scheduling is NOT stopped.
{
    mActions.clear();
}

void clRemSpec_Abstract::cancel_actions_w_action_code(const int action_code)
//Cancel all currently scheduled actions with the action code.
{
    for(auto it=mActions.begin(); it!=mActions.end(); )
    {
        if(it.value().code == action_code)
            it = mActions.erase(it);
        else
            it++;
    }
}

void clRemSpec_Abstract::cancel_actions_w_source(const int source)
//Cancel all currently scheduled actions with the source.
{
    for(auto it=mActions.begin(); it!=mActions.end(); )
    {
        if(it.value().source == source)
            it = mActions.erase(it);
        else
            it++;
    }
}

void clRemSpec_Abstract::cancel_actions_w_action_code_et_source(const int action_code,
                                                                const int source)
//Cancel all currently scheduled actions with the action code and the source.
{
    for(auto it=mActions.begin(); it!=mActions.end(); )
    {
        if(it.value().source == source && it.value().code == action_code)
            it = mActions.erase(it);
        else
            it++;
    }
}

QMultiMap<QDateTime,QString> clRemSpec_Abstract::get_scheduled_actions() const
{
    QMultiMap<QDateTime,QString> actions;
    for(auto it=mActions.constBegin(); it!=mActions.constEnd(); it++)
    {
        QDateTime t = it.key();
        clAction a = it.value();
        QString s = get_action_name(a.code);
        actions.insert(t, s);
    }

    if(mNextForthSchedulingTime.isValid())
        actions.insert(mNextForthSchedulingTime, "forth-schedule");

    return actions;
}
