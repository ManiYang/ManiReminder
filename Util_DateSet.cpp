//20171123

#include <QDebug>
#include "Util_DateSet.h"
#include "utility_DOM.h"
#include "utility_math.h"
#include "utility_date.h"

clUtil_DateSet::clUtil_DateSet()
{
}

void clUtil_DateSet::add_date_range(const clUtil_DateRange &date_range)
{
    if(date_range.is_empty())
        return;

    if(mDateRanges.isEmpty())
    {
        mDateRanges << date_range;
        return;
    }

    /// find mDateRanges[i] that overlaps or is adjacent to 'date_range'
    /// --> pos_to_merge[]
    QList<int> pos_to_merge;
    for(int i=0; i<mDateRanges.count(); i++)
    {
        if(mDateRanges.at(i).overlaps_or_is_adjacent_to(date_range))
            pos_to_merge << i;
    }

    // check that pos_to_merge[i] are ascending and consecutive.
    for(int i=0; i<pos_to_merge.count(); i++)
    {
        Q_ASSERT(pos_to_merge.at(i) == pos_to_merge.first()+i);
    }

    /// merge mDateRanges[pos_to_merge[i]] and 'date_range'
    /// --> new_range
    clUtil_DateRange new_range = date_range;
    foreach(int pos, pos_to_merge)
        new_range.merge_with(mDateRanges.at(pos));

    /// find largest (smallest) index 'left' ('right') such that mDateRanges[left]
    /// (mDateRanges[right]) does not overlap 'date_range' and is not adjacent to 'date_range'
    const QDate d0 = date_range.get_start_date();
    int left = -1;
    for(int i=0; i<mDateRanges.count(); i++)
    {
        if(pos_to_merge.contains(i))
            break;
        if(mDateRanges.at(i).get_start_date() < d0)
            left = i;
        else
            break;
    }

    int right = -1;
    for(int i=mDateRanges.count()-1; i>=0; i--)
    {
        if(pos_to_merge.contains(i))
            break;
        if(mDateRanges.at(i).get_start_date() > d0)
            right = i;
        else
            break;
    }

    ///
    QList<clUtil_DateRange> new_list;
    if(left != -1)
        new_list << mDateRanges.mid(0, left+1);
    new_list << new_range;
    if(right != -1)
        new_list << mDateRanges.mid(right);

    mDateRanges = new_list;
}

void clUtil_DateSet::add_date_range(const QDate &from, const QDate &to)
{
    Q_ASSERT(from.isValid() && to.isValid());
    Q_ASSERT(from <= to);
    add_date_range(clUtil_DateRange(from, to));
}

void clUtil_DateSet::add_date_range(const int yr, const int mn,
                                    const int day_from, const int day_to)
//Add [day_from, day_to] in the month (yr, mn). Invalid dates are ignored.
{
    QDate date(yr, mn, 1);
    if(! date.isValid())
        return;

    Q_ASSERT(day_from <= day_to);
    const int N = date.daysInMonth();

    const int d0 = max_of_two(1, day_from),
              d1 = min_of_two(N, day_to);
    if(d0 <= d1)
        add_date_range(QDate(yr, mn, d0), QDate(yr, mn, d1));
}

void clUtil_DateSet::add_date_ranges(const QList<clUtil_DateRange> &date_ranges)
{
    foreach(clUtil_DateRange DR, date_ranges)
        add_date_range(DR);
}

void clUtil_DateSet::add_dates(const QDate &from, const qint64 Ndays)
{
    Q_ASSERT(from.isValid());
    Q_ASSERT(Ndays >= 1);
    add_date_range(clUtil_DateRange(from, Ndays));
}

void clUtil_DateSet::add_date(const QDate &date)
{
    Q_ASSERT(date.isValid());
    add_date_range(clUtil_DateRange(date));
}

