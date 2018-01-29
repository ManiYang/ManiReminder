#ifndef DIALOG_CALENDARDATEPICKER_H
#define DIALOG_CALENDARDATEPICKER_H

#include <QDialog>
#include <QString>
#include <QDate>

namespace Ui {
class clUI_Dialog_CalendarDatePicker;
}

class clUI_Dialog_CalendarDatePicker : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_Dialog_CalendarDatePicker(QWidget *parent = 0);
    ~clUI_Dialog_CalendarDatePicker();

    void set_label_text(const QString &text);
    void set_date(const QDate &date);
    void set_minimum_date(const QDate &minimum_date);
    void set_maximum_date(const QDate &maximum_date);

    //
    QDate get_selected_date() const;

private:
    Ui::clUI_Dialog_CalendarDatePicker *ui;
};

#endif // DIALOG_CALENDARDATEPICKER_H
