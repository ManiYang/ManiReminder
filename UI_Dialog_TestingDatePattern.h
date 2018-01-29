#ifndef DIALOG_TESTING_DATE_PATTERN_H
#define DIALOG_TESTING_DATE_PATTERN_H

#include <QDialog>
#include "DataElem_DatePattern.h"
#include "UI_MyCalendar.h"

namespace Ui {
class clDialog_Testing_DatePattern;
}

class clDialog_Testing_DatePattern : public QDialog
{
    Q_OBJECT

public:
    explicit clDialog_Testing_DatePattern(QWidget *parent = 0);
    ~clDialog_Testing_DatePattern();

private slots:
    void on_pushButton_previous_clicked();
    void on_pushButton_next_clicked();

    void on_lineEdit_returnPressed();

private:
    Ui::clDialog_Testing_DatePattern *ui;

    clDataElem_DatePattern mDatePattern;
    clUI_MyCalendar *mCalendar;
    int mStartYear;
    int mStartMonth;

    QList<QDate> determine_included_dates_in_current_months();
};

#endif // DIALOG_TESTING_DATE_PATTERN_H
