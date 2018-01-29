#include <QDebug>
#include "RemSpec_TimeRangeFilter.h"
#include "utility_DOM.h"
typedef clDataElem_TimeRangeCollection clTimeRangeCollection;

const int ForthSchedulingDuration = 114; //(minute)

clRemSpec_TimeRangeFilter::clRemSpec_TimeRangeFilter(QObject *parent)
    : clRemSpec_Abstract(ForthSchedulingDuration, parent)
{
    mPassing = true;
}

/*
clRemSpec_TimeRangeFilter::clRemSpec_TimeRangeFilter(
                       const QList<clDataElement_TimeRangeCollection> &time_range_collections,
                       QObject *parent)
    : clRemSpec_Abstract(ForthSchedulingDuration, parent)
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

void clRemSpec_TimeRangeFilter::clear()
{
    mTimeRangeCollections.clear();
    mPassing = true;
    clRemSpec_Abstract::clear();
}

void clRemSpec_TimeRangeFilter::update_settings(const clRemSpec_TimeRangeFilter &another)
{
    clear();
    mTimeRangeCollections = another.mTimeRangeCollections;
    initialize();
}

bool clRemSpec_TimeRangeFilter::parse_and_set(QDomElement &spec_setting, QString &error_msg)
{
    clear();

    //directly under 'spec_setting':
    //  <time-range-filter> time-range collection 1 </time-range-filter>
    //  <time-range-filter> time-range collection 2 </time-range-filter>
    //  ...
    QStringList SL
            = nsDomUtil::get_texts_of_child_elements(spec_setting, "time-range-filter");
    foreach(QString TRC_setting, SL)
    {
        clDataElem_TimeRangeCollection TRC;
        bool ch = TRC.parse_and_set(TRC_setting);
        if(!ch)
        {
            error_msg = QString("Error in parsing time-range-filter setting \"%1\".")
                        .arg(TRC_setting);
            return false;
        }

        if(! TRC.is_empty())
            mTimeRangeCollections << TRC;
    }

    //
    return true;
}

void clRemSpec_TimeRangeFilter::initialize()
{
    if(is_empty())
        mPassing = true;
    else
    {
        start_forth_scheduling(); //this goes first
        determine_state(QDateTime::currentDateTime());
    }
}

bool clRemSpec_TimeRangeFilter::update_settings(
                                             const QStringList &time_range_collection_settings,
                                             QString &parse_error_msg)
//All connections must be built properly before calling this.
//Use 'passing()' afterward to get new state.
{
    clear();

    foreach(QString TRB_setting, time_range_collection_settings)
    {
        clDataElem_TimeRangeCollection TRC;
        bool ch = TRC.parse_and_set(TRB_setting);
        if(!ch)
        {
            parse_error_msg = QString("Error in parsing time-range-filter setting \"%1\".")
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

QString clRemSpec_TimeRangeFilter::print() const
{
    QString S;
    for(int i=0; i<mTimeRangeCollections.count(); i++)
    {
        if(i > 0)
            S += '\n';
        S += "time-range filter: ";
        S += mTimeRangeCollections.at(i).print();
    }
    return S;
}

void clRemSpec_TimeRangeFilter::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    for(int i=0; i<mTimeRangeCollections.size(); i++)
    {
        nsDomUtil::add_child_element_w_text(doc, parent, "time-range-filter",
                                            mTimeRangeCollections[i].print());
    }
}

bool clRemSpec_TimeRangeFilter::operator == (const clRemSpec_TimeRangeFilter &another) const
//Compare settings only.
//Return false if the settings are not literally the same, even if they
//are effectively the same.
{
    return mTimeRangeCollections == another.mTimeRangeCollections;
}

void clRemSpec_TimeRangeFilter::operator = (const clRemSpec_TimeRangeFilter &another)
//Clear and copy settings only.
{
    clear();
    mTimeRangeCollections = another.mTimeRangeCollections;
}

void clRemSpec_TimeRangeFilter::perform_actions(const QMultiMap<QDateTime,clAction> &actions)
{
    for(auto it=actions.constBegin(); it!=actions.constEnd(); it++)
    {
        if(it.value().code == FilterIn)
            mPassing = true;
        else if(it.value().code == FilterOut)
            mPassing = false;
    }
}

QMultiMap<QDateTime,clAction> clRemSpec_TimeRangeFilter::get_actions_within_time_range(
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
        TRC.state_shifts_within_time_range(t0, t1, 'I', &state0, &shifts); //(t0,t1]

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
    const int N = mTimeRangeCollections.size();

    QMultiMap<QDateTime,clAction> actions;
    cnt = count_at_t0;
    for(auto it=counts.begin(); it!=counts.end(); it++)
    {
        int new_cnt = it.value();

        if(cnt < N && new_cnt == N)
            actions.insert(it.key(), clAction(FilterIn));
        else if(cnt == N && new_cnt < N)
            actions.insert(it.key(), clAction(FilterOut));

        //
        cnt = new_cnt;
    }

    return actions;
}

void clRemSpec_TimeRangeFilter::determine_state(const QDateTime &at)
//determine `mPassing`
{
    Q_ASSERT(at.isValid());

    mPassing = true;
    foreach(clTimeRangeCollection TRC, mTimeRangeCollections)
    {
        if(! TRC.includes(at))
        {
            mPassing = false;
            break;
        }
    }
}

QString clRemSpec_TimeRangeFilter::get_action_name(const int action_code) const
{
    if(action_code == FilterIn)
        return "filter-in";
    else if(action_code == FilterOut)
        return "filter-out";

    return "unknown";
}
