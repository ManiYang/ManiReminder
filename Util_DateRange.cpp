//20171123

#include <QTextStream>
#include "Util_DateRange.h"
#include "utility_DOM.h"
#include "utility_date.h"

clUtil_DateRange::clUtil_DateRange()
{
}

clUtil_DateRange::clUtil_DateRange(const QDate &date, const qint64 Ndays)
    : mDate0(date), mDate1()
{
    Q_ASSERT(date.isValid());
    Q_ASSERT(Ndays >= 1);
    mDate1 = mDate0.addDays(Ndays-1);
}

clUtil_DateRange::clUtil_DateRange(const QDate &date_start, const QDate &date_end)
    : mDate0(date_start), mDate1(date_end)
{
    Q_ASSERT(date_start.isValid());
    Q_ASSERT(date_end.isValid());
    Q_ASSERT(date_start <= date_end);
}

void clUtil_DateRange::set_empty()
{
    mDate0 = QDate();
}

bool clUtil_DateRange::parse_and_set(const QString &s)
//Parse 's' as "yyyy/M/d(ddd)" or "yyyy/M/d(ddd)-yyyy/M/d(ddd)", where "(ddd)" can be omitted.
//Return true iff successful.
{
    int pos_dash = s.indexOf('-');
    bool good = true;
    if(pos_dash != -1)
    {
        QString s1 = s.left(pos_dash),
                s2 = s.mid(pos_dash+1);
        mDate0 = nsUtilDate::parse_as_date(s1);
        mDate1 = nsUtilDate::parse_as_date(s2);
        if(!mDate0.isValid() || !mDate1.isValid())
            good = false;
        else if(mDate1 < mDate0)
            good = false;
    }
    else
    {
        mDate0 = nsUtilDate::parse_as_date(s);
        if(! mDate0.isValid())
            good = false;
        else
            mDate1 = mDate0;
    }

    //
    if(!good)
        mDate0 = QDate();
    return good;
}

bool clUtil_DateRange::set(const int year0, const int month0, const int day0,
                           const int year1, const int month1, const int day1,
                           CSTR &var_name_year0, CSTR &var_name_month0, CSTR &var_name_day0,
                           CSTR &var_name_year1, CSTR &var_name_month1, CSTR &var_name_day1)
//Returns false if there is problem, in which case the error message will be copied to this->error_message.
{
    QDate d0(year0, month0, day0);
    if(! d0.isValid())
    {
        error_message = QString("Date (%1,%2,%3)=(%4,%5,%6) is invalid.")
                        .arg(var_name_year0, var_name_month0, var_name_day0)
                        .arg(year0).arg(month0).arg(day0);
        return false;
    }

    QDate d1(year1, month1, day1);
    if(! d1.isValid())
    {
        error_message = QString("Date (%1,%2,%3)=(%4,%5,%6) is invalid.")
                        .arg(var_name_year1, var_name_month1, var_name_day1)
                        .arg(year1).arg(month1).arg(day1);
        return false;
    }

    if(d0 > d1)
    {
        error_message = QString("Date (%1,%2,%3) cannot be later than date (%4,%5,%6).")
                        .arg(var_name_year0, var_name_month0, var_name_day0)
                        .arg(var_name_year1, var_name_month1, var_name_day1);
        return false;
    }

    //
    mDate0 = d0;
    mDate1 = d1;
    return true;
}

bool clUtil_DateRange::set(const int year, const int month, const int day,
                           CSTR &var_name_year, CSTR &var_name_month, CSTR &var_name_day,
                           const qint64 Ndays, CSTR &var_name_Ndays)
{
    QDate d(year, month, day);
    if(! d.isValid())
    {
        error_message = QString("Date (%1,%2,%3)=(%4,%5,%6) is invalid.")
                        .arg(var_name_year, var_name_month, var_name_day)
                        .arg(year).arg(month).arg(day);
        return false;
    }

    if(Ndays < 1)
    {
        error_message = QString("%1 should be >= 1.").arg(var_name_Ndays);
        return false;
    }

    //
    mDate0 = d;
    mDate1 = d.addDays(Ndays-1);
    return true;
}

