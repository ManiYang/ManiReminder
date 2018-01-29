#include <QMultiMap>
#include <QDebug>
#include "Util_TimeRange.h"

clUtil_TimeRange::clUtil_TimeRange()
{
}

clUtil_TimeRange::clUtil_TimeRange(const QDateTime &left, const QDateTime &right)
{
    Q_ASSERT(left.isValid() && right.isValid());
    Q_ASSERT(left < right);

    mLeft = left;
    mRight = right;
}

clUtil_TimeRange::clUtil_TimeRange(const std::pair<QDateTime, QDateTime> &t_pair)
{
    Q_ASSERT(t_pair.first.isValid() && t_pair.second.isValid());
    Q_ASSERT(t_pair.first < t_pair.second);

    mLeft = t_pair.first;
    mRight = t_pair.second;
}

void clUtil_TimeRange::clear()
{
    mLeft = QDateTime();
}

bool clUtil_TimeRange::includes(const QDateTime &t) const
{
    if(is_empty())
        return false;

    return mLeft <= t && t < mRight;
}

QString clUtil_TimeRange::print(const QString &time_format) const
{
    Q_ASSERT(! is_empty());

    return QString("[%1, %2)").arg(mLeft.toString(time_format))
                              .arg(mRight.toString(time_format));
}

bool clUtil_TimeRange::operator ==(const clUtil_TimeRange &another) const
{
    if(is_empty())
        return another.is_empty();

    return mLeft == another.mLeft && mRight == another.mRight;
}

//////////

void find_state_shifts_of_time_range_collection(const QList<clUtil_TimeRange> &time_ranges,
                                                const char option,
                                                const QDateTime &t0, const QDateTime &t1,
                                                bool *state_at_t0,
                                                QMap<QDateTime,bool> *state_shifts)
// If `option` is 'u' or 'U' (union):
//   The state at any time t is ON if t is included in the UNION of the time ranges in
//   `time_ranges[]`; otherwise, the state at t is OFF. If `time_ranges[]` is empty, the
//   state is OFF at any time.
// If `option` is 'i' or 'I' (intersection):
//   The state at any time t is ON if t is included in the INTERSECTION of the time ranges
//   `in time_ranges[]`; otherwise, the state at t is OFF. If `time_ranges[]` is empty, the
//   state is ON at any time.
//
// This function determines the state at 't0' and finds all state shifts within ('t0', 't1']:
// (*state_shifts)[t] = true means that there is a shift OFF->ON at t.
// (*state_shifts)[t] = false means that there is a shift ON->OFF at t.
{
    Q_ASSERT(t0.isValid() && t1.isValid());
    Q_ASSERT(t0 < t1);
    Q_ASSERT(option == 'u' || option == 'U' || option == 'i' || option == 'I');

    /// Determine the number of time intervals in `time_ranges` that includes `t0`
    /// (--> `Nranges_containing_t0`) and records all interval endpoints within
    /// (`t0`, `t1`] (--> 'endpoints')
    int Nranges_containing_t0 = 0;
    QMultiMap<QDateTime,bool> endpoints; //true: left endpoint
    foreach(clUtil_TimeRange t_rng, time_ranges)
    {
        if(t_rng.includes(t0))
            Nranges_containing_t0++;

        //
        QDateTime left_endpnt = t_rng.left(), right_endpnt = t_rng.right();
        if(t0 < left_endpnt && left_endpnt <= t1)
            endpoints.insert(left_endpnt, true);
        if(t0 < right_endpnt && right_endpnt <= t1)
            endpoints.insert(right_endpnt, false);
    }

    //
    if(option=='u' || option=='U')
        *state_at_t0 = (Nranges_containing_t0 > 0);
    else
        *state_at_t0 = (Nranges_containing_t0 == time_ranges.size());

    ///
    QMap<QDateTime,int> counts; //counts[t] is the count after t before next t

    int cnt = Nranges_containing_t0;
    for(auto it=endpoints.begin(); it!=endpoints.end(); it++)
    {
        QDateTime t = it.key();

        if(it.value())
            cnt++;
        else
            cnt--;

        counts[t] = cnt; //replace the value if `t` is already included in `counts`
    }

    ///
    state_shifts->clear();
    cnt = Nranges_containing_t0;

    if(option=='u' || option=='U')
    {
        for(auto it=counts.begin(); it!=counts.end(); it++)
        {
            int new_cnt = it.value();
            if(cnt == 0 && new_cnt > 0)
                state_shifts->insert(it.key(), true);
            else if(cnt > 0 && new_cnt == 0)
                state_shifts->insert(it.key(), false);

            //
            cnt = new_cnt;
        }
    }
    else
    {
        const int N = time_ranges.size();
        for(auto it=counts.begin(); it!=counts.end(); it++)
        {
            int new_cnt = it.value();
            if(cnt < N && new_cnt == N)
                state_shifts->insert(it.key(), true);
            else if(cnt == N && new_cnt < N)
                state_shifts->insert(it.key(), false);

            //
            cnt = new_cnt;
        }
    }
}
