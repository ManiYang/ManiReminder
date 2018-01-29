#include <cmath>
#include <QMap>
#include <QDebug>
#include "DataElem_DatePattern.h"
#include "utility_date.h"

clDataElem_DatePattern::clDataElem_DatePattern()
    : mRepeatType(NoRepeat), mRepeatEvery(1)
{
}

bool clDataElem_DatePattern::parse_and_set(const QString &S_)
//Format of 'S':
//   <base-date-set> [repeat every <method> [final <final-date>]]
//   daily [final <final-date>]
//where <method> can be
//   day, <N> days, week, <N> weeks,
//   month by <month-method>, <N> months by <month-method>,
//   year, <N> years
//where <month-method> can be dayNo, dayRevNo, weekNo, weekRevNo
//The format for a date is "YYYY/M/D" or "YYYY/M/D(ddd)".
{
    const QString S = S_.simplified();

    ///
    if(S.startsWith("daily"))
    {
        mBaseDateSet.clear();
        mBaseDateSet.add_date(QDate(2017,1,1));
        mRepeatType = Day;
        mRepeatEvery = 1;

        if(S == "daily")
            mFinalDate = QDate();
        else
        {
            if(! S.startsWith("daily final "))
                return false;
            mFinalDate = nsUtilDate::parse_as_date(S.mid(12));
            if(! mFinalDate.isValid())
                return false;
        }

        return true;
    }

    ///
    QString strBaseDateSet, strRepeatMethod, strFinalDate;

    int pos1 = S.indexOf(" repeat every ");
    strBaseDateSet = (pos1 != -1) ? S.left(pos1) : S;

    if(pos1 != -1)
    {
        int pos2 = S.indexOf(" final ");
        if(pos2 != -1)
        {
            if(pos2 <= pos1+14)
                return false;
        }

        strRepeatMethod = (pos2 != -1) ? S.mid(pos1+14, pos2-pos1-14) : S.mid(pos1+14);
        if(strRepeatMethod.isEmpty())
            return false;

        if(pos2 != -1)
        {
            strFinalDate = S.mid(pos2+7);
            if(strFinalDate.isEmpty())
                return false;
        }
    }

    //qDebug() << strBaseDateSet << strRepeatMethod << strFinalDate; //[debug]

    // parse 'strBaseDateSet'
    bool ch = mBaseDateSet.parse_and_set(strBaseDateSet);
    if(!ch)
        return false;

    //
    if(strRepeatMethod.isEmpty())
    {
        mRepeatType = NoRepeat;
        return true;
    }

    // parse 'strRepeatMethod'
    mRepeatEvery = 1;
    QString strMethodUnit = strRepeatMethod;

    int pos = strRepeatMethod.indexOf(' ');
    if(pos != -1)
    {
        QString token1 = strRepeatMethod.left(pos);
        if(token1 != "month")
        {
            bool ok;
            int N = token1.toInt(&ok);
            if(!ok)
                return false;
            if(N < 1)
                return false;

            mRepeatEvery = N;
            strMethodUnit = strRepeatMethod.mid(pos+1);
        }
    }

    if(strMethodUnit == "day" || strMethodUnit == "days")
        mRepeatType = Day;
    else if(strMethodUnit == "week" || strMethodUnit == "weeks")
        mRepeatType = Week;
    else if(strMethodUnit == "month by dayNo" || strMethodUnit == "months by dayNo")
        mRepeatType = Month_DayNo;
    else if(strMethodUnit == "month by dayRevNo" || strMethodUnit == "months by dayRevNo")
        mRepeatType = Month_DayRevNo;
    else if(strMethodUnit == "month by weekNo" || strMethodUnit == "months by weekNo")
        mRepeatType = Month_WeekNo;
    else if(strMethodUnit == "month by weekRevNo" || strMethodUnit == "months by weekRevNo")
        mRepeatType = Month_WeekRevNo;
    else if(strMethodUnit == "year" || strMethodUnit == "years")
        mRepeatType = Year;
    else
        return false;

    //
    if(strFinalDate.isEmpty())
    {
        mFinalDate = QDate();
        return true;
    }

    // parse 'strFinalDate'
    mFinalDate = nsUtilDate::parse_as_date(strFinalDate);
    if(! mFinalDate.isValid())
        return false;

    //
    return true;
}

