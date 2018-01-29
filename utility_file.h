#ifndef UTILITY_FILE_H
#define UTILITY_FILE_H

#include <QString>

bool write_to_file(const QString &fname, const QString &data, QString &error_msg);
     //output encodeing: UTF8

#endif // UTILITY_FILE_H
