//20171123

#ifndef UTILITY_DATE_H
#define UTILITY_DATE_H

#include <QDate>

namespace nsUtilDate {

int dayReverseNo_in_month(const QDate &date); //1~31

QDate date_with_dayReverseNoInMonth(const int year, const int month,
                                    const int dayReverseNo_in_month,
                                    int *dayNo = Q_NULLPTR);
      //Return the date whose day No. counting from the last day of the month is
      //'dayReverseNo_in_month'. 'dayReverseNo_in_month' must be >= 1.
      //The returned date might be invalid depending on the input.
      //If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.

////

int weekNo_in_month(const QDate &date);
    //Day 1 ~ 7 is the 0th week.
    //Result: 0 ~ 4

int weekReverseNo_in_month(const QDate &date);
    //For last 7 days of the month, the result will be 0.
    //Result: 0 ~ 4.

QDate date_with_weekNoInMonth(const int yr, const int mn,
                              const int week_No_in_month, const int day_of_week,
                              int *dayNo = Q_NULLPTR);
      //'week_No': 0 ~ 4, 'day_of_week': 1 ~ 7
      //The 0th week contains the first 7 days of the month.
      //The returned date might be invalid depending on the input.
      //If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.

QDate date_with_weekReverseNoInMonth(const int yr, const int mn,
                                     const int week_ReverseNo_in_month, const int day_of_week,
                                     int *dayNo = Q_NULLPTR);
      //'week_ReverseNo': 0 ~ 4, 'day_of_week': 1 ~ 7
      //week_ReverseNo_in_month = 0 refers to the last 7 days of the month.
      //The returned date might be invalid depending on the input.
      //If 'dayNo' is not NULL, the resulting day No. will also be copied to *dayNo.

////

QDate add_months_preserving_dayNoInMonth(const QDate &date, int Nmonths,
                                         int *yr=Q_NULLPTR, int *mn=Q_NULLPTR, int *day=Q_NULLPTR);
      //The returned date might be invalid.
      //The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.

QDate add_months_preserving_dayReverseNoInMonth(const QDate &date, int Nmonths,
                                                int *yr=Q_NULLPTR, int *mn=Q_NULLPTR, int *day=Q_NULLPTR);
      //The returned date might be invalid.
      //The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.

QDate add_months_preserving_dayOfWeek_et_weekNoInMonth(const QDate &date, int Nmonths,
                                                       int *yr=Q_NULLPTR, int *mn=Q_NULLPTR,
                                                       int *day=Q_NULLPTR);
      //The returned date might be invalid depending on the input.
      //The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.

QDate add_months_preserving_dayOfWeek_et_weekReverseNoInMonth(const QDate &date, int Nmonths,
                                                              int *yr=Q_NULLPTR, int *mn=Q_NULLPTR,
                                                              int *day=Q_NULLPTR);
      //The returned date might be invalid depending on the input.
      //The resulting date is also copied to (*yr, *mn, *day) if they are not NULL.

////

QDate first_valid_date_after(const int yr, const int mn, const int day);
QDate last_valid_date_before(const int yr, const int mn, const int day);
//'mn' must be in [1,12]

////

bool is_head_of_month(const QDate &date);
     //Is 'date' the 29th-, 30st-, or 31st-to-last of the month?

QDate first_date_of_month(const QDate &d);
QDate last_date_of_month(const QDate &d);
QDate last_date_of_month(const int year, const int month);

void normalize_yr_mn(int *yr, int *mn); //e.g. 2017/15 --> 2018/3, 2017/-2 --> 2016/10

QDate parse_as_date(const QString &S);
      //Parse 'S' as "YYYY/M/D" or "YYYY/M/D(ddd)". Return invalid date if not successful.

} //namespace nsUtilDate

#endif // UTILITY_DATE_H
