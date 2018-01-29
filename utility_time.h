#ifndef UTILITY_TIME_H
#define UTILITY_TIME_H

#include <QString>
#include <QDateTime>

QString print_time_relative_to_now(const QDateTime &t, const QDateTime &now);

QString print_xmxs(const int seconds);
QString print_xhxm(const int minutes);

#endif // UTILITY_TIME_H
