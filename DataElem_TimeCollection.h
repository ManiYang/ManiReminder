#ifndef DATA_ELEMENT_TIME_COLLECTION_H
#define DATA_ELEMENT_TIME_COLLECTION_H

#include "DataElem_DatePattern.h"

class clDataElem_TimeCollection
{
public:
    clDataElem_TimeCollection();

    bool parse_and_set(const QString &S); //format: "<date-pattern>; <h:mm>,<h:mm>,..."

    void clear();

    //
    bool is_empty() const { return mDatePattern.is_empty(); }

    bool includes_date(const QDate &date) const { return mDatePattern.includes(date); }
    QDateTime get_earliest_time() const;
    QList<QDateTime> get_times_within_time_range(const QDateTime &t0, const QDateTime &t1);
                                                 //Get the times of `*this` within (`t0`, `t1`].
    QList<QTime> get_times() const { return mTimes; }

    QString print() const;

    bool operator == (const clDataElem_TimeCollection &another) const;
                                                 //Empty collections are considered equal.

private:
    clDataElem_DatePattern mDatePattern; //empty => *this is empty
    QList<QTime> mTimes; //accurate to minute only
    // The time collection is { (d,t) | d is in `mDatePattern`, t is in `mTimes[]` }.
};

#endif // DATA_ELEMENT_TIME_COLLECTION_H