void clUtil_DateSet::take_union_with(const clUtil_DateSet &another)
{
    foreach(clUtil_DateRange r, another.mDateRanges)
        add_date_range(r);
}

void clUtil_DateSet::take_intersection_with(const clUtil_DateRange &DR)
{
    if(mDateRanges.isEmpty())
        return;
    if(DR.is_empty())
    {
        mDateRanges.clear();
        return;
    }

    //
    QList<clUtil_DateRange> old_date_ranges = mDateRanges;
    mDateRanges.clear();

    foreach(clUtil_DateRange R0, old_date_ranges)
    {
        clUtil_DateRange RR = R0;
        RR.take_intersection_with(DR);
        add_date_range(RR);
    }
}

void clUtil_DateSet::take_intersection_with(const clUtil_DateSet &another)
{
    if(mDateRanges.isEmpty())
        return;
    if(another.mDateRanges.isEmpty())
    {
        mDateRanges.clear();
        return;
    }

    //
    QList<clUtil_DateRange> old_date_ranges = mDateRanges;
    mDateRanges.clear();

    foreach(clUtil_DateRange R0, old_date_ranges)
    {
        foreach(clUtil_DateRange R1, another.mDateRanges)
        {
            clUtil_DateRange RR = R0;
            RR.take_intersection_with(R1);
            add_date_range(RR);
        }
    }
}

void clUtil_DateSet::subtracted_by(const clUtil_DateRange &DR)
{
    if(mDateRanges.isEmpty())
        return;
    if(DR.is_empty())
        return;

    QList<clUtil_DateRange> old_date_ranges = mDateRanges;
    mDateRanges.clear();

    foreach(clUtil_DateRange DR1, old_date_ranges)
        add_date_ranges( DateRange_subtraction(DR1, DR) );
}

void clUtil_DateSet::subtracted_by(const clUtil_DateSet &another)
{
    if(mDateRanges.isEmpty())
        return;
    if(another.mDateRanges.isEmpty())
        return;

    foreach(clUtil_DateRange DR2, another.mDateRanges)
        subtracted_by(DR2);
}

void clUtil_DateSet::clear()
{
    mDateRanges.clear();
}

bool clUtil_DateSet::is_empty() const
{
    return mDateRanges.isEmpty();
}

bool clUtil_DateSet::includes(const QDate &date) const
{
    Q_ASSERT(date.isValid());

    if(! get_span().includes(date))
        return false;

    foreach(clUtil_DateRange r, mDateRanges)
    {
        if(r.includes(date))
            return true;
    }
    return false;
}

bool clUtil_DateSet::includes(const clUtil_DateRange &date_range) const
//'date_range' should not be empty
{
    Q_ASSERT(! date_range.is_empty());

    if(! get_span().is_superset_of(date_range))
        return false;

    foreach(clUtil_DateRange r, mDateRanges)
    {
        if(r.is_superset_of(date_range))
            return true;
    }
    return false;
}

qint64 clUtil_DateSet::get_Ndays() const
{
    qint64 count = 0;
    foreach(clUtil_DateRange r, mDateRanges)
        count += r.get_Ndays();
    return count;
}

QList<QDate> clUtil_DateSet::get_all_dates() const
{
    QList<QDate> dates;
    foreach(clUtil_DateRange date_range, mDateRanges)
        dates << date_range.get_all_dates();
    return dates;
}

clUtil_DateRange clUtil_DateSet::get_span() const
//get the range [earliest date, latest date]
{
    if(mDateRanges.isEmpty())
        return clUtil_DateRange();
    else
        return clUtil_DateRange(mDateRanges.first().get_start_date(),
                                mDateRanges.last().get_end_date());
}

QDate clUtil_DateSet::get_earliest_date() const
//Return null date if empty.
{
    if(mDateRanges.isEmpty())
        return QDate();
    else
        return mDateRanges.first().get_start_date();
}