QString clDataElem_DatePattern::print() const
//The returned string is parsable by this->parse_and_set().
//*this cannot be empty.
{
    Q_ASSERT(! is_empty());

    // daily
    if(mRepeatType == Day && mRepeatEvery == 1 && mBaseDateSet.get_Ndays() == 1)
    {
        QDate date = mBaseDateSet.get_earliest_date();
        if(date <= QDate(2017,1,1))
        {
            QString S = "daily";
            if(mFinalDate.isValid())
                S += QString(" final %1").arg(mFinalDate.toString("yyyy/M/d"));
            return S;
        }
    }

    //
    QString S = mBaseDateSet.print();
    if(mRepeatType == NoRepeat)
        return S;

    //
    S += " repeat every ";

#define ADD_REPEAT_METHOD(unit) if(mRepeatEvery == 1) S += unit; \
                                else S += QString("%1 " unit "s").arg(mRepeatEvery);

    switch(mRepeatType)
    {
    case Day:
        ADD_REPEAT_METHOD("day");
        break;

    case Week:
        ADD_REPEAT_METHOD("week");
        break;

    case Month_DayNo:
    case Month_DayRevNo:
    case Month_WeekNo:
    case Month_WeekRevNo:
        ADD_REPEAT_METHOD("month");
        if(mRepeatType == Month_DayNo)
            S += " by dayNo";
        else if(mRepeatType == Month_DayRevNo)
            S += " by dayRevNo";
        else if(mRepeatType == Month_WeekNo)
            S += " by weekNo";
        else if(mRepeatType == Month_WeekRevNo)
            S += " by weekRevNo";
        break;

    case Year:
        ADD_REPEAT_METHOD("year");
        break;

    default:
        break;
    }

#undef ADD_REPEAT_METHOD

    //
    if(mFinalDate.isValid())
        S += QString(" final %1").arg(mFinalDate.toString("yyyy/M/d"));

    //
    return S;
}

void clDataElem_DatePattern::clear()
{
    mBaseDateSet.clear();
}

QDate clDataElem_DatePattern::get_earliest_date() const
{
    Q_ASSERT(! is_empty());
    return mBaseDateSet.get_earliest_date();
}

bool clDataElem_DatePattern::includes(const QDate &date) const
{
    Q_ASSERT(date.isValid());

    if(mBaseDateSet.is_empty())
        return false;

    if(date < mBaseDateSet.get_earliest_date())
        return false;

    if(mRepeatType == NoRepeat)
        return mBaseDateSet.includes(date);

    if(mFinalDate.isValid())
    {
        if(date > mFinalDate)
            return false;
    }

    //(now mRepeatType is not NoRepeat, 'date' is >= earliest date of the base date-set,
    // and 'date' is <= mFinalDate)
    Q_ASSERT(mRepeatEvery >= 1);

    if(mRepeatType == Day)
    {
        return date_query__repeating_every_n_days(date, mRepeatEvery);
    }
    else if(mRepeatType == Week)
    {
        return date_query__repeating_every_n_days(date, mRepeatEvery*7);
    }
    else if(mRepeatType == Month_DayNo || mRepeatType == Month_DayRevNo
            || mRepeatType == Month_WeekNo || mRepeatType == Month_WeekRevNo)
    {
        return date_query__repeating_monthly(date);
    }
    else if(mRepeatType == Year)
    {
        return date_query__repeating_yearly(date);
    }
    else
    {
        Q_ASSERT(false); //unrecognized value of 'mRepeatType'
        return false;
    }
}

bool clDataElem_DatePattern::date_query__repeating_every_n_days(const QDate &date,
                                                                   const int n) const
