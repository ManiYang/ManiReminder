#include <QDebug>
#include "RemSpec_TimeRangeBinding.h"
#include "utility_DOM.h"

const int FORTH_SCHEDULING_DURATION = 59; //(minute)

clRemSpec_TimeRangeBinding::clRemSpec_TimeRangeBinding(QObject *parent)
    : clRemSpec_Abstract(FORTH_SCHEDULING_DURATION, parent)
{
    mState = false;
}

/*
clRemSpec_TimeRangeBinding::clRemSpec_TimeRangeBinding
                       (const QList<clDataElement_TimeRangeCollection> &time_range_collections,
                        QObject *parent)
    : clRemSpec_Abstract(FORTH_SCHEDULING_DURATION, parent)
{
    foreach(clDataElement_TimeRangeCollection TRC, time_range_collections)
    {
        if(! TRC.is_empty())
            mTimeRangeCollections << TRC;
    }

    if(! mTimeRangeCollections.isEmpty())
    {
        start_forth_scheduling();
        determine_state(QDateTime::currentDateTime());
    }
}
*/

bool clRemSpec_TimeRangeBinding::parse_and_set(QDomElement &spec_setting,
                                               QString &parse_error_msg)
//Returns false iff there is error in parsing.
{
    clear();

    //directly under 'spec_setting':
    //  <time-range-binding> time-range collection 1 </time-range-binding>
    //  <time-range-binding> time-range collection 2 </time-range-binding>
    //  ...
    QStringList SL
            = nsDomUtil::get_texts_of_child_elements(spec_setting, "time-range-binding");
    foreach(QString TRC_setting, SL)
    {
        clDataElem_TimeRangeCollection TRC;
        bool ch = TRC.parse_and_set(TRC_setting);
        if(!ch)
        {
            parse_error_msg
                    = QString("Error in parsing time-range-binding setting \"%1\".")
                      .arg(TRC_setting);
            return false;
        }

        if(! TRC.is_empty())
            mTimeRangeCollections << TRC;
    }

    //
    return true;
}

void clRemSpec_TimeRangeBinding::initialize()
//also start forth scheduling if `*this` is not empty
{
    if(is_empty())
        mState = false;
    else
    {
        start_forth_scheduling(); //this goes first
        determine_state(QDateTime::currentDateTime());
    }
}

void clRemSpec_TimeRangeBinding::update_settings(const clRemSpec_TimeRangeBinding &another)
{
    clear();
    mTimeRangeCollections = another.mTimeRangeCollections;
    initialize();
}

bool clRemSpec_TimeRangeBinding::update_settings(
                                         const QStringList &time_range_collection_settings,
                                         QString &parse_error_msg)
//returns false iff there's error.
{
    clear(); //all scheduled actions and forth-scheduling are canceled

    foreach(QString TRB_setting, time_range_collection_settings)
    {
        clDataElem_TimeRangeCollection TRC;
        bool ch = TRC.parse_and_set(TRB_setting);
        if(!ch)
        {
            parse_error_msg
                    = QString("Error in parsing time-range-binding setting \"%1\".")
                      .arg(TRB_setting);
            return false;
        }

        if(! TRC.is_empty())
            mTimeRangeCollections << TRC;
    }

    //
    initialize();

    //
    return true;
}

void clRemSpec_TimeRangeBinding::clear()
{
    mTimeRangeCollections.clear();
    mState = false;
    clRemSpec_Abstract::clear();
}

QString clRemSpec_TimeRangeBinding::print() const
{
    QString S;
    for(int i=0; i<mTimeRangeCollections.count(); i++)
    {
        if(i > 0)
            S += '\n';
        S += "time-range binding: ";
        S += mTimeRangeCollections.at(i).print();
    }
    return S;
}

void clRemSpec_TimeRangeBinding::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    for(int i=0; i<mTimeRangeCollections.size(); i++)
    {
        nsDomUtil::add_child_element_w_text(doc, parent, "time-range-binding",
                                            mTimeRangeCollections[i].print());
    }
}

void clRemSpec_TimeRangeBinding::perform_actions(const QMultiMap<QDateTime,clAction> &actions)
{
    for(auto it=actions.constBegin(); it!=actions.constEnd(); it++)
    {
        switch(it.value().code)
        {
        case Activate:
            mState = true;
            break;

        case Deactivate:
            mState = false;
            break;
        }
    }
}

QMultiMap<QDateTime,clAction> clRemSpec_TimeRangeBinding::get_actions_within_time_range(
                                                                        const QDateTime &t0,
                                                                        const QDateTime &t1)
