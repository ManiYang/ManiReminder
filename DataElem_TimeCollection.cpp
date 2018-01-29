#include <algorithm>
#include <QMap>
#include <QDebug>
#include <QMessageBox>
#include "DataElem_TimeCollection.h"

clDataElem_TimeCollection::clDataElem_TimeCollection()
{

}

void clDataElem_TimeCollection::clear()
{
    mDatePattern.clear();
    mTimes.clear();
}

QList<QDateTime> clDataElem_TimeCollection::get_times_within_time_range(const QDateTime &t0,
                                                                           const QDateTime &t1)
//Get the times of `*this` within (`t0`, `t1`].
{
    Q_ASSERT(t0.isValid() && t1.isValid());
    Q_ASSERT(t0 < t1);

    QList<QDateTime> times;

    if(mDatePattern.is_empty())
        return times;

    //
    const QDate d0 = t0.date(), d1 = t1.date();
    for(QDate d=d0; d<=d1; d=d.addDays(1))
    {
        const bool whole_day_within = (d0 < d && d < d1);
        if(mDatePattern.includes(d))
        {
            if(whole_day_within)
            {
                foreach(QTime t, mTimes)
                    times << QDateTime(d, t);
            }
            else
            {
                foreach(QTime t, mTimes)
                {
                    QDateTime dt(d, t);
                    if(t0 < dt && dt <= t1)
                        times << dt;
                }
            }
        }
    }

    return times;
}

bool clDataElem_TimeCollection::operator==(const clDataElem_TimeCollection &another) const
//Empty collections are considered equal.
{
    if(is_empty())
        return another.is_empty();

    if(mDatePattern != another.mDatePattern)
        return false;

    //
    QSet<QTime> T1, T2;
    T1 = QSet<QTime>::fromList(mTimes);
    T2 = QSet<QTime>::fromList(another.mTimes);
    return T1 == T2;
}

QDateTime clDataElem_TimeCollection::get_earliest_time() const
{
    Q_ASSERT(! mDatePattern.is_empty());
    QDate d = mDatePattern.get_earliest_date();
    QTime t = *std::min_element(mTimes.constBegin(), mTimes.constEnd());
    return QDateTime(d, t);
}

bool clDataElem_TimeCollection::parse_and_set(const QString &S)
//format: "<date-pattern>; <h:mm>,<h:mm>,..."
{
    clear();

#define ON_ERR {clear(); return false;}

    //
    QStringList tokens = S.split(';');
    if(tokens.count() != 2)
        ON_ERR;

    //
    bool ch = mDatePattern.parse_and_set(tokens.at(0));
    if(!ch)
        ON_ERR;

    //
    QStringList str_times = tokens.at(1).split(',');
    if(str_times.isEmpty())
        ON_ERR;

    QMap<QTime,bool> times; //the keys will be unique and in ascending order
    foreach(QString str_time, str_times)
    {
        QTime t = QTime::fromString(str_time.simplified(), "h:m");
        if(! t.isValid())
            ON_ERR;
        times.insert(t, false);
    }
    mTimes = times.keys();

    //
    return true;

#undef ON_ERR
}

QString clDataElem_TimeCollection::print() const
{
    if(mDatePattern.is_empty())
        return QString();

    //
    QString S = mDatePattern.print();
    S += "; ";
    for(auto it=mTimes.begin(); it!=mTimes.end(); it++)
    {
        if(it != mTimes.begin())
            S += ',';
        S += (*it).toString("h:mm");
    }

    return S;
}
