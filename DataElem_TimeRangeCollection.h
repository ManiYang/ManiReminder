#ifndef CLDATAELEMENT_TIMERANGECOLLECTION_H
#define CLDATAELEMENT_TIMERANGECOLLECTION_H

#include <QList>
#include <QDateTime>
#include "DataElem_DatePattern.h"
#include "Util_HrMinRange.h"
#include "Util_TimeRange.h"

class clDataElem_TimeRangeCollection
{
public:
    clDataElem_TimeRangeCollection();

    bool parse_and_set(const QString &S);
         //formats:
         //  "all time"
         //  "<date-pattern>; <hr>:<min>-<hr>:<min>,<hr>:<min>-<hr>:<min>,..."

    void clear();

    //
    bool is_empty() const { return mDatePattern.is_empty(); }

    bool includes(const QDateTime &t) const;
    bool includes_date(const QDate &date) const; //include a time range in `date`?

    void state_shifts_within_time_range(const QDateTime &t0, const QDateTime &t1,
                                        const char option,
                                        bool *is_on_at_t0,
                                        QMap<QDateTime,bool> *state_shifts);
         //If `option` is 'u' or 'U':
         //  The state at time t is ON iff t is included in the union of `*this`.
         //If `option` is 'i' or 'I':
         //  The state at time t is ON iff t is included in the intersection of `*this`.
         //
         //This function determines the state at `t0` and finds all state shifts within
         //(`t0`, `t1`].
         //`(*state_shifts)[t] == true` means there's a state shift OFF->ON at `t`.
         //`(*state_shifts)[t] == false` means there's a state shift ON->OFF at `t`.

    QList<clUtil_HrMinRange> get_hrmin_ranges() const { return mHrMinRanges; }

    bool operator == (const clDataElem_TimeRangeCollection &another) const;
                     //Empty collections are considered equal.
                     //Return false if the settings are not literally the same, even if they are
                     //effectively the same.

    QString print() const; //*this cannot be empty

private:
    clDataElem_DatePattern mDatePattern; //empty => *this is empty
    QList<clUtil_HrMinRange> mHrMinRanges; //intervals like [(hr0,min0), (hr1,min1))
    // The time-range collection is
    //     { (D, T) | D is a date included in 'mDatePattern',
    //                and T is a hr-min range in mHrMinRanges[] }.

    //
    int HrMinRanges_cross_days() const;
    bool is_in_HrMinRanges(const int hr, const int min, const int sec, const int msec) const;
};

#endif // CLDATAELEMENT_TIMERANGECOLLECTION_H
