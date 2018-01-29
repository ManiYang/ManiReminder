#include <QList>
#include <QDateTime>
#include <QString>
#include "RemSpec_GEventDurationBinding.h"
#include "utility_DOM.h"
#include "utility_general.h"
#include "utility_time.h"

clRemSpec_GEventDurationBinding::clRemSpec_GEventDurationBinding(QObject *parent)
    : QObject(parent), mStartCount(0)
{
}

static bool parse_as_duration(const QString &S, int &duration_minute)
//'S' is expected to be like "10m", "20 m", "1h30m", "2 h 40 m".
{
    if(! S.endsWith('m'))
        return false;

    QString S1 = S.left(S.length()-1);
    QStringList tokens = S1.split('h');
    if(tokens.count()!=1 && tokens.count()!=2)
        return false;

    if(tokens.count() == 1)
    {
        bool ch;
        duration_minute = tokens.at(0).toInt(&ch);
        if(!ch)
            return false;
    }
    else
    {
        bool ch;
        duration_minute = tokens.at(0).toInt(&ch) * 60;
        if(!ch)
            return false;
        duration_minute += tokens.at(1).toInt(&ch);
        if(!ch)
            return false;
    }

    return true;
}

static bool parse_as_gevent_duration_binding(QDomElement &parent, clDataElem_GEvent &gevent,
                                             int &duration_minute, QString &err_msg)
//under 'parent':
//  <g-event> g-event-setting </g-event>
//  <duration> duration-setting </duration>
//duration-setting examples: "10m", "20 m", "1h 30m", "2 h 40 m"
{
    QStringList SL = nsDomUtil::get_texts_of_child_elements(parent, "g-event");
    if(SL.count() != 1)
    {
        err_msg = "Should have exactly 1 element with tag <g-event>.";
        return false;
    }
    QString gevent_setting = SL.at(0).simplified();

    SL = nsDomUtil::get_texts_of_child_elements(parent, "duration");
    if(SL.count() != 1)
    {
        err_msg = "Should have exactly 1 element with tag <duration>.";
        return false;
    }
    QString dur_setting = SL.at(0).simplified();

    //
    bool ch = gevent.parse_and_set(gevent_setting);
    if(!ch)
    {
        err_msg = QString("Could not parse \"%1\" as a g-event.").arg(gevent_setting);
        return false;
    }

    ch = parse_as_duration(dur_setting, duration_minute);
    if(!ch)
    {
        err_msg = QString("Could not parse \"%1\" as a duration setting.").arg(dur_setting);
        return false;
    }
    if(duration_minute <= 0)
    {
        err_msg = QString("Duration must be > 0.");
        return false;
    }

    return true;
}

bool clRemSpec_GEventDurationBinding::parse_and_set(QDomElement &spec_setting,
                                                    QString &parse_error_msg)
//Return false iff there's error in parsing.
{
    clear();

    //under `spec_setting`:
    //  <g-event-duration-binding> ... </g-event-duration-binding>
    //  <g-event-duration-binding> ... </g-event-duration-binding>
    //  ...
    for(int i=0; true; i++)
    {
        QDomElement elem = nsDomUtil::get_ith_child_element(spec_setting, i,
                                                         "g-event-duration-binding");
        if(elem.isNull())
            break;

        clDataElem_GEvent gevent;
        int dur_minute;
        QString err_msg;
        bool ch = parse_as_gevent_duration_binding(elem, gevent, dur_minute, err_msg);
        if(!ch)
        {
            parse_error_msg
                    = "Error in parsing g-event-duration-binding setting: " + err_msg;
            return false;
        }

        //
        if(mGEvents.contains(gevent))
        {
            parse_error_msg
                    = "Error in parsing g-event-duration-binding setting: "
                      "Duplicated g-event.";
            return false;
        }

        mGEvents << gevent;
        mDurations << dur_minute;
    }

    //
    return true;
}

void clRemSpec_GEventDurationBinding::initialize(const QDateTime &t0,
                                           const QMultiMap<QDateTime,clGEvent> &gevent_history)
//Set internal state to that at time `t0`.
//`gevent_history` should be the g-event record at `t0`.
{
    if(is_empty())
        return;

    //
    int dt_max = 0; //minutes
    for(int i=0; i<mGEvents.size(); i++)
    {
        if(mDurations.at(i) > dt_max)
            dt_max = mDurations.at(i);
    }

    //
    mStartCount = 0;
    mEnds.clear();

    QDateTime t1 = t0.addSecs(-dt_max*60 - 1);
    QMultiMap<QDateTime,clGEvent>::const_iterator it = gevent_history.upperBound(t1);
    for( ; it != gevent_history.constEnd(); it++)
    {
        QDateTime t = it.key();
        clGEvent gevent = it.value();

        int i = mGEvents.indexOf(gevent);
        if(i == -1)
            continue;

        int dt = mDurations.at(i);
        QDateTime t_end = t.addSecs(dt*60);
        if(t0 < t_end)
        {
            mStartCount++;
            mEnds << t_end;
            emit to_set_alarm(t_end);
        }
    }
}

