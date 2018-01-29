#include <QMessageBox>
#include <QDebug>
#include <QStatusBar>
#include "Control.h"
#include "DataElem_GEvent.h"

clControl::clControl(QObject *parent)
    : QObject(parent)
{
    uMainWindow = new clUI_MainWindow(&mReminders_ReadOnly, &mSituations, &mEvents);

    connect(uMainWindow,   SIGNAL(gevent_happened(clDataElem_GEvent,QDateTime)),
            this, SLOT(MainWindow_gevent_happened(clDataElem_GEvent,QDateTime)));

    connect(uMainWindow,   SIGNAL(to_modify_reminder(int,int,const clReminder*)),
            this, SLOT(MainWindow_to_modify_reminder(int,int,const clReminder*)));
    connect(uMainWindow,   SIGNAL(to_modify_reminder_spec(int,const clReminderSpec*)),
            this, SLOT(MainWindow_to_modify_reminder_spec(int,const clReminderSpec*)));
    connect(uMainWindow,   SIGNAL(to_add_reminder_record(int,QDateTime,QString)),
            this, SLOT(MainWindow_to_add_reminder_record(int,QDateTime,QString)));
    connect(uMainWindow,   SIGNAL(to_create_new_reminder(QString)),
            this, SLOT(MainWindow_to_create_new_reminder(QString)));

    connect(uMainWindow,   SIGNAL(to_get_day_planning_status(QDate,QMap<int,clDataElem_ScheduleStatus>*)),
            this, SLOT(MainWindow_to_get_day_planning_status(QDate,QMap<int,clDataElem_ScheduleStatus>*)));
    connect(uMainWindow,   SIGNAL(day_planning_status_modified(QDate,QMap<int,clDataElem_ScheduleStatus>)),
            this, SLOT(MainWindow_day_planning_status_modified(QDate,QMap<int,clDataElem_ScheduleStatus>)));

    //
    uAlarmClockService = new clAlarmClockService(this);
    connect(uAlarmClockService, SIGNAL(time_reached(QDateTime,int)),
            this, SLOT(AlarmClockService_time_reached(QDateTime,int)));


}

clControl::~clControl()
{
    for(auto it=mReminders.begin(); it!=mReminders.end(); it++)
        it.value()->deleteLater();
}

void clControl::start_up()
{
    uMainWindow->show();

    // get situations & events from file
    mSituations = uFileReadWrite.read_all_situations();
    mEvents = uFileReadWrite.read_all_events();

    // get g-event history from file
    QDateTime t0 = QDateTime::currentDateTime();
    QMultiMap<QDateTime, clDataElem_GEvent> GEventHistory
            = uFileReadWrite.read_gevent_history();

    // get all reminders from file
    QList<clReminder*> reminders = uFileReadWrite.read_all_reminders();
                                   //*reminders[i] are created

    mReminders.clear();
    mReminders_ReadOnly.clear();
    for(int i=0; i<reminders.size(); i++)
    {
        clReminder *rem = reminders[i];
        int id = rem->get_id();
        Q_ASSERT(! mReminders.contains(id)); //should not have duplicated id
        mReminders.insert(id, rem);
        mReminders_ReadOnly.insert(id, rem);

        // build connections
        connect(rem, SIGNAL(set_alarm(int,QDateTime)),
                uAlarmClockService, SLOT(schedule_alarm(int,QDateTime)));
        connect(rem, SIGNAL(become_active(int)),   this, SLOT(reminder_becomes_active(int)));
        connect(rem, SIGNAL(become_inactive(int)), this, SLOT(reminder_becomes_inactive(int)));

        // initialize
        rem->initialize(t0, GEventHistory,
                        QSet<QString>()); //start with no situation selected
    }

    // tell `uMainWindow` to start up
    uMainWindow->start_up();

    // on active reminders
    for(auto it=mReminders.begin(); it!=mReminders.end(); it++)
    {
        if(it.value()->is_active_via_bindings())
            reminder_becomes_active(it.key());
    }

    //
    uMainWindow->statusBar()->showMessage(QString("%1 reminders read.").arg(mReminders.size()),
                                          5000);
}

