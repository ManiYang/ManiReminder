#include "utility_time.h"
#include "utility_general.h"

QString print_time_relative_to_now(const QDateTime &t, const QDateTime &now)
{
    Q_ASSERT(t.isValid() && now.isValid());

    const QString time_str = t.time().toString("hh:mm");

    if(t.date() == now.date())
        return time_str;

    if(t.date().month() == now.date().month())
        return print_ith(t.date().day()) + " " + time_str;

    if(t.date().year() == now.date().year())
        return t.toString("M/d") + " " + time_str;

    return t.toString("yyyy/M/d") + " " + time_str;
}

QString print_xmxs(const int seconds)
{
    if(seconds == 0)
        return "0s";

    int mm = seconds/60;
    int ss = seconds%60;

    QString str;
    if(mm != 0)
        str = QString("%1m").arg(mm);
    if(ss != 0)
        str += QString("%1s").arg(ss);
    return str;
}

QString print_xhxm(const int minutes)
{
    if(minutes == 0)
        return "0m";

    int hh = minutes/60;
    int mm = minutes%60;

    QString str;
    if(hh != 0)
        str += QString("%1s").arg(hh);
    if(mm != 0)
        str = QString("%1m").arg(mm);
    return str;
}
