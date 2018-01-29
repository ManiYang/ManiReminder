#ifndef UI_MY_CALENDAR_H
#define UI_MY_CALENDAR_H

#include <QFrame>
#include <QList>
#include <QDate>

namespace Ui {
class clUI_MyCalendar;
}

class clUI_MyCalendar : public QFrame
{
    Q_OBJECT

public:
    explicit clUI_MyCalendar(QWidget *parent = 0);
    ~clUI_MyCalendar();

    void set_months_to_show(const int start_year, const int start_month, const int Nmonths);
    void set_dates_to_highlight(const QList<QDate> &dates); //clear original highlight

    QDate get_first_date() const { return mFirstDate; }
    QDate get_last_date() const { return mLastDate; }

private:
    Ui::clUI_MyCalendar *ui;

    int mNMonths;
    int mStartYear;
    int mStartMonth;

    QDate mFirstDate;
    QDate mLastDate;

    QList<QDate> mDatesHighlight;

    void highlight_dates();
    void clear_highlight();
};

#endif // UI_MY_CALENDAR_H
