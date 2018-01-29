#include <QDebug>
#include "UI_MyCalendar.h"
#include "ui_UI_MyCalendar.h"
#include "utility_date.h"

clUI_MyCalendar::clUI_MyCalendar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::clUI_MyCalendar)
{
    ui->setupUi(this);

    //
    ui->tableWidget->setColumnCount(8);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << ""
                                               << "日" << "ㄧ" << "二" << "三"
                                               << "四" << "五" << "六");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setVisible(false);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    //
    set_months_to_show(2017, 1, 3);
}

clUI_MyCalendar::~clUI_MyCalendar()
{
    delete ui;
}

void clUI_MyCalendar::set_months_to_show(const int start_year, const int start_month,
                                         const int Nmonths)
{
    Q_ASSERT(start_month >= 1 && start_month <= 12);
    Q_ASSERT(Nmonths >= 1);

    mStartYear = start_year;
    mStartMonth = start_month;
    mNMonths = Nmonths;

    // determine (end_year, end_month)
    int end_year = start_year;
    int end_month = start_month + Nmonths - 1;
    while(end_month > 12)
    {
        end_month -= 12;
        end_year++;
    }

    // determine number of weeks to be shown --> 'NWeeks'
    QDate date0(start_year, start_month, 1);
    Q_ASSERT(date0.isValid());
    QDate date1 = nsUtilDate::last_date_of_month(end_year, end_month);

    int dow0 = date0.dayOfWeek();
    int dow1 = date1.dayOfWeek();

    QDate date00 = date0.addDays((dow0<7) ? (-dow0) : 0);
    QDate date11 = date1.addDays((dow1<7) ? (6-dow1) : 6);
    Q_ASSERT(date00.dayOfWeek() == 7);
    Q_ASSERT(date11.dayOfWeek() == 6);

    int NWeeks = (date00.daysTo(date11) + 1)/7;
    ui->tableWidget->setRowCount(NWeeks);

    mFirstDate = date00;
    mLastDate = date11;

    // fill in the table
    QBrush brush1(QColor(255,255,255));
    QBrush brush2(QColor(230,230,230));

    QDate date = date00;
    for(int row=0; row<NWeeks; row++)
    {
        QTableWidgetItem *it_col0 = ui->tableWidget->item(row, 0);
        if(it_col0 == Q_NULLPTR)
        {
            it_col0 = new QTableWidgetItem;
            ui->tableWidget->setItem(row, 0, it_col0);
        }
        if((date.month() - start_month)%2 == 0)
            it_col0->setBackground(brush1);
        else
            it_col0->setBackground(brush2);

        //
        for(int col=1; col<=7; col++)
        {
            QTableWidgetItem *it = ui->tableWidget->item(row, col);
            if(it == Q_NULLPTR)
            {
                it = new QTableWidgetItem;
                ui->tableWidget->setItem(row, col, it);
            }
            it->setText( date.toString("d") );
            it->setFont(this->font());
            it->setForeground(QBrush(QColor(Qt::black)));
            it->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

            if((date.month() - start_month)%2 == 0)
                it->setBackground(brush1);
            else
                it->setBackground(brush2);

            //
            date = date.addDays(1);
        }

        //
        if(date.day() >= 8 && date.day() <= 14)
        {
            it_col0->setText( date.toString("yyyy,MMM") );

            QFont f = this->font();
            f.setBold(true);
            it_col0->setFont(f);

            it_col0->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else
            it_col0->setText("");
    }

    //
    highlight_dates();
}

void clUI_MyCalendar::set_dates_to_highlight(const QList<QDate> &dates)

{
    mDatesHighlight = dates;
    clear_highlight();
    highlight_dates();
}

void clUI_MyCalendar::highlight_dates()
{
    QFont bold_font = this->font();
    bold_font.setBold(true);

    const QColor highlight_color(Qt::red);

    //
    foreach(QDate d, mDatesHighlight)
    {
        if(d < mFirstDate || d > mLastDate)
            continue;

        // get (column, row) that holds the date 'd'
        int diff = mFirstDate.daysTo(d);
        int row = diff/7;
        int column = diff%7 + 1;
        QTableWidgetItem *it = ui->tableWidget->item(row, column);
        if(it != Q_NULLPTR)
        {
            it->setForeground(QBrush(highlight_color));
            it->setFont(bold_font);
        }
    }
}

void clUI_MyCalendar::clear_highlight()
{
    for(int row=0; row<ui->tableWidget->rowCount(); row++)
    {
        for(int col=1; col<8; col++)
        {
            QTableWidgetItem *it = ui->tableWidget->item(row, col);
            it->setFont(this->font());
            it->setForeground(QBrush(QColor(Qt::black)));
        }
    }
}
