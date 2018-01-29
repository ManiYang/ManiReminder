//20171123

#ifndef UTIL_DATE_SET_H
#define UTIL_DATE_SET_H

#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include "Util_DateRange.h"

/* A "date set" is a set of dates, implemented here as a list of date ranges. */

class clUtil_DateSet
{
public:
    clUtil_DateSet();

    void add_date_range(const clUtil_DateRange &date_range);
    void add_date_range(const QDate &from, const QDate &to);
    void add_date_range(const int yr, const int mn, const int day_from, const int day_to);
                        //Add [day_from, day_to] in the month (yr, mn). Invalid dates are ignored.
    void add_date_ranges(const QList<clUtil_DateRange> &date_ranges);
    void add_dates(const QDate &from, const qint64 Ndays);
    void add_date(const QDate &date);

    void take_union_with(const clUtil_DateSet &another);
    void merge_with(const clUtil_DateSet &another) { take_union_with(another); }
    void take_intersection_with(const clUtil_DateRange &DR); // [not tested]
    void take_intersection_with(const clUtil_DateSet &another); // [not tested]
    void subtracted_by(const clUtil_DateRange &DR);     // [not tested]
    void subtracted_by(const clUtil_DateSet &another);  // [not tested]

    void clear();

    bool parse_and_set(const QString &S);
        //Parse 'S' as "YYYY/M/D[-YYYY/M/D],YYYY/M/D[-YYYY/M/D],...".
        //Correct "(ddd)" can be added right after any "YYYY/M/D".
        //Return true iff successful.

    void shift_by_days(const int n);
    void shift_by_months_preserving_dayNo(const int n);
    void shift_by_months_preserving_dayRevNo(const int n);
    void shift_by_months_preserving_weekNo_et_dayOfWeek(const int n);
    void shift_by_months_preserving_weekRevNo_et_dayOfWeek(const int n);

    //
    bool is_empty() const;

    bool includes(const QDate &date) const;
    bool includes(const clUtil_DateRange &date_range) const; //'date_range' should not be empty

    clUtil_DateRange get_span() const; //get the range [earliest date, latest date]
    QDate get_earliest_date() const; //Return null date if empty.
    QDate get_latest_date() const; //Return null date if empty.

    qint64 get_Ndays() const;
    QList<QDate> get_all_dates() const;

    QString print(const bool MMDD = false) const;
            //'MMDD': date format (true: YYYY/MM/DD, false: YYYY/M/D)

    bool operator == (const clUtil_DateSet &another) const; //empty date sets are considered equal
    bool operator != (const clUtil_DateSet &another) const; //empty date sets are considered equal

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    bool set_from_XML(QDomElement &parent);
         //Read and parse the child node that have tag name "clUtil_DateSet".
         //Returns true iff done.

private:
    QList<clUtil_DateRange> mDateRanges;
    // mDateRanges[] should be simplified so that
    //   1. there is no empty date range,
    //   2. there are no adjacent date-ranges (e.g., 2017/1/1-2017/1/10, 2017/1/11-2017/1/20),
    //   3. there are no overlaping date-ranges (e.g., 2017/1/1-2017/1/12, 2017/1/9-2017/1/20),
    //   4. the date-ranges are in ascending order.
};

/////////

clUtil_DateSet shift_date_range_by_months_preserving_weekNo_et_dayOfWeek(
                                          const clUtil_DateRange &date_range, const int Nmonths);
clUtil_DateSet shift_date_range_by_months_preserving_weekRevNo_et_dayOfWeek(
                                          const clUtil_DateRange &date_range, const int Nmonths);

#endif // UTIL_DATE_SET_H
