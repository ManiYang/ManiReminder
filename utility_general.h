#ifndef UTILITY_GENERAL_H
#define UTILITY_GENERAL_H

#include <QList>
#include <QSet>
#include <QStringList>

QString print_ith(const int i);

//
QSet<QString> to_set_of_string(const QStringList &list);

//
template <class T>
void reverse_list(QList<T> *list)
{
    const int n = list->size();
    for(int i=0; i<n/2; i++)
        list->swap(i, n-1-i);
}

void reverse_list(QStringList *list);


#endif // UTILITY_GENERAL_H
