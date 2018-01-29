//20171123

#ifndef CLUTIL_DATE_RANGE_H
#define CLUTIL_DATE_RANGE_H

#include <QDate>
#include <QString>
#include <QList>
#include <QDomDocument>
#include <QDomElement>

typedef const QString CSTR;

class clUtil_DateRange
{
    friend QList<clUtil_DateRange> DateRange_subtraction(const clUtil_DateRange &DR1,
                                                         const clUtil_DateRange &DR2);

public:
    clUtil_DateRange(); //empty range
    clUtil_DateRange(const QDate &date_start, const QDate &date_end);
    explicit clUtil_DateRange(const QDate &date, const qint64 Ndays = 1);

    void set_empty();
    bool parse_and_set(const QString &s); //Parse 's' as "yyyy/M/d(ddd)" or
                                          //"yyyy/M/d(ddd)-yyyy/M/d(ddd)", where "(ddd)" can be
                                          //omitted. Return true iff successful.

    //
    bool set(const int year0, const int month0, const int day0,
             const int year1, const int month1, const int day1,
             CSTR &var_name_year0, CSTR &var_name_month0, CSTR &var_name_day0,
             CSTR &var_name_year1, CSTR &var_name_month1, CSTR &var_name_day1); //CSTR := const QString
    bool set(const int year, const int month, const int day,
             CSTR &var_name_year, CSTR &var_name_month, CSTR &var_name_day,
             const qint64 Ndays = 1, CSTR &var_name_Ndays = "");
    QString error_message;
        //Returns false if there is problem, in which case the error message will be copied
        //to this->error_message.

    void take_union_with(const clUtil_DateRange &another);
    void merge_with(const clUtil_DateRange &another) { take_union_with(another); }
                    //If *this and another are both non-empty, 'another' must overlaps or is
                    //adjacent to *this.
    void take_intersection_with(const clUtil_DateRange &another);

    void shift_by_days(const int n);
    void shift_by_months_preserving_dayNo(const int n); //number of days in the range may change
    void shift_by_months_preserving_dayRevNo(const int n); //number of days in the range may change

    //
    bool is_empty() const;
    QDate get_start_date() const; //*this should not be empty
    QDate get_end_date() const;   //*this should not be empty
    qint64 get_Ndays() const;
    QList<QDate> get_all_dates() const;
    QString print(const QString &to = "-", const bool MMDD = false) const;
            //*this should not be empty
            //MMDD -- true: YYYY/MM/DD,  false: YYYY/M/D

    bool includes(const QDate &d) const;
    bool is_superset_of(const clUtil_DateRange &another) const;
    bool is_subset_of(const clUtil_DateRange &another) const;
    bool overlaps(const clUtil_DateRange &another) const;
    bool overlaps_or_is_adjacent_to(const clUtil_DateRange &another) const;

    bool has_tail_of_month() const; //contains 29th, 30st, or 31st of month
    bool has_head_of_month() const; //contains 29th, 30st, or 31st of month counted backward
    bool has_Feb_29() const; //contains Feb 29

    bool operator == (const clUtil_DateRange &another) const; //empty ranges are considered equal

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    bool set_from_XML(QDomElement &parent, int child_No = 0);
         //Read and parse the 'child_No'-th child node that have tag name "clUtil_DateRange".
         //'child_No' counts from 0. Returns true iff done.

private:
    QDate mDate0; //starting date
    QDate mDate1; //ending date
    // If mDate0 is invalid, *this is an empty range.
    // Otherwise, mDate1 should be >= mDate0.
    // If mDate0 = mDate1, *this contains only a single date.
};

////

QList<clUtil_DateRange> DateRange_subtraction(const clUtil_DateRange &DR1,
                                              const clUtil_DateRange &DR2); //DR1 - DR2

clUtil_DateRange get_month_including(const QDate &date);
                 //Return the month, as a date-range, that includes 'date'.

QList<clUtil_DateRange> split_date_range_by_months(const clUtil_DateRange &date_range);
                        //Divide 'date_range' into date-ranges, each of which lies within a month.

#endif // CLUTIL_DATE_RANGE_H
