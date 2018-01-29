#include "UI_Dialog_CalendarDatePicker.h"
#include "ui_UI_Dialog_CalendarDatePicker.h"

clUI_Dialog_CalendarDatePicker::clUI_Dialog_CalendarDatePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clUI_Dialog_CalendarDatePicker)
{
    ui->setupUi(this);
}

clUI_Dialog_CalendarDatePicker::~clUI_Dialog_CalendarDatePicker()
{
    delete ui;
}

void clUI_Dialog_CalendarDatePicker::set_label_text(const QString &text)
{
    ui->label->setText(text);
}

void clUI_Dialog_CalendarDatePicker::set_date(const QDate &date)
{
    ui->calendarWidget->setSelectedDate(date);
}


void clUI_Dialog_CalendarDatePicker::set_minimum_date(const QDate &minimum_date)
{
    ui->calendarWidget->setMinimumDate(minimum_date);
}

void clUI_Dialog_CalendarDatePicker::set_maximum_date(const QDate &maximum_date)
{
    ui->calendarWidget->setMaximumDate(maximum_date);
}

QDate clUI_Dialog_CalendarDatePicker::get_selected_date() const
{
    return ui->calendarWidget->selectedDate();
}
