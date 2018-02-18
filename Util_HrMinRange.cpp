//20171206

#include <QTime>
#include "Util_HrMinRange.h"
#include "utility_DOM.h"

clUtil_HrMinRange::clUtil_HrMinRange()
    : mHr0(-1)
{
}

clUtil_HrMinRange::clUtil_HrMinRange(const int hr0, const int min0,
                                     const int hr1, const int min1)
{
    Q_ASSERT(0 <= hr0 && hr0 < 24);
    Q_ASSERT(0 <= min0 && min0 < 60);
    Q_ASSERT(0 <= min1 && min1 < 60);
    Q_ASSERT((hr1==hr0 && min1>min0) || hr1>hr0);

    mHr0 = hr0;
    mMin0 = min0;
    mHr1 = hr1;
    mMin1 = min1;
}

int clUtil_HrMinRange::get_start_hr() const
{
    Q_ASSERT(mHr0 != -1);
    return mHr0;
}

int clUtil_HrMinRange::get_start_min() const
{
    Q_ASSERT(mHr0 != -1);
    return mMin0;
}

int clUtil_HrMinRange::get_end_hr() const
{
    Q_ASSERT(mHr0 != -1);
    return mHr1;
}

int clUtil_HrMinRange::get_end_min() const
{
    Q_ASSERT(mHr0 != -1);
    return mMin1;
}

bool clUtil_HrMinRange::parse_and_set(const QString &S_)
//parse 'S_' as "[hr0:min0,hr1:min1)" or "hr0:min0-hr1:min1"
{
#define ON_ERR { mHr0 = -1; return false; }

    QString S = S_.simplified();

    //
    QStringList SS;
    if(S.startsWith('['))
    {
        if(! S.endsWith(')'))
            ON_ERR;
        SS = S.remove(S.length()-1, 1).remove(0, 1).split(',');
    }
    else
    {
        if(! S.contains('-'))
            ON_ERR;
        SS = S.split('-');
    }

    //
    if(SS.count() != 2)
        ON_ERR;

    QStringList SS0 = SS.at(0).split(':');
    QStringList SS1 = SS.at(1).split(':');
    if(SS0.count() != 2 || SS1.count() != 2)
        ON_ERR;

    bool ch, ok;
    mHr0  = SS0.at(0).toInt(&ok); ch = ok;
    mMin0 = SS0.at(1).toInt(&ok); ch &= ok;
    mHr1  = SS1.at(0).toInt(&ok); ch &= ok;
    mMin1 = SS1.at(1).toInt(&ok); ch &= ok;
    if(!ch)
        ON_ERR;

    if(! QTime(mHr0, mMin0).isValid())
        ON_ERR;
    if(mMin1 < 0 || mMin1 > 59)
        ON_ERR;
    if(mHr0>mHr1 || (mHr0==mHr1 && mMin0>=mMin1))
        ON_ERR;

    return true;
#undef ON_ERR
}

void clUtil_HrMinRange::merge_with_overlapping_or_adjacent(const clUtil_HrMinRange &another)
//'another' must overlap or is adjacent to *this.
{
    Q_ASSERT(overlaps_or_is_adjacent_to(another));

    if(another.get_start_minute_number() < get_start_minute_number())
    {
        mHr0 = another.mHr0;
        mMin0 = another.mMin0;
    }

    if(another.get_end_minute_number() > get_end_minute_number())
    {
        mHr1 = another.mHr1;
        mMin1 = another.mMin1;
    }
}

//QString clUtil_HrMinRange::print(const QString to) const
//{
//    Q_ASSERT(mHr0 != -1);
//    QChar z = '0';
//    return QString("%1:%2").arg(mHr0,2,10,z).arg(mMin0,2,10,z)
//            + to + QString("%1:%2").arg(mHr1,2,10,z).arg(mMin1,2,10,z);
//}

QString clUtil_HrMinRange::print(const int option) const
//option -- 1: "[h:mm,h:mm)"
//          2: "h:mm-h:mm"
//          3: "hh:mm-hh:mm"
{
    Q_ASSERT(! is_empty());
    Q_ASSERT(option==1 || option==2 || option==3);

    QString start, end;
    if(option == 3)
    {
        start = QString("%1:%2").arg(mHr0,2,10,QChar('0')).arg(mMin0,2,10,QChar('0'));
        end   = QString("%1:%2").arg(mHr1,2,10,QChar('0')).arg(mMin1,2,10,QChar('0'));
    }
    else
    {
        start = QString("%1:%2").arg(mHr0).arg(mMin0,2,10,QChar('0'));
        end   = QString("%1:%2").arg(mHr1).arg(mMin1,2,10,QChar('0'));
    }

    if(option == 1)
        return QString("[%1,%2)").arg(start, end);
    else
        return QString("%1-%2").arg(start,end);
}

bool clUtil_HrMinRange::includes(const int hr, const int min,
                                const int sec, const int minisec) const
//Returns whether the specified time is within [(mHr0, mMin0), (mHr1, mMin1)).
//'hr' can be >= 24, meaning on the next day or beyond.
{
    if(mHr0 == -1)
        return false;

    Q_ASSERT(hr >= 0);
    Q_ASSERT(0 <= min && min <= 59);
    Q_ASSERT(0 <= sec && sec <= 59);
    Q_ASSERT(0 <= minisec && minisec <= 999);

    return    is_later_or_equal_than_start_time(hr, min, sec, minisec)
           && is_earlier_than_end_time(hr, min, sec, minisec);
}

bool clUtil_HrMinRange::overlaps_or_is_adjacent_to(const clUtil_HrMinRange &another) const
{
    if(is_empty() || another.is_empty())
        return false;

    return another.get_start_minute_number() <= get_end_minute_number()
            && another.get_end_minute_number() >= get_start_minute_number();
}

