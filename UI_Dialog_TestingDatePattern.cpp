#include <QMessageBox>
#include "UI_Dialog_TestingDatePattern.h"
#include "ui_UI_Dialog_TestingDatePattern.h"
#include "utility_date.h"

clDialog_Testing_DatePattern::clDialog_Testing_DatePattern(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clDialog_Testing_DatePattern)
{
    ui->setupUi(this);

    //
    mCalendar = new clUI_MyCalendar(this);
    ui->horizontalLayout->insertWidget(1, mCalendar);

    //
    QDate today = QDate::currentDate();
    mStartYear = today.year();
    mStartMonth = today.month();
    mCalendar->set_months_to_show(mStartYear, mStartMonth, 4);

    //
    ui->buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(false);
    ui->label_message->setText("Empty date pattern");
}

clDialog_Testing_DatePattern::~clDialog_Testing_DatePattern()
{
    delete ui;
}

void clDialog_Testing_DatePattern::on_pushButton_previous_clicked()
{
    mStartMonth -= 3;
    if(mStartMonth < 1)
    {
        mStartMonth += 12;
        mStartYear--;
    }
    mCalendar->set_months_to_show(mStartYear, mStartMonth, 4);

    QList<QDate> dates = determine_included_dates_in_current_months();
    mCalendar->set_dates_to_highlight(dates);
}

void clDialog_Testing_DatePattern::on_pushButton_next_clicked()
{
    mStartMonth += 3;
    if(mStartMonth > 12)
    {
        mStartMonth -= 12;
        mStartYear++;
    }
    mCalendar->set_months_to_show(mStartYear, mStartMonth, 4);

    QList<QDate> dates = determine_included_dates_in_current_months();
    mCalendar->set_dates_to_highlight(dates);
}

void clDialog_Testing_DatePattern::on_lineEdit_returnPressed()
{
    QString S = ui->lineEdit->text();
    bool good = mDatePattern.parse_and_set(S);
    if(!good)
    {
        QMessageBox::warning(this, "warning", "Could not parse the setting.");
        return;
    }

    QList<QDate> dates = determine_included_dates_in_current_months();
    mCalendar->set_dates_to_highlight(dates);
    mCalendar->setFocus();
}

QList<QDate> clDialog_Testing_DatePattern::determine_included_dates_in_current_months()
{
    const QDate first_date = mCalendar->get_first_date();
    const QDate last_date = mCalendar->get_last_date();

    //
    QStringList strMonths;
    QList<QDate> dates_included;
    QDate M0 = nsUtilDate::first_date_of_month(first_date);
    for( ; M0<=last_date; M0=M0.addMonths(1))
    {
        QList<int> days = mDatePattern.days_within_month(M0.year(), M0.month());
        foreach(int d, days)
            dates_included << QDate(M0.year(), M0.month(), d);

        //
        strMonths << M0.toString("MMM");
    }

    // for all dates d in the months, check whether mDatePattern.includes(d)
    // equals dates_included.contains(d)
    if(mDatePattern.is_empty())
        ui->label_message->setText("Empty date pattern");
    else
    {
        bool good = true;

        QDate D0 = nsUtilDate::first_date_of_month(first_date);
        QDate D1 = nsUtilDate::last_date_of_month(last_date);
        for(QDate d=D0; d<=D1; d=d.addDays(1))
        {
            if(mDatePattern.includes(d) != dates_included.contains(d))
            {
                QString message = "Check failed for\n"+d.toString("yyyy/M/d");
                ui->label_message->setText(message);
                good = false;
                break;
            }
        }

        //
        if(good)
        {
            QString message = "Checked all dates\nin months:";
            foreach(QString str, strMonths)
                message += " \n"+str;
            ui->label_message->setText(message);
        }
    }

    //
    return dates_included;
}
