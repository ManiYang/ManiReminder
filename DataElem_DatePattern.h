#ifndef CLDATAELEMENT_DATEPATTERN_H
#define CLDATAELEMENT_DATEPATTERN_H

#include "Util_DateSet.h"

/* A "date-pattern" contains a date-set (called the base date-set), which can be repeated
 * periodically, possibly up to a final date. */

class clDataElem_DatePattern
{
public:
    enum enRepeatType {
        NoRepeat,
        Day, Week,       //repeat every n days/weeks
        Month_DayNo,     //repeat on the same day number, every n months
        Month_DayRevNo,  //repeat on the same day number counted from last day of month
        Month_WeekNo,    //repeat on the day (Mon~Sun) of the same week number of month
        Month_WeekRevNo, //repeat on the day of the same week number counted from last
        Year             //repeat on the same date (month/day), every n years
    };

    clDataElem_DatePattern();

    bool parse_and_set(const QString &S);
                      //Format of `S`:
                      //   <base-date-set> [repeat every <method> [final <final-date>]]
                      //   daily [final <final-date>]
                      //where <method> can be
                      //   day, <N> days, week, <N> weeks,
                      //   month by <month-method>, <N> months by <month-method>,
                      //   year, <N> years
                      //where <month-method> can be dayNo, dayRevNo, weekNo, weekRevNo.
                      //The format for a date is "YYYY/M/D" or "YYYY/M/D(ddd)".

    void clear();

    //
    bool is_empty() const { return mBaseDateSet.is_empty(); }

    QDate get_earliest_date() const;

    bool includes(const QDate &date) const;

    QList<int> days_within_month(const int year, const int month) const;
               //Get the (day numbers of) days within (year, month) that are included.

    bool operator == (const clDataElem_DatePattern &another) const;
    bool operator != (const clDataElem_DatePattern &another) const;

    QString print() const; //The returned string is parsable by `this->parse_and_set()`
                           //`*this` cannot be empty.

private:
    clUtil_DateSet mBaseDateSet; //If empty, `*this` is empty and the following parameters are
                                 //meaningless.
    enRepeatType mRepeatType; //If is NoRepeat, the following parameters are meaningless.
    int mRepeatEvery; //must be >= 1
    QDate mFinalDate; //null date: no final date.
                      //'mFinalDate' is considered only if 'mRepeatType' != NoRepeat.
    //
    bool date_query__repeating_every_n_days(const QDate &date, const int n) const;
        //Determine whether 'date' is included when 'mBaseDateSet' (non-empty) is repeated every
        //n days infinitely.

    bool date_query__repeating_monthly(const QDate &date) const;
        //Determine whether 'date' is included when 'mBaseDateSet' is repeated according to
        //'mRepeatType' (Month_DayNo, Month_DayRevNo, Month_WeekNo, or Month_WeekRevNo) every
        //'mRepeatEvery' months infinitely.

    bool date_query__repeating_monthly___base_within_month(const QDate &date,
                                                      const clUtil_DateSet &base_dateSet) const;
        //Determine whether 'date' is included when 'base_dateSet' (within a month) is repeated
        //according to 'mRepeatType' (Month_DayNo, Month_DayRevNo, Month_WeekNo, or
        //Month_WeekRevNo) every 'mRepeatEvery' months infinitely.

    bool date_query__repeating_yearly(const QDate &date) const;
        //Determine whether 'date' is included when 'mBaseDateSet' is repeated every
        //'mRepeatEvery' years infinitely.
};

#endif // CLDATAELEMENT_DATEPATTERN_H