//Determine whether 'date' is included when 'mBaseDateSet' (non-empty) is repeated every n days.
{
    Q_ASSERT(date.isValid());
    Q_ASSERT(n >= 1);
    Q_ASSERT(! mBaseDateSet.is_empty());

    const QDate base_date0 = mBaseDateSet.get_earliest_date();
    const QDate base_date1 = mBaseDateSet.get_latest_date();

    // find all integer i >= 0 such that 'date' is in [base_date0 + n*i, base_date1 + n*i]
    // (ith repeat of the span of base date-set)
    // --> i_min, i_max
    int i_min, i_max;
    {
        double diff_tmp;
        diff_tmp = base_date1.daysTo(date);
        i_min = int( ceil(diff_tmp/n) );

        diff_tmp = base_date0.daysTo(date);
        i_max = int( floor(diff_tmp/n) );

        if(i_min > i_max) //(no such i)
            return false;
        if(i_max < 0)
            return false; //(no such i)

        if(i_min < 0)
            i_min = 0;
    }

    // for each i >= 0 such that 'date' is included in ith repeat of the span of base date-set
    for(int i=i_min; i<=i_max; i++)
    {
        if(mBaseDateSet.includes( date.addDays(-n*i) ))
            return true;
    }

    return false;
}

bool clDataElem_DatePattern::date_query__repeating_monthly(const QDate &date) const
//Determine whether 'date' is included when 'mBaseDateSet' is repeated according to
//'mRepeatType' (Month_DayNo, Month_DayRevNo, Month_WeekNo, or Month_WeekRevNo) every
//'mRepeatEvery' months infinitely.
{
    Q_ASSERT(date.isValid());
    Q_ASSERT(mRepeatEvery >= 1);
    Q_ASSERT(! mBaseDateSet.is_empty());
    Q_ASSERT(mRepeatType==Month_DayNo || mRepeatType==Month_DayRevNo
             || mRepeatType==Month_WeekNo || mRepeatType==Month_WeekRevNo);

    // split 'mBaseDateSet' by months --> baseDateSet_split[]
    const QDate base_date0 = mBaseDateSet.get_earliest_date();
    const QDate base_date1 = mBaseDateSet.get_latest_date();

    QList<clUtil_DateSet> baseDateSet_split;
    {
        QDate M(base_date0.year(), base_date0.month(), 1);
        for( ; M<=base_date1; M=M.addMonths(1))
        {
            QDate M1(M.year(), M.month(), M.daysInMonth());

            clUtil_DateSet DS = mBaseDateSet;
            DS.take_intersection_with(clUtil_DateRange(M, M1));

            baseDateSet_split << DS;
        }
    }

    //
    foreach(clUtil_DateSet DS, baseDateSet_split)
    {
        if(date_query__repeating_monthly___base_within_month(date, DS))
            return true;
    }

    return false;
}

bool clDataElem_DatePattern
       ::date_query__repeating_monthly___base_within_month(const QDate &date,
                                                           const clUtil_DateSet &base_dateSet) const
//Determine whether 'date' is included when 'base_dateSet' (within a month) is repeated
//according to 'mRepeatType' (Month_DayNo, Month_DayRevNo, Month_WeekNo, or Month_WeekRevNo)
//every 'mRepeatEvery' months infinitely.
{
    Q_ASSERT(mRepeatEvery >= 1);

    const int yr0 = base_dateSet.get_earliest_date().year(),
              mn0 = base_dateSet.get_earliest_date().month();

    {
        const int yr = date.year(), mn = date.month();
        const int month_diff = (yr - yr0)*12 + (mn - mn0);

        if(month_diff < 0)
            return false;
        if(month_diff % mRepeatEvery != 0)
            return false;
    }

    //
    QDate d_test;
    if(mRepeatType==Month_DayNo)
    {
        d_test = QDate(yr0, mn0, date.day());
    }
    else if(mRepeatType==Month_DayRevNo)
    {
        int dayRevNo = nsUtilDate::dayReverseNo_in_month(date);
        d_test = nsUtilDate::date_with_dayReverseNoInMonth(yr0, mn0, dayRevNo);
    }
    else if(mRepeatType==Month_WeekNo)
    {
        int weekNo = nsUtilDate::weekNo_in_month(date);
        int dw = date.dayOfWeek(); //1-7
        d_test = nsUtilDate::date_with_weekNoInMonth(yr0, mn0, weekNo, dw);
    }
    else if(mRepeatType==Month_WeekRevNo)
    {
        int weekRevNo = nsUtilDate::weekReverseNo_in_month(date);
        int dw = date.dayOfWeek();
        d_test = nsUtilDate::date_with_weekReverseNoInMonth(yr0, mn0, weekRevNo, dw);
    }

    if(! d_test.isValid())
        return false;
    else
        return base_dateSet.includes(d_test);
}

