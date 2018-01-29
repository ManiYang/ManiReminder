#include <QDebug>
#include "RemSpec_GEventTrigger.h"
#include "utility_DOM.h"
typedef clDataElem_GEvent clGEvent;
typedef clDataElem_TimeSequence clTimeSequence;

const int ForthSchedulingDuration = 57; //(minute)

clRemSpec_GEventTrigger::clRemSpec_GEventTrigger(QObject *parent)
    : clRemSpec_Abstract(ForthSchedulingDuration, parent)
{
}

static bool parse_one(const QString &S, clGEvent &triggering_gevent, clTimeSequence &time_seq,
                      clGEvent &closing_gevent, QString &parse_error_msg)
//format:
//  triggering-g-event[; time-sequence][; until closing-g-event]
{
    QStringList tokens = S.split(';');
    if(tokens.count() < 0 || tokens.count() > 3)
    {
        parse_error_msg = "";
        return false;
    }

    // triggering g-event
    bool ch = triggering_gevent.parse_and_set(tokens.at(0));
    if(!ch)
    {
        parse_error_msg = QString("Could not parse \"%1\" as a g-event.").arg(tokens.at(0));
        return false;
    }

    // time sequence & closing g-event
    if(tokens.count() == 3)
    {
        ch = time_seq.parse_and_set(tokens.at(1));
        if(!ch)
        {
            parse_error_msg = QString("Could not parse \"%1\" as a time sequence.")
                              .arg(tokens.at(1));
            return false;
        }

        //
        QString str = tokens.at(2).simplified();
        if(! str.startsWith("until "))
        {
            parse_error_msg = "The setting for closing g-event must start with \"until \".";
            return false;
        }
        str = str.mid(6);
        ch = closing_gevent.parse_and_set(str);
        if(!ch)
        {
            parse_error_msg = QString("Could not parse \"%1\" as a g-event.").arg(str);
            return false;
        }
    }
    else if(tokens.count() == 1)
        time_seq.set(0, 1, 1); //no delay, single instant
        //no closing g-event
    else //(tokens.count() == 2)
    {
        QString str1 = tokens.at(1).simplified();
        if(str1.startsWith("until "))
        {
            time_seq.set(0, 1, 1); //no delay, single instant

            //
            str1 = str1.mid(6);
            ch = closing_gevent.parse_and_set(str1);
            if(!ch)
            {
                parse_error_msg = QString("Could not parse \"%1\" as a g-event.").arg(str1);
                return false;
            }
        }
        else
        {
            ch = time_seq.parse_and_set(str1);
            if(!ch)
            {
                parse_error_msg = QString("Could not parse \"%1\" as a time sequence.")
                                  .arg(str1);
                return false;
            }
            //no closing g-event
        }
    }

    //
    if(time_seq.get_t_start() < 0)
    {
        parse_error_msg = "The start time of time sequence must be >= 0.";
        return false;
    }

    //
    return true;
}

bool clRemSpec_GEventTrigger::parse_and_set(QDomElement &spec_setting, QString &parse_error_msg)
{
    //find under `spec_setting` occurrences of
    //  <g-event-trigger>
    //     triggering-g-event[; time-sequence][; until closing-g-event]
    //  </g-event-trigger>
    QStringList SL = nsDomUtil::get_texts_of_child_elements(spec_setting, "g-event-trigger");
    foreach(QString S, SL)
    {
        clGEvent triggering_gevent;
        clTimeSequence time_seq;
        clGEvent closing_gevent;

        bool ch = parse_one(S, triggering_gevent, time_seq, closing_gevent, parse_error_msg);
        if(!ch)
            return false;

        //
        mTriggeringGEvents << triggering_gevent;
        mTimeSequences << time_seq;
        mClosingGEvents << closing_gevent;
    }

    //
    return true;
}

static QDateTime find_last_occurence(const QMap<QDateTime,clGEvent> &gevents,
                                     const clGEvent target)
//return null date-time if not found
{
    QDateTime t;
    for(auto it=gevents.constBegin(); it!=gevents.constEnd(); it++)
    {
        if(it.value() == target)
            t = it.key();
    }
    return t;
}

void clRemSpec_GEventTrigger::initialize(const QDateTime &t0,
                                         const QMultiMap<QDateTime,clGEvent> &gevent_history)
