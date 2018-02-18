#ifndef UTILITY_GENERAL_H
#define UTILITY_GENERAL_H

#include <algorithm>
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

//
template <class T>
void remove_elements_from_list(QList<T> &list, const QSet<int> &indices)
//remove `list[indices[]]`
{
    QList<int> ind = indices.toList();
    std::sort(ind.begin(), ind.end());
    reverse_list(&ind); //(now `ind[]` is in descending order)
    for(int i=0; i<ind.size(); i++)
        list.removeAt(ind.at(i));
}

#endif // UTILITY_GENERAL_H
