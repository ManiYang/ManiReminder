#ifndef UTILITY_WIDGETS_H
#define UTILITY_WIDGETS_H

#include <QList>
#include <QStringList>
#include <QTableWidget>

void fill_table_widget(QTableWidget &table_widget,
                       const QList<QStringList> &columns, const QStringList &column_labels);

void add_table_widget_row(QTableWidget &table_widget, const QStringList &row_cells);

void insert_table_widget_row(QTableWidget &table_widget, const int at_row,
                             const QStringList &row_cells);

#endif // UTILITY_WIDGETS_H