void clUtil_DateRange::take_union_with(const clUtil_DateRange &another)
//If *this and another are both non-empty, 'another' must overlaps or is adjacent to *this.
{
    if(! another.mDate0.isValid())
        return;
    else if(! mDate0.isValid())
    {
        mDate0 = another.mDate0;
        mDate1 = another.mDate1;
    }
    else
    {
        Q_ASSERT(overlaps_or_is_adjacent_to(another));
        if(another.mDate0 < mDate0)
            mDate0 = another.mDate0;
        if(another.mDate1 > mDate1)
            mDate1 = another.mDate1;
    }
}

void clUtil_DateRange::take_intersection_with(const clUtil_DateRange &another)
{
    if(! mDate0.isValid())
        return;
    if(! another.mDate0.isValid())
    {
        mDate0 = QDate();
        return;
    }
    if(! overlaps(another))
    {
        mDate0 = QDate();
        return;
    }

    //
    if(another.mDate0 > mDate0)
        mDate0 = another.mDate0;
    if(another.mDate1 < mDate1)
        mDate1 = another.mDate1;
}

void clUtil_DateRange::shift_by_days(const int n)
{
    if(! mDate0.isValid())
        return;

    mDate0 = mDate0.addDays(n);
    mDate1 = mDate1.addDays(n);
}

void clUtil_DateRange::shift_by_months_preserving_dayNo(const int n)
//The number of days in the range may change.
{
    if(! mDate0.isValid())
        return;

    int yr0, mn0, day0;
    int yr1, mn1, day1;
    nsUtilDate::add_months_preserving_dayNoInMonth(mDate0, n, &yr0, &mn0, &day0);
    nsUtilDate::add_months_preserving_dayNoInMonth(mDate1, n, &yr1, &mn1, &day1);

    QDate date0(yr0, mn0, day0);
    QDate date1(yr1, mn1, day1);
    if(! date0.isValid())
        date0 = nsUtilDate::first_valid_date_after(yr0, mn0, day0);
    if(! date1.isValid())
        date1 = nsUtilDate::last_valid_date_before(yr1, mn1, day1);

    if(date0 > date1)
        mDate0 = QDate(); //=> empty range
    else
    {
        mDate0 = date0;
        mDate1 = date1;
    }
}

void clUtil_DateRange::shift_by_months_preserving_dayRevNo(const int n)
{
    if(! mDate0.isValid())
        return;

    int yr0, mn0, day0;
    int yr1, mn1, day1;
    nsUtilDate::add_months_preserving_dayReverseNoInMonth(mDate0, n, &yr0, &mn0, &day0);
    nsUtilDate::add_months_preserving_dayReverseNoInMonth(mDate1, n, &yr1, &mn1, &day1);

    QDate date0(yr0, mn0, day0);
    QDate date1(yr1, mn1, day1);
    if(! date0.isValid())
        date0 = nsUtilDate::first_valid_date_after(yr0, mn0, day0);
    if(! date1.isValid())
        date1 = nsUtilDate::last_valid_date_before(yr1, mn1, day1);

    if(date0 > date1)
        mDate0 = QDate(); //=> empty range
    else
    {
        mDate0 = date0;
        mDate1 = date1;
    }
}

bool clUtil_DateRange::is_empty() const
{
    return !mDate0.isValid();
}

#define ASSERT_NON_EMPTY Q_ASSERT(mDate0.isValid());

QDate clUtil_DateRange::get_start_date() const
{
    ASSERT_NON_EMPTY;
    return mDate0;
}

QDate clUtil_DateRange::get_end_date() const
{
    ASSERT_NON_EMPTY;
    return mDate1;
}