bool clDataElem_DatePattern::date_query__repeating_yearly(const QDate &date) const
//Determine whether 'date' is included when 'mBaseDateSet' is repeated every 'mRepeatEvery'
//years infinitely.
{
    Q_ASSERT(date.isValid());
    Q_ASSERT(! mBaseDateSet.is_empty());
    Q_ASSERT(mRepeatEvery >= 1);

    // split 'mBaseDateSet' by years --> baseDateSet_split
    const QDate base_date0 = mBaseDateSet.get_earliest_date();
    const QDate base_date1 = mBaseDateSet.get_latest_date();

    QList<clUtil_DateSet> baseDateSet_split;
    {
        for(int yr=base_date0.year(); yr<=base_date1.year(); yr++)
        {
            QDate d0(yr, 1, 1), d1(yr, 12, 31);

            clUtil_DateSet DS = mBaseDateSet;
            DS.take_intersection_with(clUtil_DateRange(d0, d1));

            baseDateSet_split << DS;
        }
    }

    //
    foreach(clUtil_DateSet baseDS, baseDateSet_split)
    {
        const int yr0 = baseDS.get_earliest_date().year();

        int year_diff = date.year() - yr0;
        if(year_diff < 0) //('date' is not included in the repeats of year 'yr0')
            continue;
        if(year_diff % mRepeatEvery != 0) //('date' is not included in the repeats of year 'yr0')
            continue;

        //
        QDate d_test(yr0, date.month(), date.day());
        if(d_test.isValid())
        {
            if(baseDS.includes(d_test)) //('date' is not included in the repeats of 'baseDS')
                return true;
        }
    }

    return false;
}

bool clDataElem_DatePattern::operator != (const clDataElem_DatePattern &another) const
{
    return ! (*this == another);
}

bool clDataElem_DatePattern::operator == (const clDataElem_DatePattern &another) const
{
    if(mBaseDateSet != another.mBaseDateSet)
        return false;

    if(mBaseDateSet.is_empty())
        return true;

    //
    if(mRepeatType != another.mRepeatType)
        return false;

    if(mRepeatType == NoRepeat)
        return true;

    //
    if(mRepeatEvery != another.mRepeatEvery)
        return false;

    //
    return mFinalDate == another.mFinalDate; //(invalid dates are equal)
}

