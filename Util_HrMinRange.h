//20171206

#ifndef CLUTIL_HRMIN_RANGE_H
#define CLUTIL_HRMIN_RANGE_H

#include <QDomDocument>
#include <QDomElement>
#include <QDateTime>
#include "Util_TimeRange.h"

/* Class representing a time interval [(hr0, min0, sec=0.0), (hr1, min1, sec=0.0)). */

class clUtil_HrMinRange
{
public:
    clUtil_HrMinRange();
    clUtil_HrMinRange(const int hr0, const int min0, const int hr1, const int min1);
                     //'hr0': 0 ~ 23.
                     //'hr1' can be >= 24, meaning on the next day or beyond

    void clear() { mHr0 = -1; }

    bool parse_and_set(const QString &S);
                       //parse 'S' as "[hr0:min0,hr1:min1)" or "hr0:min0-hr1:min1"

    void merge_with_overlapping_or_adjacent(const clUtil_HrMinRange &another);
                                            //'another' must overlap or is adjacent to *this.

    //
    bool is_empty() const { return mHr0 == -1; }

    int get_start_hr() const;
    int get_start_min() const;
    int get_end_hr() const;  //note: the range is [start, end)
    int get_end_min() const; //note: the range is [start, end)

    int get_start_minute_number() const; //mHr0 * 60 + mMin0
    int get_end_minute_number() const;

    QString print(const int option = 1) const; //option -- 1: "[hr0:min0,hr1:min1)"
                                               //          2: "hr0:min0-hr1:min1"

    bool includes(const int hr, const int min,
                  const int sec = 0, const int minisec = 0) const;
                  //Returns whether the specified time is within
                  //   [ (mHr0, mMin0, sec=0.0), (mHr1, mMin1, sec=0.0) ).
                  //'hr' can be >= 24, meaning on the next day or beyond.

    bool overlaps_or_is_adjacent_to(const clUtil_HrMinRange &another) const;

    bool operator == (const clUtil_HrMinRange &another) const;
                      //Empty ranges are considered equal.

    //
    void get_DateTime_range(const QDate &base_date, QDateTime &t0, QDateTime &t1) const;
    clUtil_TimeRange get_DateTime_range(const QDate &base_date) const;
                   // Get date-time interval [t0,t1) represented by *this, regarding (hr=0,min=0)
                   // as the time 0:00 on 'base_date'.

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    bool set_from_XML(QDomElement &parent, int child_No = 0);
         //Read and parse the 'child_No'-th of the child nodes that have target tag name.
         //Returns true iff such child node is found. child_No counts from 0.

private:
    int mHr0; //0 ~ 23. -1: empty
    int mMin0; //0 ~ 59
    int mHr1; // >= 0; can be >= 24, meaning on the next days
    int mMin1; //0 ~ 59
    // The time range is [(mHr0, mMin0, sec=0.0), (mHr1, mMin1, sec=0.0)).

    //
    bool is_later_or_equal_than_start_time(const int hr, const int min,
                                           const int sec, const int minisec) const;
    bool is_earlier_than_end_time(const int hr, const int min,
                                  const int sec, const int minisec) const;    
};

//////////////////////////

void hrmin_range_from_time_range(const clUtil_TimeRange &time_range,
                                 clUtil_HrMinRange *hrmin_range, QDate *base_date);
//Find `*hrmin_range`, `*base_date` such that `*hrmin_range` on `*base_date` is equivalent
//to `time_range`. Accurate only to minutes.

#endif // CLUTIL_HRMIN_RANGE_H