//within (t0,t1]
{
    QMultiMap<QDateTime,bool> state_shifts;
    int count_at_t0 = 0;
    foreach(clDataElem_TimeRangeCollection TRC, mTimeRangeCollections)
    {
        bool state0;
        QMap<QDateTime,bool> shifts;
        TRC.state_shifts_within_time_range(t0, t1, 'U', &state0, &shifts); //(t0,t1]

        if(state0)
            count_at_t0++;
        state_shifts.unite(shifts);
    }

    //
    QMap<QDateTime,int> counts; //counts[t] is the count after t before next t
    int cnt = count_at_t0;
    for(auto it=state_shifts.begin(); it!=state_shifts.end(); it++)
    {
        if(it.value())
            cnt++;
        else
            cnt--;

        QDateTime t = it.key();
        counts[t] = cnt; //replace if `counts` already contains `t`
    }

    //
    QMultiMap<QDateTime,clAction> actions;
    cnt = count_at_t0;
    for(auto it=counts.begin(); it!=counts.end(); it++)
    {
        int new_cnt = it.value();

        if(cnt == 0 && new_cnt > 0)
            actions.insert(it.key(), clAction(Activate));
        else if(cnt > 0 && new_cnt == 0)
            actions.insert(it.key(), clAction(Deactivate));

        //
        cnt = new_cnt;
    }

    return actions;
}

QString clRemSpec_TimeRangeBinding::get_action_name(const int action_code) const
{
    if(action_code == Activate)
        return "activate";
    else if(action_code == Deactivate)
        return "deactivate";

    return "unknown";
}

void clRemSpec_TimeRangeBinding::determine_state(const QDateTime &at)
{
    mState = false;
    foreach(clDataElem_TimeRangeCollection TRC, mTimeRangeCollections)
    {
        if(TRC.includes(at))
        {
            mState = true;
            break;
        }
    }
}

void clRemSpec_TimeRangeBinding::operator =(const clRemSpec_TimeRangeBinding &another)
//Clear and copy settings only.
//[You can call `initialize()` to let `*this` start working, but all
// connections must be built properly beforehand.]
{
    clear();
    mTimeRangeCollections = another.mTimeRangeCollections;
}

bool clRemSpec_TimeRangeBinding::operator ==(const clRemSpec_TimeRangeBinding &another) const
//compare settings only
{
    return mTimeRangeCollections == another.mTimeRangeCollections;
}

QList<clUtil_HrMinRange> clRemSpec_TimeRangeBinding::get_time_ranges_on_date(
                                                                      const QDate &date) const
//Get time ranges starting on date `date`.
//Returned list will be separated and in ascending order.
{
    if(is_empty())
        return QList<clUtil_HrMinRange>();

    //
    QList<clUtil_HrMinRange> hrmin_ranges;
    for(int i=0; i<mTimeRangeCollections.size(); i++)
    {
        clDataElem_TimeRangeCollection TRC = mTimeRangeCollections.at(i);
        if(TRC.includes_date(date))
            hrmin_ranges << TRC.get_hrmin_ranges();
    }

    // simplify
    QList<clUtil_TimeRange> t_ranges;
    QDateTime latest_t = QDateTime(date, QTime(0,0));
    for(int i=0; i<hrmin_ranges.size(); i++)
    {
        clUtil_TimeRange t_range = hrmin_ranges.at(i).get_DateTime_range(date);
        t_ranges << t_range;

        if(t_range.right() > latest_t)
            latest_t = t_range.right();
    }

    bool state_at_beginning;
    QMap<QDateTime,bool> state_shifts;
    find_state_shifts_of_time_range_collection(t_ranges, 'u',
                                               QDateTime(date, QTime(0,0)), latest_t,
                                               &state_at_beginning, &state_shifts);

    //
    t_ranges.clear();

    QDateTime left;
    if(state_at_beginning)
        left = QDateTime(date, QTime(0,0));
    for(auto it=state_shifts.begin(); it!=state_shifts.end(); it++)
    {
        if(it.value())
        {
           Q_ASSERT(left.isNull());
           left = it.key();
        }
        else
        {
            Q_ASSERT(left.isValid());
            t_ranges << clUtil_TimeRange(left, it.key());
            left = QDateTime();
        }
    }
    Q_ASSERT(left.isNull());

    //
    hrmin_ranges.clear();

    clUtil_HrMinRange hrmin_range;
    QDate base_date;
    foreach(clUtil_TimeRange t_range, t_ranges)
    {
        hrmin_range_from_time_range(t_range, &hrmin_range, &base_date);
        Q_ASSERT(base_date == date);
        hrmin_ranges << hrmin_range;
    }

    return hrmin_ranges;
}
