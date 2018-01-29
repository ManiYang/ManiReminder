#include "UI_Debug_ScheduledActions.h"
#include "ui_UI_Debug_ScheduledActions.h"
#include "utility_widgets.h"

clUI_Debug_ScheduledActions::clUI_Debug_ScheduledActions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clUI_Debug_ScheduledActions)
{
    ui->setupUi(this);
}

clUI_Debug_ScheduledActions::~clUI_Debug_ScheduledActions()
{
    delete ui;
}

void clUI_Debug_ScheduledActions::set_data(const QDateTime &t, const QString &reminder_title,
                                           const QMultiMap<QDateTime,QString> &scheduled_actions)
{
    ui->label->setText( QString("Scheduled actions for reminder \"%1\" at %2")
                        .arg(reminder_title)
                        .arg(t.toString("yyyy/M/d hh:mm:ss")) );

    //
    QStringList column_labels;
    column_labels << "time" << "action";

    //
    QStringList times, actions;
    for(auto it=scheduled_actions.constBegin(); it!=scheduled_actions.constEnd(); it++)
    {
        times << it.key().toString("yyyy/MM/dd hh:mm:ss");
        actions << it.value();
    }

    QList<QStringList> columns;
    columns << times << actions;

    //
    //ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    fill_table_widget(*(ui->tableWidget), columns, column_labels);
}