qint64 clUtil_DateRange::get_Ndays() const
{
    if(is_empty())
        return 0;
    else
        return mDate0.daysTo(mDate1) + 1;
}

QList<QDate> clUtil_DateRange::get_all_dates() const
{
    QList<QDate> dates;
    if(mDate0.isValid())
    {
        Q_ASSERT(mDate1 >= mDate0);
        for(QDate d=mDate0; d<=mDate1; d=d.addDays(1))
            dates << d;
    }
    return dates;
}

QString clUtil_DateRange::print(const QString &to, const bool MMDD) const
{
    ASSERT_NON_EMPTY;
    QString format = MMDD ? "yyyy/MM/dd" : "yyyy/M/d";
    QString s = mDate0.toString(format);
    if(mDate1 != mDate0)
        s += to + mDate1.toString(format);
    return s;
}

bool clUtil_DateRange::includes(const QDate &d) const
{
    Q_ASSERT(d.isValid());
    if(is_empty())
        return false;
    else
        return mDate0 <= d && d <= mDate1;
}

bool clUtil_DateRange::is_superset_of(const clUtil_DateRange &another) const
{
    if(another.is_empty())
        return true;
    if(is_empty())
        return false;

    return mDate0 <= another.mDate0  &&  mDate1 >= another.mDate1;
}

bool clUtil_DateRange::is_subset_of(const clUtil_DateRange &another) const
{
    if(is_empty())
        return true;
    if(another.is_empty())
        return false;

    return mDate0 >= another.mDate0  &&  mDate1 <= another.mDate1;
}

bool clUtil_DateRange::overlaps(const clUtil_DateRange &another) const
{
    if(is_empty())
        return false;
    if(another.is_empty())
        return false;
    return !(mDate1 < another.mDate0 || mDate0 > another.mDate1);
}

bool clUtil_DateRange::overlaps_or_is_adjacent_to(const clUtil_DateRange &another) const
{
    if(is_empty())
        return false;
    if(another.is_empty())
        return false;

    return !(mDate1.addDays(1) < another.mDate0 || mDate0.addDays(-1) > another.mDate1);
}

bool clUtil_DateRange::has_tail_of_month() const
//contains 29th, 30st, or 31st of month
{
    if(is_empty())
        return false;

    for(QDate d=mDate0; d<=mDate1; d=d.addDays(1))
    {
        if(d.day() > 28)
            return true;
        else if(d.day() < 28)
            d.setDate(d.year(), d.month(), 28); //skip to 28th
        Q_ASSERT(d.isValid());
    }
    return false;
}

bool clUtil_DateRange::has_head_of_month() const
//contains 29th-, 30st-, or 31st-to-last of month
{
    if(is_empty())
        return false;

    for(QDate d=mDate0; d<=mDate1; d=d.addDays(1))
    {
        if(d.day() <= d.daysInMonth()-28)
            return true;
        else
            d.setDate(d.year(), d.month(), d.daysInMonth()); //skip to last day
        Q_ASSERT(d.isValid());
    }
    return false;
}

bool clUtil_DateRange::has_Feb_29() const
//contains Feb 29
{
    if(is_empty())
        return false;

    for(QDate d=mDate0; d<=mDate1; d=d.addDays(1))
    {
        if(d.month() == 2 && d.day() == 29)
            return true;
        if(d < QDate(d.year(), 2, 28))
            d = QDate(d.year(), 2, 28);
        else if(d.month() >= 3)
            d = QDate(d.year()+1, 2, 28);
        Q_ASSERT(d.isValid());
    }
    return false;
}

bool clUtil_DateRange::operator == (const clUtil_DateRange &another) const
{
    if(is_empty())
        return another.is_empty();

    if(another.is_empty())
        return false;

    return (mDate0 == another.mDate0) && (mDate1 == another.mDate1);
}

