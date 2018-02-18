#include <utility>
#include <algorithm>
#include <QDebug>
#include "DataElem_TimeRangeCollection.h"

clDataElem_TimeRangeCollection::clDataElem_TimeRangeCollection()
{
}

void clDataElem_TimeRangeCollection::clear()
{
    mDatePattern.clear();
}

bool clDataElem_TimeRangeCollection::operator ==
                                       (const clDataElem_TimeRangeCollection &another) const
//Empty collections are considered equal.
{
    if(is_empty())
        return another.is_empty();

    return mDatePattern == another.mDatePattern
           && mHrMinRanges == another.mHrMinRanges;
}

bool clDataElem_TimeRangeCollection::includes(const QDateTime &t) const
{
    if(is_empty())
        return false;

    if(t.date() < mDatePattern.get_earliest_date())
        return false;

    //
    const QDate date = t.date();
    const QTime time = t.time();

    int N_extended_days = HrMinRanges_cross_days();
    //qDebug() << N_extended_days;
    for(int dext=0; dext<=N_extended_days; dext++)
    {
        QDate base_date = date.addDays(-dext);
        if(mDatePattern.includes(base_date))
        {
            //qDebug() << base_date;
            if(is_in_HrMinRanges(time.hour()+24*dext, time.minute(), time.second(), time.msec()))
                return true;
        }
    }
    return false;
}

void clDataElem_TimeRangeCollection::state_shifts_within_time_range(
                                                   const QDateTime &t0, const QDateTime &t1,
                                                   const char option, bool *is_on_at_t0,
                                                   QMap<QDateTime, bool> *state_shifts)
//If `option` is 'u' or 'U':
//  The state at time t is ON iff t is included in the union of `*this`.
//If `option` is 'i' or 'I':
//  The state at time t is ON iff t is included in the intersection of `*this`.
//
//This function determines the state at `t0` and finds all state shifts within (`t0`, `t1`].
//`(*state_shifts)[t] == true` means there's a state shift OFF->ON at `t`.
//`(*state_shifts)[t] == false` means there's a state shift ON->OFF at `t`.
{
    Q_ASSERT(t0.isValid() && t1.isValid());
    Q_ASSERT(t0 < t1);
    Q_ASSERT(option=='u' || option=='U' || option=='i' || option=='I');

    const int dext = HrMinRanges_cross_days();

    // get all time intervals with base dates within [`t0.date()`-`dext`. `t1.date()`]
    // --> `time_ranges[]`
    QList<clUtil_TimeRange> time_ranges;

    QDate base_date = t0.date().addDays(-dext);
    for( ; base_date<=t1.date(); base_date=base_date.addDays(1))
    {
        foreach(clUtil_HrMinRange hm_rng, mHrMinRanges)
        {
            clUtil_TimeRange t_rng = hm_rng.get_DateTime_range(base_date);
            time_ranges << t_rng;
        }
    }

    // get the state at `t0` and the state shifts within (`t0`, `t1`], given the time
    // intervals `time_ranges[]`
    find_state_shifts_of_time_range_collection(time_ranges, option, t0, t1,
                                               is_on_at_t0, state_shifts);
}

QString clDataElem_TimeRangeCollection::print() const
//*this cannot be empty
{
    Q_ASSERT(! is_empty());

    QString S = mDatePattern.print();
    S += "; ";
    for(int i=0; i<mHrMinRanges.count(); i++)
    {
        if(i > 0)
            S += ",";
        S += mHrMinRanges.at(i).print(2);
    }
    return S;
}

bool clDataElem_TimeRangeCollection::parse_and_set(const QString &S)
//format:
//  "all time"
//  "<date-pattern>; <hr>:<min>-<hr>:<min>,<hr>:<min>-<hr>:<min>,..."
{
    mDatePattern.clear();
    mHrMinRanges.clear();

    //
    QStringList tokens;
    if(S.simplified() == "all time")
        tokens << "daily" << "0:00-24:00";
    else
        tokens = S.split(';');

    if(tokens.count() != 2)
        return false;

    //
    bool ch = mDatePattern.parse_and_set(tokens.at(0));
    if(!ch)
        return false;

    //
    QStringList str_HrMinRanges = tokens.at(1).split(',');
    if(str_HrMinRanges.isEmpty())
    {
        mDatePattern.clear();
        return false;
    }

    foreach(QString strR, str_HrMinRanges)
    {
        clUtil_HrMinRange R;
        bool ch = R.parse_and_set(strR);
        if(!ch)
        {
            mDatePattern.clear();
            return false;
        }
        mHrMinRanges << R;
    }

    //
    return true;
}

int clDataElem_TimeRangeCollection::HrMinRanges_cross_days() const
//Returns 0 if the hr-min ranges mHrMinRanges[] all lie within "today".
//Returns n if mHrMinRanges[] extends to n days and at most n days after today.
{
    Q_ASSERT(! mHrMinRanges.isEmpty());

    // find the upper bound (hr1, min1, sec=0.0) of the set of time ranges represented
    // by mHrMinRanges[]
    int hr1=-1, min1;
    foreach(clUtil_HrMinRange R, mHrMinRanges)
    {
        if( R.get_end_hr() > hr1
            || (R.get_end_hr() == hr1 && R.get_end_min() > min1) )
        {
            hr1 = R.get_end_hr();
            min1 = R.get_end_min();
        }
    }

    //
    int answer;
    if(hr1%24 == 0 && min1 == 0)
        answer = hr1/24 - 1;
    else
        answer = hr1/24;

    //
    Q_ASSERT(answer >= 0);
    return answer;
}

bool clDataElem_TimeRangeCollection::is_in_HrMinRanges(const int hr, const int min,
                                                          const int sec, const int msec) const
{
    foreach(clUtil_HrMinRange R, mHrMinRanges)
    {
        if(R.includes(hr, min, sec, msec))
            return true;
    }
    return false;
}

bool clDataElem_TimeRangeCollection::includes_date(const QDate &date) const
//include a time range in `date`?
{
    return mDatePattern.includes(date);
}