//Set internal state to that at time `t0`.
//`gevent_history` should be the g-event record at `t0`.
{
    if(is_empty())
        return;

    // add items to `mActiveGEvents[]` according to `gevent_history`
    for(int i=0; i<mTriggeringGEvents.size(); i++)
    {
        clGEvent triggering_gevent = mTriggeringGEvents.at(i);
        QDateTime t = gevent_history.key(triggering_gevent, QDateTime());
        if(t.isNull())
            continue;

        // time at last occurence
        t = find_last_occurence(gevent_history, triggering_gevent);

        // closing event occured after `t`?
        bool ended = false;
        if(! mClosingGEvents.at(i).is_empty())
        {
            QDateTime t1 = find_last_occurence(gevent_history, mClosingGEvents.at(i));
            if(t1.isValid())
            {
                if(t1 > t)
                    ended = true;
            }
        }
        if(ended)
            continue;

        // `t0` is beyond lastest time?
        if(! mTimeSequences.at(i).is_infinite())
        {
            if(mTimeSequences.at(i).get_latest_time(t) < t0)
                ended = true;
        }

        //
        if(! ended) //(the g-event is still active)
            mActiveGEvents.insert(i, t);
    }

    //
    if(! mActiveGEvents.isEmpty())
        start_forth_scheduling();
}

void clRemSpec_GEventTrigger::update_settings(const clRemSpec_GEventTrigger &another,
                                              const QDateTime &t0,
                                              const QMap<QDateTime,clGEvent> &gevent_history)
{
    clear();
    mTriggeringGEvents = another.mTriggeringGEvents;
    mTimeSequences = another.mTimeSequences;
    mClosingGEvents = another.mClosingGEvents;
    initialize(t0, gevent_history);
}

bool clRemSpec_GEventTrigger::update_settings(const QStringList &gevent_trigger_settings,
                                              const QDateTime &t0,
                                              const QMap<QDateTime,clGEvent> &gevent_history,
                                              QString &parse_error_msg)
{
    clear(); //all scheduled actions and forth-scheduling are canceled

    foreach(QString S, gevent_trigger_settings)
    {
        clGEvent triggering_gevent;
        clTimeSequence time_seq;
        clGEvent closing_gevent;

        bool ch = parse_one(S, triggering_gevent, time_seq, closing_gevent, parse_error_msg);
        if(!ch)
            return false;

        //
        mTriggeringGEvents << triggering_gevent;
        mTimeSequences << time_seq;
        mClosingGEvents << closing_gevent;
    }

    //
    initialize(t0, gevent_history);

    //
    return true;
}

QString clRemSpec_GEventTrigger::print() const
{
    QString S;
    if(is_empty())
        return S;

    //
    for(int i=0; i<mTriggeringGEvents.count(); i++)
    {
        if(! S.isEmpty())
            S += '\n';

        S += "g-event-trigger: ";

        S += mTriggeringGEvents[i].print();

        if(mTimeSequences[i].get_Ntimes() != 1)
            S += "; " + mTimeSequences[i].print();

        if(! mClosingGEvents[i].is_empty())
            S += "; until " + mClosingGEvents[i].print();
    }

    return S;
}

void clRemSpec_GEventTrigger::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    for(int i=0; i<mTriggeringGEvents.size(); i++)
    {
        QString S;
        S += mTriggeringGEvents[i].print();
        if(mTimeSequences[i].get_Ntimes() != 1)
            S += "; " + mTimeSequences[i].print();
        if(! mClosingGEvents[i].is_empty())
            S += "; until " + mClosingGEvents[i].print();

        nsDomUtil::add_child_element_w_text(doc, parent, "g-event-trigger", S);
    }
}

QSet<QString> clRemSpec_GEventTrigger::get_situations_involved() const
{
    QSet<QString> sits;
    for(int i=0; i<mTriggeringGEvents.count(); i++)
    {
        if(mTriggeringGEvents[i].is_start_of_situation()
                || mTriggeringGEvents[i].is_end_of_situation())
            sits.insert(mTriggeringGEvents[i].get_name());

        if(mClosingGEvents[i].is_start_of_situation()
                || mClosingGEvents[i].is_end_of_situation())
            sits.insert(mClosingGEvents[i].get_name());
    }

    return sits;
}

QSet<QString> clRemSpec_GEventTrigger::get_events_involved() const
{
    QSet<QString> events;
    for(int i=0; i<mTriggeringGEvents.count(); i++)
    {
        if(mTriggeringGEvents[i].is_event())
            events.insert(mTriggeringGEvents[i].get_name());

        if(mClosingGEvents[i].is_event())
            events.insert(mClosingGEvents[i].get_name());
    }

    return events;
}

void clRemSpec_GEventTrigger::clear()
{
    mTriggeringGEvents.clear();
    mTimeSequences.clear();
    mClosingGEvents.clear();

    mActiveGEvents.clear();
    clRemSpec_Abstract::clear();
}