QDate clUtil_DateSet::get_latest_date() const
//Return null date if empty.
{
    if(mDateRanges.isEmpty())
        return QDate();
    else
        return mDateRanges.last().get_end_date();
}

QString clUtil_DateSet::print(const bool MMDD) const
{
    QString S;
    for(int i=0; i<mDateRanges.count(); i++)
    {
        if(i > 0)
            S += ",";
        S += mDateRanges.at(i).print("-", MMDD);
    }
    return S;
}

bool clUtil_DateSet::parse_and_set(const QString &S)
//Parse 'S' as "YYYY/M/D[-YYYY/M/D],YYYY/M/D[-YYYY/M/D],...".
//Correct "(ddd)" can be added right after any "YYYY/M/D".
//Return true iff successful.
{
    QStringList tokens = S.split(',');
    mDateRanges.clear();
    foreach(QString str, tokens)
    {
        clUtil_DateRange r;
        if(! r.parse_and_set(str))
            return false;
        add_date_range(r);
    }

    return true;
}

void clUtil_DateSet::shift_by_days(const int n)
{
    for(int i=0; i<mDateRanges.count(); i++)
        mDateRanges[i].shift_by_days(n);
}

void clUtil_DateSet::shift_by_months_preserving_dayNo(const int n)
{
    if(n == 0)
        return;

    QList<clUtil_DateRange> old_ranges = mDateRanges;
    mDateRanges.clear();
    foreach(clUtil_DateRange DR, old_ranges)
    {
        DR.shift_by_months_preserving_dayNo(n);
        add_date_range(DR);
    }
}

void clUtil_DateSet::shift_by_months_preserving_dayRevNo(const int n)
{
    if(n == 0)
        return;

    QList<clUtil_DateRange> old_ranges = mDateRanges;
    mDateRanges.clear();
    foreach(clUtil_DateRange DR, old_ranges)
    {
        DR.shift_by_months_preserving_dayRevNo(n);
        add_date_range(DR);
    }
}

void clUtil_DateSet::shift_by_months_preserving_weekNo_et_dayOfWeek(const int n)
{
    if(n == 0)
        return;

    QList<clUtil_DateRange> old_ranges = mDateRanges;
    mDateRanges.clear();
    foreach(clUtil_DateRange DR, old_ranges)
    {
        clUtil_DateSet DS = shift_date_range_by_months_preserving_weekNo_et_dayOfWeek(DR, n);
        take_union_with(DS);
    }
}

void clUtil_DateSet::shift_by_months_preserving_weekRevNo_et_dayOfWeek(const int n)
{
    if(n == 0)
        return;

    QList<clUtil_DateRange> old_ranges = mDateRanges;
    mDateRanges.clear();
    foreach(clUtil_DateRange DR, old_ranges)
    {
        clUtil_DateSet DS = shift_date_range_by_months_preserving_weekRevNo_et_dayOfWeek(DR, n);
        take_union_with(DS);
    }
}

bool clUtil_DateSet::operator == (const clUtil_DateSet &another) const
//empty date sets are considered equal
{
    return mDateRanges == another.mDateRanges;
}

bool clUtil_DateSet::operator != (const clUtil_DateSet &another) const
//empty date sets are considered equal
{
    return mDateRanges != another.mDateRanges;
}

void clUtil_DateSet::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    QDomElement node0 = nsDomUtil::add_child_element(doc, parent, "clUtil_DateSet");
    foreach(clUtil_DateRange R, mDateRanges)
        R.add_to_XML(doc, node0);
}