bool clUtil_HrMinRange::operator == (const clUtil_HrMinRange &another) const
//Empty ranges are considered equal.
{
    if(mHr0 == -1)
        return another.mHr0 == -1;

    return mHr0==another.mHr0 && mMin0==another.mMin0
           && mHr1==another.mHr1 && mMin1==another.mMin1;
}

void clUtil_HrMinRange::get_DateTime_range(const QDate &base_date,
                                           QDateTime &t0, QDateTime &t1) const
// Get date-time interval [t0,t1) represented by *this, regarding (hr=0,min=0)
// as the time 0:00 on 'base_date'.
{
    Q_ASSERT(! is_empty());
    Q_ASSERT(base_date.isValid());

    t0 = QDateTime(base_date, QTime(mHr0, mMin0));

    QDate d = base_date.addDays(mHr1/24);
    t1 = QDateTime(d, QTime(mHr1%24, mMin1));
}

clUtil_TimeRange clUtil_HrMinRange::get_DateTime_range(const QDate &base_date) const
{
    QDateTime t0, t1;
    get_DateTime_range(base_date, t0, t1);
    return clUtil_TimeRange(t0, t1);
}

void clUtil_HrMinRange::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    Q_ASSERT(mHr0 != -1);

    QDomElement node_time_range
            = nsDomUtil::add_child_element(doc, parent, "time_range");

    nsDomUtil::add_child_element_w_text(doc, node_time_range, "hr0",  QString().setNum(mHr0));
    nsDomUtil::add_child_element_w_text(doc, node_time_range, "min0", QString().setNum(mMin0));
    nsDomUtil::add_child_element_w_text(doc, node_time_range, "hr1",  QString().setNum(mHr1));
    nsDomUtil::add_child_element_w_text(doc, node_time_range, "min1", QString().setNum(mMin1));
}

bool clUtil_HrMinRange::set_from_XML(QDomElement &parent, int child_No)
//Read and parse the 'child_No'-th of the child nodes that have target tag name.
//Returns true iff such child node is found. child_No counts from 0.
{
    mHr0 = -1;

    //
    QDomElement node_time_range
            = nsDomUtil::get_ith_child_element(parent, child_No, "time_range");
    if(node_time_range.isNull())
        return false;

    QString str_hr0  = nsDomUtil::get_text_of_first_child_element(node_time_range, "hr0");
    QString str_min0 = nsDomUtil::get_text_of_first_child_element(node_time_range, "min0");
    QString str_hr1  = nsDomUtil::get_text_of_first_child_element(node_time_range, "hr1");
    QString str_min1 = nsDomUtil::get_text_of_first_child_element(node_time_range, "min1");
    Q_ASSERT(   !str_hr0.isEmpty() && !str_min0.isEmpty()
             && !str_hr1.isEmpty() && !str_min1.isEmpty());

    // parse
    bool ch, ok1;
    mHr0  = str_hr0 .toInt(&ok1); ch = ok1;
    mMin0 = str_min0.toInt(&ok1); ch &= ok1;
    mHr1  = str_hr1 .toInt(&ok1); ch &= ok1;
    mMin1 = str_min1.toInt(&ok1); ch &= ok1;
    Q_ASSERT(ch);
    Q_ASSERT(mHr0>=0 && mHr0<=23 && mMin0>=0 && mMin0<=59
                     && mHr1>=0  && mMin1>=0 && mMin1<=59);
    Q_ASSERT(mHr0<mHr1 || (mHr0==mHr1 && mMin0<=mMin1));

    //
    return true;
}

//// private methods ////

bool clUtil_HrMinRange::is_later_or_equal_than_start_time(const int hr, const int min,
                                                  const int sec, const int minisec) const
{
    Q_UNUSED(sec)
    Q_UNUSED(minisec)
    Q_ASSERT(mHr0 != -1);

    if(hr > mHr0)
        return true;
    if(hr < mHr0)
        return false;

    //now hr = mHr0
    if(min > mMin0)
        return true;
    if(min < mMin0)
        return false;

    //now hr = mHr0 and min = mMin0
    return true;
}

bool clUtil_HrMinRange::is_earlier_than_end_time(const int hr, const int min,
                                                const int sec, const int minisec) const
{
    Q_UNUSED(sec)
    Q_UNUSED(minisec)
    Q_ASSERT(mHr0 != -1);

    if(hr > mHr1)
        return false;
    if(hr < mHr1)
        return true;

    //now hr = mHr1
    if(min > mMin1)
        return false;
    if(min < mMin1)
        return true;

    //now hr = mHr1 and min = mMin1
    return false;
}

int clUtil_HrMinRange::get_start_minute_number() const
{
    Q_ASSERT(mHr0 != -1);
    return mHr0*60 + mMin0;
}

int clUtil_HrMinRange::get_end_minute_number() const
{
    Q_ASSERT(mHr0 != -1);
    return mHr1*60 + mMin1;
}

////////////////////////////

void hrmin_range_from_time_range(const clUtil_TimeRange &time_range,
                                 clUtil_HrMinRange *hrmin_range, QDate *base_date)
//Find `*hrmin_range`, `*base_date` such that `*hrmin_range` on `*base_date` is equivalent
//to `time_range`. Accurate only to minutes.
{
    const QDateTime t0 = time_range.left(),
                    t1 = time_range.right();

    *base_date = t0.date();
    int hr0 = t0.time().hour();
    int min0 = t0.time().minute();

    int d = base_date->daysTo(t1.date());
    int hr1 = t1.time().hour() + d*24;
    int min1 = t1.time().minute();

    *hrmin_range = clUtil_HrMinRange(hr0, min0, hr1, min1);
}