void clUtil_DateRange::add_to_XML(QDomDocument &doc, QDomElement &parent) const
//Add under 'parent':
//   <clUtil_DateRange>
//     <date0> yyyy/MM/dd </date0>
//     <date1> yyyy/MM/dd </date1>
//   </clUtil_DateRange>
{
    QDomElement node_date_range
        = nsDomUtil::add_child_element(doc, parent, "clUtil_DateRange");

    if(! is_empty())
    {
        nsDomUtil::add_child_element_w_text(doc, node_date_range,
                                            "date0", mDate0.toString("yyyy/MM/dd"));
        nsDomUtil::add_child_element_w_text(doc, node_date_range,
                                            "date1", mDate1.toString("yyyy/MM/dd"));
    }
    else
    {
        nsDomUtil::add_child_element_w_text(doc, node_date_range, "date0", "null");
        nsDomUtil::add_child_element_w_text(doc, node_date_range, "date1", "null");
    }
}

bool clUtil_DateRange::set_from_XML(QDomElement &parent, int child_No)
//Read and parse the 'child_No'-th child node that have tag name "clUtil_DateRange".
//'child_No' counts from 0. Returns true iff done.
{
    mDate0 = mDate1 = QDate();

    //
    QDomElement node_date_range
            = nsDomUtil::get_ith_child_element(parent, child_No, "clUtil_DateRange");
    if(node_date_range.isNull())
        return false;

    QString str_date0 = nsDomUtil::get_text_of_first_child_element(node_date_range, "date0");
    QString str_date1 = nsDomUtil::get_text_of_first_child_element(node_date_range, "date1");
    if(str_date0.isEmpty() || str_date1.isEmpty())
        return false;

    // parse
    if(str_date0 == "null")
        return true; //as empty range

    mDate0 = QDate::fromString(str_date0, "yyyy/MM/dd");
    mDate1 = QDate::fromString(str_date1, "yyyy/MM/dd");

    bool good = true;
    if(!mDate0.isValid() || !mDate1.isValid())
        good = false;
    else if(mDate0 > mDate1)
        good = false;

    //
    if(!good)
        mDate0 = QDate();
    return good;
}

////

QList<clUtil_DateRange> DateRange_subtraction(const clUtil_DateRange &DR1,
                                              const clUtil_DateRange &DR2)
//DR1 - DR2
{
    QList<clUtil_DateRange> DR_list;

    if(DR1.is_empty())
        return DR_list;
    // now DR1 is non-empty

    if(! DR1.overlaps(DR2))
    {
        DR_list << DR1;
        return DR_list;
    }
    // now DR1 overlaps DR2

    if(DR1.mDate0 < DR2.mDate0)
        DR_list << clUtil_DateRange(DR1.mDate0, DR2.mDate0.addDays(-1));
    if(DR1.mDate1 > DR2.mDate1)
        DR_list << clUtil_DateRange(DR2.mDate1.addDays(1), DR1.mDate1);
    return DR_list;
}

clUtil_DateRange get_month_including(const QDate &date)
//Return the month, as a date-range, that includes 'date'.
{
    Q_ASSERT(date.isValid());
    return clUtil_DateRange(nsUtilDate::first_date_of_month(date),
                            nsUtilDate::last_date_of_month(date));
}

QList<clUtil_DateRange> split_date_range_by_months(const clUtil_DateRange &date_range)
//Divide 'date_range' into date-ranges, each of which lies within a month.
{
    QList<clUtil_DateRange> DRs;
    if(date_range.is_empty())
        return DRs;

    //
    QDate dm0 = nsUtilDate::first_date_of_month(date_range.get_start_date());
    for( ; dm0<=date_range.get_end_date(); dm0=dm0.addMonths(1))
    {
        clUtil_DateRange R = date_range;
        R.take_intersection_with(get_month_including(dm0));
        DRs << R;
    }

    return DRs;
}
