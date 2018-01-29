#ifndef UTIL_TIME_RANGE_H
#define UTIL_TIME_RANGE_H

#include <QDateTime>
#include <QList>
#include <QMap>

class clUtil_TimeRange
{
public:
    clUtil_TimeRange();
    clUtil_TimeRange(const QDateTime &left, const QDateTime &right); //[left, right)
    explicit clUtil_TimeRange(const std::pair<QDateTime, QDateTime> &t_pair); //[first, second)

    void clear();

    //
    QDateTime left() const { return mLeft; }
    QDateTime right() const { return mRight; }

    bool is_empty() const { return ! mLeft.isValid(); }
    bool includes(const QDateTime &t) const;

    QString print(const QString &time_format = "yyyy/MM/dd.hh:mm:ss") const;

    bool operator == (const clUtil_TimeRange &another) const;

private:
    QDateTime mLeft; //invalid: *this is empty
    QDateTime mRight;
    //The time interval is [mLeft, mRight).
};

////

void find_state_shifts_of_time_range_collection(const QList<clUtil_TimeRange> &time_ranges,
                                                const char option,
                                                const QDateTime &t0, const QDateTime &t1,
                                                bool *state_at_t0,
                                                QMap<QDateTime,bool> *state_shifts);
// If `option` is 'u' or 'U' (union):
//   The state at any time t is ON if t is included in the UNION of the time ranges in
//   `time_ranges[]`; otherwise, the state at t is OFF.
// If `option` is 'i' or 'I' (intersection):
//   The state at any time t is ON if t is included in the INTERSECTION of the time ranges
//   `in time_ranges[]`; otherwise, the state at t is OFF.
//
// This function determines the state at 't0' and finds all state shifts within ('t0', 't1']:
// (*state_shifts)[t] = true means that there is a shift OFF->ON at t.
// (*state_shifts)[t] = false means that there is a shift ON->OFF at t.

#endif // UTIL_TIME_RANGE_H
