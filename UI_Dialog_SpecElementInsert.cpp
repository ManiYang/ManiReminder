#include <QDate>
#include "UI_Dialog_SpecElementInsert.h"
#include "ui_UI_Dialog_SpecElementInsert.h"
#include "UI_Dialog_CalendarDatePicker.h"

clUI_Dialog_SpecElementInsert::clUI_Dialog_SpecElementInsert(const QSet<QString> *p_Situations,
                                                             const QSet<QString> *p_Events,
                                                             QWidget *parent)
    : QDialog(parent), ui(new Ui::clUI_Dialog_SpecElementInsert)
{
    ui->setupUi(this);

    // populate combo-boxes
    ui->comboBox_events->addItems(QStringList(p_Events->toList()));
    ui->comboBox_situations->addItems(QStringList(p_Situations->toList()));

    // connections
    connect(ui->comboBox_events, SIGNAL(activated(QString)),
            this, SLOT(event_selected(QString)));
    connect(ui->comboBox_situations, SIGNAL(activated(QString)),
            this, SLOT(situation_selected(QString)));
}

clUI_Dialog_SpecElementInsert::~clUI_Dialog_SpecElementInsert()
{
    delete ui;
}

void clUI_Dialog_SpecElementInsert::on_pushButton_today_clicked()
{
    mResult = QDate::currentDate().toString("yyyy/M/d(ddd)");
    accept();
}

void clUI_Dialog_SpecElementInsert::on_pushButton_dates_clicked()
{
    clUI_Dialog_CalendarDatePicker dialog;
    dialog.set_label_text("Selecte date:");
    dialog.set_date(QDate::currentDate());
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        mResult = dialog.get_selected_date().toString("yyyy/M/d(ddd)");
        accept();
    }
}

void clUI_Dialog_SpecElementInsert::situation_selected(const QString &sit)
{
    mResult = sit;
    accept();
}

void clUI_Dialog_SpecElementInsert::event_selected(const QString &event)
{
    mResult = event;
    accept();
}