void clRemSpec_GEventTrigger::On_gevent(const clDataElem_GEvent &gevent)
{
    if(is_empty())
        return;

    const QDateTime now = QDateTime::currentDateTime();

    for(int i=0; i<mTriggeringGEvents.size(); i++)
    {
        /// if `gevent` is ith triggering g-event
        if(gevent == mTriggeringGEvents[i])
        {
            if(mTimeSequences[i].get_t_start() == 0
               && mTimeSequences[i].get_Ntimes() == 1) //(immediate and one-time)
            {
                Q_ASSERT(! mActiveGEvents.contains(i)); //should not be active
                emit to_trigger();
                continue;
            }

            // (now `mTimeSequences[i]` is not immediate or is not single time)
            if(mActiveGEvents.contains(i))  //(`mTriggeringGEvents[i]` is already active)
                cancel_actions_w_source(i); //cancel all scheduled actions with source `i`
            mActiveGEvents[i] = now;

            //
            if(mTimeSequences[i].get_t_start() == 0) //(immediate)
                emit to_trigger();

            //
            if(! is_forth_scheduling_started())
                start_forth_scheduling();
            else
            {
                // get triggering times up to next forth-scheduling time
                const QDateTime t_next_forth_scheduling = get_next_forth_scheduling_time();
                QList<QDateTime> trigger_times
                    = mTimeSequences[i].get_times_within_time_range(now, now.addSecs(-1),
                                                                    t_next_forth_scheduling);

                // insert actions
                QMultiMap<QDateTime,clAction> actions;
                foreach(QDateTime t, trigger_times)
                {
                    clAction a(0, i);
                    actions.insert(t, a);
                }
                insert_actions_up_to_next_forth_scheduling_time(actions);
            }
        }

        /// if `gevent` is the closing g-event for ith triggering g-event
        if(gevent == mClosingGEvents[i])
        {
            // cancel all scheduled actions due to ith triggering g-event
            cancel_actions_w_source(i);

            // remove `i`th triggering event from `mActiveGEvents[]`
            mActiveGEvents.remove(i);

            //
            if(mActiveGEvents.isEmpty())
                stop_forth_scheduling();
        }
    }
}

bool clRemSpec_GEventTrigger::operator == (const clRemSpec_GEventTrigger &another) const
//compare settings only
{
    const int n = mTriggeringGEvents.size();
    if(another.mTriggeringGEvents.size() != n)
        return false;

    for(int i=0; i<n; i++)
    {
        clGEvent triggering_gevent = mTriggeringGEvents.at(i);

        bool found = false;
        int pos = 0;
        while(pos < n && !found)
        {
            pos = another.mTriggeringGEvents.indexOf(triggering_gevent, pos);
            if(pos == -1)
                break;

            //
            if(another.mTimeSequences.at(pos) == mTimeSequences.at(i)
               && another.mClosingGEvents.at(pos) == mClosingGEvents.at(i))
                found = true;

            //
            pos++;
        }

        //
        if(! found)
            return false;
    }
    return true;
}

void clRemSpec_GEventTrigger::operator = (const clRemSpec_GEventTrigger &another)
//clear and copy settings
{
    clear();
    mTriggeringGEvents = another.mTriggeringGEvents;
    mTimeSequences = another.mTimeSequences;
    mClosingGEvents = another.mClosingGEvents;
}

void clRemSpec_GEventTrigger::perform_actions(const QMultiMap<QDateTime,clAction> &actions)
{
    for(int j=0; j<actions.count(); j++)
        emit to_trigger();
}

QMultiMap<QDateTime,clAction> clRemSpec_GEventTrigger::get_actions_within_time_range(
                                                                         const QDateTime &t0,
                                                                         const QDateTime &t1)
//within (t0,t1]
//Erase `mActiveGEvents[i]` if the ith triggering event has no triggering time > t0.
//Stop forth scheduling if `mActiveGEvents[]` becomes empty.
{
    QMultiMap<QDateTime,clAction> actions;

    if(is_empty())
        return actions;

    //
    for(auto it=mActiveGEvents.begin(); it!=mActiveGEvents.end(); )
    {
        const int index = it.key();
        const QDateTime t_happen = it.value();

        // if `mTimeSequences[index]` is finite and has no instants > `t0`
        if(! mTimeSequences[index].is_infinite())
        {
            QDateTime latest = mTimeSequences[index].get_latest_time(t_happen);
            if(t0 >= latest)
            {
                it = mActiveGEvents.erase(it); //remove `it`
                continue;
            }
        }

        //
        QList<QDateTime> trigger_times
                = mTimeSequences[index].get_times_within_time_range(t_happen, t0, t1);
        foreach(QDateTime t, trigger_times)
        {
            clAction a(0, index);
            actions.insert(t, a);
        }

        //
        it++;
    }

    //
    if(mActiveGEvents.isEmpty())
        stop_forth_scheduling();

    return actions;
}

QString clRemSpec_GEventTrigger::get_action_name(const int action_code) const
{
    Q_UNUSED(action_code);
    return "trigger";
}
