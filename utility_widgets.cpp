#include <QDebug>
#include "utility_widgets.h"

void fill_table_widget(QTableWidget &table_widget,
                       const QList<QStringList> &columns, const QStringList &column_labels)
//Fill the QTableWidget with contents `columns` and header labels `column_labels`.
//`columns` and `column_labels` must have the same size.
//All `columns[i]` must have the same size.
{
    Q_ASSERT(columns.count() == column_labels.count());

    table_widget.clear();
    table_widget.setRowCount(0);
    table_widget.setHorizontalHeaderLabels(column_labels);

    if(columns.isEmpty())
        return;
    if(columns.first().isEmpty())
        return;

    //
    const int Nrows = columns.first().count();
    const int Ncols = columns.count();

    //
    table_widget.setRowCount(Nrows);
    table_widget.setColumnCount(Ncols);

    for(int c=0; c<Ncols; c++)
    {
        Q_ASSERT(columns.at(c).count() == Nrows);
        for(int r=0; r<Nrows; r++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(columns.at(c).at(r));
            table_widget.setItem(r, c, item);
        }
    }
}

void add_table_widget_row(QTableWidget &table_widget, const QStringList &row_cells)
{
    int Ncol = table_widget.columnCount();
    int Nrow = table_widget.rowCount();

    if(Ncol == 0 || Nrow == 0)
    {
        Nrow = 0;
        table_widget.setRowCount(Nrow);

        Ncol = row_cells.size();
        table_widget.setColumnCount(Ncol);
    }
    else
        Q_ASSERT(row_cells.size() == Ncol);

    table_widget.setRowCount(++Nrow);

    for(int c=0; c<Ncol; c++)
        table_widget.setItem(Nrow-1, c, new QTableWidgetItem(row_cells.at(c)));
}

void insert_table_widget_row(QTableWidget &table_widget, const int at_row,
                             const QStringList &row_cells)
{
    if(table_widget.columnCount() == 0 || table_widget.rowCount() == 0)
    {
        table_widget.setRowCount(0);
        table_widget.setColumnCount(row_cells.size());
    }

    Q_ASSERT(row_cells.size() == table_widget.columnCount());
    Q_ASSERT(at_row >= 0 && at_row <= table_widget.rowCount());

    table_widget.insertRow(at_row);
    for(int c=0; c<row_cells.size(); c++)
        table_widget.setItem(at_row, c, new QTableWidgetItem(row_cells.at(c)));
}