QList<int> clDataElem_DatePattern::days_within_month(const int year, const int month) const
//Get the (day numbers of) days within (year, month) that are included.
{
//    qDebug() << mBaseDateSet.print();
//    qDebug() << mRepeatType;
//    qDebug() << mRepeatEvery;
//    qDebug() << mFinalDate;
//    qDebug() << "query for " << year << "/" << month;

    if(mBaseDateSet.is_empty())
        return QList<int>();

    //
    Q_ASSERT(1 <= month && month <= 12);

    clUtil_DateRange base_span = mBaseDateSet.get_span();
    clUtil_DateRange the_month = get_month_including(QDate(year, month, 1));

    if(the_month.get_end_date() < base_span.get_start_date())
        return QList<int>(); //empty

    if(mRepeatType!=NoRepeat && mFinalDate.isValid())
    {
        if(the_month.get_start_date() > mFinalDate)
            return QList<int>(); //empty
    }

    //
    if(mRepeatType != NoRepeat)
        Q_ASSERT(mRepeatEvery >= 1);

    QMap<int, bool> days;

    switch(mRepeatType)
    {
    case NoRepeat:
    {
        clUtil_DateSet DS = mBaseDateSet;
        DS.take_intersection_with(the_month);

        QList<QDate> dates = DS.get_all_dates();
        foreach(QDate d, dates)
            days.insert(d.day(), false);
        break;
    }

    case Day:
    case Week:
    {
        const int p = (mRepeatType==Day) ? mRepeatEvery : (mRepeatEvery*7); //repeat period in days

        const QDate d0 = base_span.get_start_date(), d1 = base_span.get_end_date();

        // find all i>=0 such that [d0+i*p, d1+i*p] overlaps with 'the_month'
        int i_min, i_max;
        {
            double diff = d1.daysTo(the_month.get_start_date());
            i_min = int(ceil(diff/p));

            diff = d0.daysTo(the_month.get_end_date());
            i_max = int(floor(diff/p));

            if(i_min > i_max) //(no such i)
                break;
            if(i_max < 0) //(no such i)
                break;
            if(i_min < 0)
                i_min = 0;
        }

        //
        for(int i=i_min; i<=i_max; i++)
        {
            clUtil_DateSet DS = mBaseDateSet;
            DS.shift_by_days(i*p);
            DS.take_intersection_with(the_month);
            if(mFinalDate.isValid())
                DS.take_intersection_with(clUtil_DateRange(the_month.get_start_date(), mFinalDate));

            QList<QDate> dates = DS.get_all_dates();
            foreach(QDate d, dates)
                days.insert(d.day(), false);
        }
        break;
    }

    case Month_DayNo:
    case Month_DayRevNo:
    case Month_WeekNo:
    case Month_WeekRevNo:
    case Year:
    {
        const QDate d0 = base_span.get_start_date(),
                    d1 = base_span.get_end_date();
        const int m0 = d0.year()*12 + d0.month();
        const int m1 = d1.year()*12 + d1.month();
        const int m = year*12 + month;

        const int p = (mRepeatType==Year) ? mRepeatEvery*12 : mRepeatEvery; //repeat period in months

        // find all i>=0 such that 'm' is in [m0+i*p, m1+i*p]
        int i_min, i_max;
        {
            i_min = int(ceil((double(m)-m1)/p));
            i_max = int(floor((double(m)-m0)/p));

            if(i_max < i_min)
                break; //(no such i)
            if(i_max < 0)
                break; //(no such i)
            if(i_min < 0)
                i_min = 0;
        }

        //
        for(int i=i_min; i<=i_max; i++)
        {
            clUtil_DateSet DS = mBaseDateSet;
            if(mRepeatType == Month_DayNo)
                DS.shift_by_months_preserving_dayNo(i*mRepeatEvery);
            else if(mRepeatType == Month_DayRevNo)
                DS.shift_by_months_preserving_dayRevNo(i*mRepeatEvery);
            else if(mRepeatType == Month_WeekNo)
                DS.shift_by_months_preserving_weekNo_et_dayOfWeek(i*mRepeatEvery);
            else if(mRepeatType == Month_WeekRevNo)
                DS.shift_by_months_preserving_weekRevNo_et_dayOfWeek(i*mRepeatEvery);
            else //(Year)
                DS.shift_by_months_preserving_dayNo(i*mRepeatEvery*12);

            DS.take_intersection_with(the_month);
            if(mFinalDate.isValid())
                DS.take_intersection_with(clUtil_DateRange(the_month.get_start_date(), mFinalDate));

            QList<QDate> dates = DS.get_all_dates();
            foreach(QDate d, dates)
                days.insert(d.day(), false);
        }
        break;
    }

    default:
        Q_ASSERT(false); //unrecognized value of mRepeatType
        break;
    }

    //
    return days.keys();
}