void clControl::AlarmClockService_time_reached(const QDateTime &t, int rem_id)
{
    Q_ASSERT(mReminders.contains(rem_id));
    mReminders[rem_id]->On_time_reached(t);
}

void clControl::reminder_becomes_active(int id)
{
    uMainWindow->reminder_becomes_active(id);
}

void clControl::reminder_becomes_inactive(int id)
{
    uMainWindow->reminder_becomes_inactive(id);
}

void clControl::save_reminder_data(const int id)
{
    QString err_msg;
    bool ch = uFileReadWrite.save_reminder(*mReminders[id], err_msg);
    if(!ch)
        QMessageBox::warning(nullptr, "error",
                             QString("Failed to save reminder data.\n\n%1").arg(err_msg));
    uMainWindow->statusBar()->showMessage("File written.", 5000);
}

void clControl::MainWindow_to_modify_reminder(int id, int modifyFlags,
                                              const clReminder *new_data)
{
    Q_ASSERT(mReminders.contains(id));
    mReminders[id]->update_data(modifyFlags, new_data);
    save_reminder_data(id);
    uMainWindow->reminder_updated(id);
}

void clControl::MainWindow_to_modify_reminder_spec(int id, const clReminderSpec *new_spec)
{
    Q_ASSERT(mReminders.contains(id));

    // update situation/event list
    mSituations += new_spec->get_situations_involved();
    mEvents += new_spec->get_events_involved();

    uMainWindow->situation_event_list_updated();

    // get g-event history from file
    QDateTime t0 = QDateTime::currentDateTime();
    QMultiMap<QDateTime, clDataElem_GEvent> GEventHistory = uFileReadWrite.read_gevent_history();

    // update spec
    bool old_activeness = mReminders[id]->is_active_via_bindings();
    mReminders[id]->update_spec_setting(*new_spec, t0, GEventHistory, mActiveSituations);

    //
    save_reminder_data(id);
    uMainWindow->reminder_updated(id);

    // new activeness
    if(mReminders[id]->is_active_via_bindings())
    {
        if(!old_activeness)
            reminder_becomes_active(id);
    }
    else
    {
        if(old_activeness)
            reminder_becomes_inactive(id);
    }
}

void clControl::MainWindow_to_add_reminder_record(int id, const QDateTime &t,
                                                  const QString &log_text)
{
    Q_ASSERT(mReminders.contains(id));
    mReminders[id]->add_record(t, log_text);
    save_reminder_data(id);
    uMainWindow->reminder_updated(id);
}

void clControl::MainWindow_to_create_new_reminder(const QString &title)
{
    int id = mReminders.isEmpty() ? 0 : (mReminders.lastKey()+1);

    clReminder *rem = new clReminder(id);
    rem->set_title(title);

    // save reminder
    QString err_msg;
    bool ch = uFileReadWrite.save_reminder(*rem, err_msg);
    if(!ch)
    {
        QMessageBox::warning(nullptr, "warning",
                             QString("Could not save reminder data.\n\n%1").arg(err_msg));
        return;
    }
    uMainWindow->statusBar()->showMessage("File written.", 5000);

    //
    mReminders.insert(id, rem);
    mReminders_ReadOnly.insert(id, rem);

    // build connections
    connect(rem, SIGNAL(set_alarm(int,QDateTime)),
            uAlarmClockService, SLOT(schedule_alarm(int,QDateTime)));
    connect(rem, SIGNAL(become_active(int)),   this, SLOT(reminder_becomes_active(int)));
    connect(rem, SIGNAL(become_inactive(int)), this, SLOT(reminder_becomes_inactive(int)));

    // initialize
    QDateTime t0 = QDateTime::currentDateTime();
    QMultiMap<QDateTime, clDataElem_GEvent> GEventHistory
            = uFileReadWrite.read_gevent_history();

    rem->initialize(t0, GEventHistory, mActiveSituations);

    // inform MainWindow
    uMainWindow->reminder_created(id);
}

