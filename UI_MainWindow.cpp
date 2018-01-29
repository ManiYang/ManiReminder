#include <QDesktopWidget>
#include <QMessageBox>
#include <QInputDialog>
#include "UI_MainWindow.h"
#include "ui_UI_MainWindow.h"
#include "UI_Debug_ScheduledActions.h"

clUI_MainWindow::clUI_MainWindow(const QMap<int, const clReminder*> *p_reminders,
                                 const QSet<QString> *p_situations,
                                 const QSet<QString> *p_events, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::clUI_MainWindow),
      pReminders(p_reminders), pSituations(p_situations), pEvents(p_events)
{
    ui->setupUi(this);

    //
    resize(QApplication::desktop()->availableGeometry(this).size() * 0.75);
    setWindowTitle("MyReminder");

    //
    ui->tabWidget->setTabText(0, "Board");
    ui->tabWidget->setTabText(1, "Day Plan");
    ui->tabWidget->setTabText(2, "All Reminders");

    ui->tabWidget->setCurrentIndex(0);

    //
    mUI_Board = new clUI_Board(pReminders, pSituations, pEvents, ui->tab_board);
    ui->tab_board->layout()->addWidget(mUI_Board);

    connect(mUI_Board, SIGNAL(gevent_happened(clDataElem_GEvent,QDateTime)),
            this,      SIGNAL(gevent_happened(clDataElem_GEvent,QDateTime)));
    connect(mUI_Board, SIGNAL(to_modify_reminder(int,int,const clReminder*)),
            this,      SIGNAL(to_modify_reminder(int,int,const clReminder*)));
    connect(mUI_Board, SIGNAL(to_add_reminder_record(int,QDateTime,QString)),
            this,      SIGNAL(to_add_reminder_record(int,QDateTime,QString)));

    //
    mUI_DayPlan = new clUI_DayPlan(pReminders, ui->tab_day_plan);
    ui->tab_day_plan->layout()->addWidget(mUI_DayPlan);

    connect(mUI_DayPlan, SIGNAL(to_get_day_planning_status(QDate,QMap<int,clDataElem_ScheduleStatus>*)),
            this,        SIGNAL(to_get_day_planning_status(QDate,QMap<int,clDataElem_ScheduleStatus>*)));
    connect(mUI_DayPlan, SIGNAL(day_planning_status_modified(QDate,QMap<int,clDataElem_ScheduleStatus>)),
            this,        SIGNAL(day_planning_status_modified(QDate,QMap<int,clDataElem_ScheduleStatus>)));

    //
    mUI_AllReminders = new clUI_AllReminders(pReminders, pSituations, pEvents,
                                             ui->tab_all_reminders);
    ui->tab_all_reminders->layout()->addWidget(mUI_AllReminders);

    connect(mUI_AllReminders, SIGNAL(to_modify_reminder(int,int,const clReminder*)),
            this,             SIGNAL(to_modify_reminder(int,int,const clReminder*)));
    connect(mUI_AllReminders, SIGNAL(to_modify_reminder_spec(int,const clReminderSpec*)),
            this,             SIGNAL(to_modify_reminder_spec(int,const clReminderSpec*)));
}

clUI_MainWindow::~clUI_MainWindow()
{
    delete ui;
}

void clUI_MainWindow::start_up()
{
    mUI_AllReminders->update_all_reminders_view();
    mUI_Board->situation_event_list_updated();
    mUI_DayPlan->start_up();
}

void clUI_MainWindow::on_actionQuit_triggered()
{
    close();
}

void clUI_MainWindow::reminder_created(int id)
//Reminder `id` must be empty, having only title.
{
    mUI_AllReminders->add_reminder(id);
}

void clUI_MainWindow::reminder_updated(int id)
{
    mUI_AllReminders->update_reminder_view(id);
    mUI_Board->reminder_updated(id);
    mUI_DayPlan->reminder_updated(id);
}

void clUI_MainWindow::situation_event_list_updated()
{
    mUI_Board->situation_event_list_updated();
}

void clUI_MainWindow::reminder_becomes_active(int id)
{
    mUI_Board->show_reminder(id);
}

void clUI_MainWindow::reminder_becomes_inactive(int id)
{
    mUI_Board->hide_reminder(id);
}

void clUI_MainWindow::on_actionScheduled_Actions_triggered()
{
    int id = mUI_AllReminders->get_current_reminder_id();
    if(id == -1)
    {
        QMessageBox::information(this, "info",
                                 "Please select a reminder in tab \"All Reminders\".");
        return;
    }

    // show dialog
    Q_ASSERT(pReminders->contains(id));
    QDateTime now = QDateTime::currentDateTime();
    QMultiMap<QDateTime,QString> actions = (*pReminders)[id]->get_scheduled_actions();

    clUI_Debug_ScheduledActions dialog(this);
    dialog.set_data(now, (*pReminders)[id]->get_title(), actions);
    dialog.exec();
}

void clUI_MainWindow::on_actionNew_Reminder_triggered()
{
    ui->tabWidget->setCurrentIndex(2);

    QInputDialog dialog;
    dialog.setLabelText("Title for new reminder:");
    dialog.setInputMode(QInputDialog::TextInput);

    dialog.resize(300,100);
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        QString title = dialog.textValue().simplified();
        emit to_create_new_reminder(title);
    }
}
