//20171123

#include <cmath>
#include "utility_date.h"

namespace nsUtilDate {

int dayReverseNo_in_month(const QDate &date)
{
    Q_ASSERT(date.isValid());
    return date.daysInMonth() + 1 - date.day();
}

QDate date_with_dayReverseNoInMonth(const int year, const int month,
                                    const int dayReverseNo_in_month, int *dayNo)
//Return the date whose day No. counting from the last day of the month is
//'dayReverseNo_in_month'. 'dayReverseNo_in_month' must be >= 1.
//The returned date might be invalid depending on the input.
//If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.
{
    QDate date(year, month, 1);
    if(! date.isValid())
        return QDate();

    int day = date.daysInMonth() + 1 - dayReverseNo_in_month;

    //
    if(dayNo != Q_NULLPTR)
        *dayNo = day;

    date.setDate(year, month, day);
    return date;
}

int weekNo_in_month(const QDate &date)
//Day 1 ~ 7 is the 0th week.
//Result: 0 ~ 4.
{
    Q_ASSERT(date.isValid());
    return (date.day()-1)/7;
}

int weekReverseNo_in_month(const QDate &date)
//For last 7 days of the month, the result will be 0.
//Result: 0 ~ 4.
{
    Q_ASSERT(date.isValid());
    return (date.daysInMonth()-date.day())/7;
}

QDate date_with_weekNoInMonth(const int yr, const int mn,
                              const int week_No_in_month, const int day_of_week, int *dayNo)
//week_No_in_month: 0~4, day_of_week: 1~7
//The 0th week contains the first 7 days of the month.
//The returned date might be invalid depending on the input.
//If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.
{
    Q_ASSERT(week_No_in_month >= 0 && week_No_in_month <= 4);
    Q_ASSERT(day_of_week >= 1 && day_of_week <= 7);

    int dw_1 = QDate(yr, mn, 1).dayOfWeek(); //1~7
    Q_ASSERT(dw_1 != 0); //[(yr,mn) is not invalid]

    int sh = day_of_week - dw_1;
    if(sh < 0)
        sh += 7;

    const int day = 1 + sh + week_No_in_month*7;

    if(dayNo != Q_NULLPTR)
        *dayNo = day;
    return QDate(yr, mn, day);
}

QDate date_with_weekReverseNoInMonth(const int yr, const int mn,
                                     const int week_ReverseNo_in_month, const int day_of_week,
                                     int *dayNo)
//week_ReverseNo_in_month:0~4, day_of_week: 1~7
//week_ReverseNo_in_month = 0 refers to the last 7 days of the month.
//The returned date might be invalid depending on the input.
//If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.
{
    Q_ASSERT(week_ReverseNo_in_month >= 0 && week_ReverseNo_in_month <= 4);
    Q_ASSERT(day_of_week >= 1 && day_of_week <= 7);

    int Ndays = QDate(yr, mn, 1).daysInMonth();
    Q_ASSERT(Ndays != 0); //[(yr,mn) is invalid]
    int dw_last = QDate(yr, mn, Ndays).dayOfWeek(); //1~7

    int sh = day_of_week - dw_last;
    if(sh > 0)
        sh -= 7;

    const int day = Ndays + sh - week_ReverseNo_in_month*7;

    if(dayNo != Q_NULLPTR)
        *dayNo = day;
    return QDate(yr, mn, day);
}

QDate add_months_preserving_dayNoInMonth(const QDate &date, int Nmonths, int *yr, int *mn, int *day)
//The returned date might be invalid depending on the input.
//The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.
{
    Q_ASSERT(date.isValid());

    int Y = date.year();
    int M = date.month() + Nmonths;
    normalize_yr_mn(&Y, &M);

    int D = date.day();

    //
    if(yr != Q_NULLPTR)
        *yr = Y;
    if(mn != Q_NULLPTR)
        *mn = M;
    if(day != Q_NULLPTR)
        *day = D;

    return QDate(Y, M, D);
}

QDate add_months_preserving_dayReverseNoInMonth(const QDate &date, int Nmonths,
                                                int *yr, int *mn, int *day)
//The returned date might be invalid depending on the input.
//The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.
{
    Q_ASSERT(date.isValid());

    int Y = date.year();
    int M = date.month() + Nmonths;
    normalize_yr_mn(&Y, &M);

    QDate date1 = date_with_dayReverseNoInMonth(Y, M, dayReverseNo_in_month(date), day);

    //
    if(yr != Q_NULLPTR)
        *yr = Y;
    if(mn != Q_NULLPTR)
        *mn = M;
    //('*day' is already set above)

    return date1;
}

QDate add_months_preserving_dayOfWeek_et_weekNoInMonth(const QDate &date, int Nmonths,
                                                       int *yr, int *mn, int *day)
//The returned date might be invalid depending on the input.
//The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.
{
    Q_ASSERT(date.isValid());

    int Y = date.year();
    int M = date.month() + Nmonths;
    normalize_yr_mn(&Y, &M);

    QDate date1 = date_with_weekNoInMonth(Y, M, weekNo_in_month(date), date.dayOfWeek(), day);

    //
    if(yr != Q_NULLPTR)
        *yr = Y;
    if(mn != Q_NULLPTR)
        *mn = M;
    //('*day' is already set above)

    return date1;
}

QDate add_months_preserving_dayOfWeek_et_weekReverseNoInMonth(const QDate &date, int Nmonths,
                                                              int *yr, int *mn, int *day)
//The returned date might be invalid depending on the input.
//The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.
{
    Q_ASSERT(date.isValid());

    int Y = date.year();
    int M = date.month() + Nmonths;
    normalize_yr_mn(&Y, &M);

    QDate date1 = date_with_weekReverseNoInMonth(Y, M, weekReverseNo_in_month(date),
                                                 date.dayOfWeek(), day);

    //
    if(yr != Q_NULLPTR)
        *yr = Y;
    if(mn != Q_NULLPTR)
        *mn = M;
    //('*day' is already set above)

    return date1;
}

bool is_head_of_month(const QDate &date)
//Is 'date' the 29th-, 30st-, or 31st-to-last of themonth?
{
    Q_ASSERT(date.isValid());
    return date.day() <= date.daysInMonth() - 28;
}

QDate first_valid_date_after(const int yr, const int mn, const int day)
//'mn' must be in [1,12]
{
    Q_ASSERT(mn>=1 && mn<=12);

    QDate date(yr, mn, 1);
    if(day < 1)
        return date;
    else if(day > date.daysInMonth())
        return date.addMonths(1);
    else
    {
        date = QDate(yr, mn, day);
        Q_ASSERT(date.isValid());
        return date;
    }
}

QDate last_valid_date_before(const int yr, const int mn, const int day)
{
    Q_ASSERT(mn>=1 && mn<=12);

    QDate date(yr, mn, 1);
    int N = date.daysInMonth();
    if(day > N)
        return QDate(yr, mn, N);
    else if(day < 1)
        return date.addDays(-1);
    else
    {
        date = QDate(yr, mn, day);
        Q_ASSERT(date.isValid());
        return date;
    }
}

QDate first_date_of_month(const QDate &d)
{
    Q_ASSERT(d.isValid());
    return QDate(d.year(), d.month(), 1);
}

QDate last_date_of_month(const QDate &d)
{
    Q_ASSERT(d.isValid());
    return QDate(d.year(), d.month(), d.daysInMonth());
}

QDate last_date_of_month(const int year, const int month)
{
    Q_ASSERT(month >= 1 && month <= 12);

    QDate d(year, month, 1);
    Q_ASSERT(d.isValid());

    return QDate(year, month, d.daysInMonth());
}

void normalize_yr_mn(int *yr, int *mn)
{
    if(*mn > 12)
    {
        *yr += (*mn - 1)/12;
        *mn = (*mn - 1)%12 + 1;
    }
    else if(*mn < 1)
    {
        int dyr = int(ceil((1.0 - (*mn))/12));
        *yr -= dyr;
        *mn += 12*dyr;
    }
}

QDate parse_as_date(const QString &S)
//Parse 'S' as "YYYY/M/D" or "YYYY/M/D(ddd)". Return invalid date if not successful.
{
    int pos = S.indexOf('(');
    if(pos == -1)
        return QDate::fromString(S, "yyyy/M/d");

    //
    QString S1 = S.left(pos);
    QDate date = QDate::fromString(S1, "yyyy/M/d");
    if(! date.isValid())
        return date;

    QString S2 = S.mid(pos);
    int dow;
    if     (S2 == "(Mon)") dow = 1;
    else if(S2 == "(Tue)") dow = 2;
    else if(S2 == "(Wed)") dow = 3;
    else if(S2 == "(Thu)") dow = 4;
    else if(S2 == "(Fri)") dow = 5;
    else if(S2 == "(Sat)") dow = 6;
    else if(S2 == "(Sun)") dow = 7;
    else
        return QDate();

    if(dow != date.dayOfWeek())
        return QDate();

    return date;
}

} //namespace nsUtilDate