void clControl::MainWindow_to_get_day_planning_status(const QDate &date,
                                                   QMap<int,clDataElem_ScheduleStatus> *status)
{
    *status = uFileReadWrite.read_day_planning_status(date);
}

void clControl::MainWindow_day_planning_status_modified(const QDate &date,
                                              const QMap<int,clDataElem_ScheduleStatus> &status)
{
    uFileReadWrite.save_day_planning_status(date, status);
    uMainWindow->statusBar()->showMessage("File written.", 5000);
}

static bool induces(const QString &sit1, const QString &sit2)
//Returns whether `sit1` induces `sit2`
{
    if(sit2 == sit1)
        return true;
    else
        return sit1.startsWith(sit2+':');
}

bool clControl::is_induced_by_selected_situation(const QString &sit) const
//Returns whether `sit` is induced by any currently selected situation.
{
    foreach(QString selected_sit, mSelectedSituations)
    {
        if(induces(selected_sit, sit))
            return true;
    }
    return false;
}

QSet<QString> clControl::get_induced_names(const QString &name) const
//Example: If `name` is "A:B:C", the returned string list will be {"A", "A:B", "A:B:C"}.
{
    if(name.isEmpty())
        return QSet<QString>();

    QSet<QString> induced_names;
    QString induced;
    QStringList tokens = name.split(':');
    foreach(QString token, tokens)
    {
        if(! induced.isEmpty())
            induced += ':';
        induced += token;

        induced_names << induced;
    }

    return induced_names;
}

void clControl::update_active_situations()
//Set `mActiveSituations` according to `mSelectedSituations`.
{
    mActiveSituations.clear();
    foreach(QString selected_sit, mSelectedSituations)
        mActiveSituations += get_induced_names(selected_sit);
}

void clControl::MainWindow_gevent_happened(const clDataElem_GEvent &gevent, const QDateTime &at)
{
    // get current g-event history from file
    QMultiMap<QDateTime,clDataElem_GEvent> GEventHistory = uFileReadWrite.read_gevent_history();

    // get g-events to trigger
    QList<clDataElem_GEvent> gevents_to_trigger;

    QSet<QString> induced_names = get_induced_names(gevent.get_name());
    QSet<QString> old_active_sits = mActiveSituations;
    if(gevent.is_start_of_situation())
    {
        mSelectedSituations += gevent.get_name();
        mActiveSituations += induced_names;

        //
        QSet<QString> sits_to_activate = mActiveSituations - old_active_sits;
        foreach(QString sit, sits_to_activate)
            gevents_to_trigger << clDataElem_GEvent(clDataElem_GEvent::StartOfSituation, sit);
    }
    else if(gevent.is_end_of_situation())
    {
        mSelectedSituations -= gevent.get_name();
        update_active_situations();

        //
        QSet<QString> sits_to_end = old_active_sits - mActiveSituations;
        foreach(QString sit, sits_to_end)
            gevents_to_trigger << clDataElem_GEvent(clDataElem_GEvent::EndOfSituation, sit);
    }
    else //`gevent` is an event
    {
        foreach(QString event, induced_names)
            gevents_to_trigger << clDataElem_GEvent(clDataElem_GEvent::Event, event);
    }

    // add `gevents_to_trigger[]` to g-event history and save history to file
    foreach(clDataElem_GEvent gev, gevents_to_trigger)
        GEventHistory.insert(at, gev);
    uFileReadWrite.save_gevent_history(GEventHistory);
    uMainWindow->statusBar()->showMessage("File written.", 5000);

    // inform all reminders of `gevents_to_trigger[]`
    for(auto it=mReminders.begin(); it!=mReminders.end(); it++)
    {
        for(int i=0; i<gevents_to_trigger.size(); i++)
            it.value()->On_gevent(gevents_to_trigger.at(i));
    }
}