void clRemSpec_GEventDurationBinding::update_settings(
                                          const clRemSpec_GEventDurationBinding &another,
                                          const QDateTime &t0,
                                          const QMultiMap<QDateTime,clGEvent> &gevent_history)
{
    clear();
    mGEvents = another.mGEvents;
    mDurations = another.mDurations;
    initialize(t0, gevent_history);
}

bool clRemSpec_GEventDurationBinding::update_settings(
                                       QList<std::tuple<clGEvent, int, int> > gevents_hrs_mins,
                                       const QDateTime &t0,
                                       const QMultiMap<QDateTime,clGEvent> &gevent_history,
                                       QString &parse_error_msg)
//returns false iff there's error
{
    clear();

    // set settings
    for(int i=0; i<gevents_hrs_mins.size(); i++)
    {
        clGEvent gevent = std::get<0>(gevents_hrs_mins.at(i));
        int hr = std::get<1>(gevents_hrs_mins.at(i));
        int min = std::get<2>(gevents_hrs_mins.at(i));

        if(mGEvents.contains(gevent))
        {
            parse_error_msg = "Error: Duplicated g-event.";
            return false;
        }
        if(hr < 0 || min < 0)
        {
            parse_error_msg = "Error: Negative value of hours or minutes.";
            return false;
        }

        mGEvents << gevent;
        mDurations << hr*60 + min;
    }

    //
    initialize(t0, gevent_history);

    //
    return true;
}

void clRemSpec_GEventDurationBinding::clear()
{
    mGEvents.clear();
    mDurations.clear();
    mStartCount = 0;
    mEnds.clear();
}

//////

void clRemSpec_GEventDurationBinding::On_gevent(const clGEvent &gevent)
{
    int pos = mGEvents.indexOf(gevent);
    if(pos != -1) //('gevent' found in mGEvents[])
    {
        //int pos2 = mGEvents.indexOf(gevent, pos+1);
        //Q_ASSERT(pos2 == -1); //should not have duplicated elements in mGEvents[]

        //
        mStartCount++;

        //
        QDateTime now = QDateTime::currentDateTime();
        QDateTime end = now.addSecs(mDurations.at(pos)*60);
        mEnds << end;
        emit to_set_alarm(end);
    }
}

void clRemSpec_GEventDurationBinding::On_time_reached(const QDateTime &t)
{
    for(auto it=mEnds.begin(); it!=mEnds.end(); )
    {
        if(*it <= t)
        {
            it = mEnds.erase(it);
            mStartCount--;
            Q_ASSERT(mStartCount >= 0);
        }
        else
            it++;
    }
}

QString clRemSpec_GEventDurationBinding::print() const
{
    QString S = "event-duration binding: ";
    for(int i=0; i<mGEvents.count(); i++)
    {
        if(i > 0)
            S += ", ";
        S += mGEvents.at(i).print();
        S += " | ";
        S += QString::number(mDurations.at(i))+" min";
    }
    return S;
}

void clRemSpec_GEventDurationBinding::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    for(int i=0; i<mGEvents.size(); i++)
    {
        QDomElement elem0
                = nsDomUtil::add_child_element(doc, parent, "g-event-duration-binding");
        nsDomUtil::add_child_element_w_text(doc, elem0,
                                            "g-event", mGEvents[i].print());
        nsDomUtil::add_child_element_w_text(doc, elem0,
                                            "duration", print_xhxm(mDurations[i]));
    }
}

QSet<QString> clRemSpec_GEventDurationBinding::get_situations_involved() const
{
    QList<QString> sits;
    foreach(clDataElem_GEvent gevent, mGEvents)
    {
        if(gevent.is_start_of_situation() || gevent.is_end_of_situation())
            sits << gevent.get_name();
    }

    return QSet<QString>::fromList(sits);
}

QSet<QString> clRemSpec_GEventDurationBinding::get_events_involved() const
{
    QList<QString> events;
    foreach(clDataElem_GEvent gevent, mGEvents)
    {
        if(gevent.is_event())
            events << gevent.get_name();
    }

    return QSet<QString>::fromList(events);
}

bool clRemSpec_GEventDurationBinding::operator == (
                                         const clRemSpec_GEventDurationBinding &another) const
//compare settings only
{
    int n = mGEvents.size();
    if(another.mGEvents.size() != n)
        return false;

    for(int i=0; i<n; i++)
    {
        int p = another.mGEvents.indexOf(mGEvents[i]);
        if(p == -1)
            return false;

        if(another.mDurations[p] != mDurations[i])
            return false;
    }

    return true;
}

void clRemSpec_GEventDurationBinding::operator =(const clRemSpec_GEventDurationBinding &another)
//Clear and copy settings only.
//[You can call `initialize(...)` to let `*this` start working properly,
// but all connections must be built beforehand.]
{
    clear();
    mGEvents = another.mGEvents;
    mDurations = another.mDurations;
}

QMultiMap<QDateTime,QString> clRemSpec_GEventDurationBinding::get_scheduled_actions() const
{
    QMultiMap<QDateTime,QString> actions;
    foreach(QDateTime t , mEnds)
        actions.insert(t,"end");
    return actions;
}
