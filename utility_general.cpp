#include "utility_general.h"

QSet<QString> to_set_of_string(const QStringList &list)
{
    QSet<QString> set;
    foreach(QString s, list)
        set << s;
    return set;
}

QString print_ith(const int i)
{
    QString s = QString::number(i);

    int d = i%10;
    if(d == 1)
        s += "st";
    else if(d == 2)
        s += "nd";
    else if(d == 3)
        s += "rd";
    else
        s += "th";

    return s;
}

void reverse_list(QStringList *list)
{
    reverse_list((QList<QString>*)(list));
}