bool clUtil_DateSet::set_from_XML(QDomElement &parent)
//Read and parse the child node that have tag name "clUtil_DateSet".
//Returns true iff done.
{
    mDateRanges.clear();

    QDomElement node0 = parent.firstChildElement("clUtil_DateSet");
    if(node0.isNull())
        return false;

    for(int i=0; true; i++)
    {
        clUtil_DateRange R;
        bool ch = R.set_from_XML(node0, i);
        if(!ch)
            break;
        mDateRanges << R;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

static void shift_cyclically_within_week(const int d0, const int d1,
                                         const int shift_days, const int week_start_d,
                                         int *dd0, int *dd1, int *dd2, int *dd3)
//Shift [d0,d1] by 'shift_days' cyclically, staying within [week_start_d, week_start_d+6].
//The result is [*dd0,*dd1] U [*dd2,*dd3] if *dd2 != 0, or [*dd0,*dd1] if *dd2 = 0.
//Requirements:
//   * week_start_d <= d0 <= d1 <= week_start_d+6
//   * shift_days in [0,6]
{
    Q_ASSERT(week_start_d<=d0 && d0<=d1 && d1<=week_start_d+6);
    Q_ASSERT(0<=shift_days && shift_days<=6);

    *dd2 = *dd3 = 0;

    if(shift_days == 0)
    {
        *dd0 = d0;
        *dd1 = d1;
    }
    else if(d1 - d0 == 6) //(whole week)
    {
        *dd0 = d0;
        *dd1 = d1;
    }
    else
    {
        *dd0 = d0 + shift_days;
        *dd1 = d1 + shift_days;

        if(*dd0 > week_start_d+6)
        {
            *dd0 -= 7;
            *dd1 -= 7;
        }

        if(*dd1 > week_start_d+6)
        {
            *dd2 = week_start_d;
            *dd3 = *dd1 - 7;
            *dd1 = week_start_d+6;
        }
    }
}

clUtil_DateSet shift_date_range_by_months_preserving_weekNo_et_dayOfWeek(
                                          const clUtil_DateRange &date_range, const int Nmonths)
{
    clUtil_DateSet DS;
    if(date_range.is_empty())
        return DS;

    if(Nmonths == 0)
    {
        DS.add_date_range(date_range);
        return DS;
    }

    // split 'date_range' by months --> date_range_split[]
    QList<clUtil_DateRange> date_range_split = split_date_range_by_months(date_range);

    // shift date_range_split[i] (which is within a month) by 'Nmonths' months; add
    // the results to 'DS'
    foreach(clUtil_DateRange DR, date_range_split)
    {
        const int yr = DR.get_start_date().year();
        const int mn = DR.get_start_date().month();
        const int Ndays_in_month = DR.get_start_date().daysInMonth();
        const int day0 = DR.get_start_date().day();
        const int day1 = DR.get_end_date().day();
        const int dw1 = QDate(yr, mn, 1).dayOfWeek(); //day-of-week of first day

        int yr_new = yr;
        int mn_new = mn + Nmonths;
        nsUtilDate::normalize_yr_mn(&yr_new, &mn_new);
        const int dw1_new = QDate(yr_new, mn_new, 1).dayOfWeek();

        int shift_days = dw1 - dw1_new;
        if(shift_days < 0)
            shift_days += 7; //(now 'shift_days' is 0 ~ 6)

        // consider weeks of the month
        for(int weekNo=0; weekNo<=4; weekNo++)
        {
            const int week_start_day = weekNo*7 + 1;
            if(week_start_day > Ndays_in_month) //(no such week for this month)
                continue;
            const int week_end_day = min_of_two(weekNo*7 + 7, Ndays_in_month);

            // days of 'DR' ([day0, day1]) within this week --> [dd0, dd1]
            int dd0 = max_of_two(week_start_day, day0);
            int dd1 = min_of_two(week_end_day, day1);
            if(dd0 > dd1) //(no such days)
                continue;

            // shift [dd0, dd1] by 'shift_days' cyclically, staying within
            // [week_start_day, week_start_day+6]
            // -->  [dd0_new, dd1_new] U [dd2_new, dd3_new]
            int dd0_new, dd1_new, dd2_new, dd3_new;
            shift_cyclically_within_week(dd0, dd1, shift_days, week_start_day,
                                         &dd0_new, &dd1_new, &dd2_new, &dd3_new);

            // add [dd0_new, dd1_new] and [dd2_new, dd3_new] (if 'dd2_new' > 0) in the new
            // month to 'DS', excluding invalid dates
            DS.add_date_range(yr_new, mn_new, dd0_new, dd1_new);
            if(dd2_new > 0)
                DS.add_date_range(yr_new, mn_new, dd2_new, dd3_new);
        }
    }

    //
    return DS;
}

clUtil_DateSet shift_date_range_by_months_preserving_weekRevNo_et_dayOfWeek(
                                          const clUtil_DateRange &date_range, const int Nmonths)
{
    clUtil_DateSet DS;
    if(date_range.is_empty())
        return DS;

    if(Nmonths == 0)
    {
        DS.add_date_range(date_range);
        return DS;
    }

    // split 'date_range' by months --> date_range_split[]
    QList<clUtil_DateRange> date_range_split = split_date_range_by_months(date_range);

    // shift date_range_split[i] (which is within a month) by 'Nmonths' months, add
    // the results to 'DS'
    foreach(clUtil_DateRange DR, date_range_split)
    {
        const int yr = DR.get_start_date().year();
        const int mn = DR.get_start_date().month();
        const int Ndays_in_month = DR.get_start_date().daysInMonth();
        const int revday0 = nsUtilDate::dayReverseNo_in_month(DR.get_end_date());
        const int revday1 = nsUtilDate::dayReverseNo_in_month(DR.get_start_date());
        const int dw_last = QDate(yr, mn, Ndays_in_month).dayOfWeek(); //day-of-week of last day

        int yr_new = yr;
        int mn_new = mn + Nmonths;
        nsUtilDate::normalize_yr_mn(&yr_new, &mn_new);
        const int Ndays_in_month_new = QDate(yr_new, mn_new, 1).daysInMonth();
        const int dw_last_new = QDate(yr_new, mn_new, Ndays_in_month_new).dayOfWeek();

        int shift_revdays = - dw_last + dw_last_new;
        if(shift_revdays < 0)
            shift_revdays += 7; //(now 'shift_days' is 0 ~ 6)

        // consider weeks of the month
        for(int weekRevNo=0; weekRevNo<=4; weekRevNo++)
        {
            const int week_end_dayRevNo = weekRevNo*7 + 1;
            if(week_end_dayRevNo > Ndays_in_month) //(no such week for this month)
                continue;
            const int week_start_dayRevNo = min_of_two(week_end_dayRevNo+6, Ndays_in_month);

            // days of 'DR' ([revday0, revday1]) within this week --> [rd0, rd1]
            int rd0 = max_of_two(week_end_dayRevNo, revday0);
            int rd1 = min_of_two(week_start_dayRevNo, revday1);
            if(rd0 > rd1) //(no such days)
                continue;

            // shift [rd0, rd1] by 'shift_revdays' cyclically, staying within
            // [week_end_dayRevNo, week_end_dayRevNo+6]
            // -->  [rd0_new, rd1_new] U [rd2_new, rd3_new]
            int rd0_new, rd1_new, rd2_new, rd3_new;
            shift_cyclically_within_week(rd0, rd1, shift_revdays, week_end_dayRevNo,
                                         &rd0_new, &rd1_new, &rd2_new, &rd3_new);

            // add [rd0_new, rd1_new] and [rd2_new, rd3_new] (if 'rd2_new' > 0) in the new
            // month to 'DS', excluding invalid dates
            DS.add_date_range(yr_new, mn_new, Ndays_in_month_new+1-rd1_new,
                                              Ndays_in_month_new+1-rd0_new);
            if(rd2_new > 0)
                DS.add_date_range(yr_new, mn_new, Ndays_in_month_new+1-rd3_new,
                                                  Ndays_in_month_new+1-rd2_new);
        }
    }

    //
    return DS;
}
