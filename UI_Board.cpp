#include <QDebug>
#include <QTableWidget>
#include <QPushButton>
#include <QTextBlock>
#include <QDateTime>
#include "UI_Board.h"
#include "ui_UI_Board.h"
#include "utility_time.h"
#include "utility_widgets.h"
typedef clDataElem_ButtonSet_Abstract clButtonSet;

clUI_Board::clUI_Board(const QMap<int, const clReminder *> *p_reminders,
                       const QSet<QString> *p_situations, const QSet<QString> *p_events,
                       QWidget *parent)
    : QWidget(parent),
      ui(new Ui::clUI_Board), pReminders(p_reminders), pSituations(p_situations),
      pEvents(p_events)
{
    ui->setupUi(this);

    //
    ui->textBrowser_detail->setPlaceholderText("Select a reminder to view its details");

    ui->tableWidget_history->setColumnCount(2);
    ui->tableWidget_history->horizontalHeader()->setVisible(false);
    ui->tableWidget_history->verticalHeader()->setVisible(false);
    ui->tableWidget_history->horizontalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_history->verticalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);

    // `mContexMenu_for_SelectedSituations`
    mContextMenu_for_SelectedSituations = new QMenu(this);
    mAction_EndSituation = mContextMenu_for_SelectedSituations->addAction("End Situation");

    // `mTimer`
    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(update_history_time_display()));
    update_history_time_display();

    // reminder (title) list, `mRemList`
    mRemList = new clUI_Board_RemList;
    connect(mRemList, SIGNAL(current_reminder_changed(int,clUI_Board_RemList::enAlarmState)),
            this, SLOT(On_RemList_current_reminder_changed(int,clUI_Board_RemList::enAlarmState)));
    connect(mRemList, SIGNAL(current_reminder_alarm_state_changed(int,clUI_Board_RemList::enAlarmState)),
            this, SLOT(On_RemList_current_reminder_alarm_state_changed(int,clUI_Board_RemList::enAlarmState)));

    ui->splitter->insertWidget(1, mRemList);

    //
    mAction_GoToAllReminders = new QAction("Show in \"All Reminders\"", this);
    mRemList->setContextMenuPolicy(Qt::ActionsContextMenu);
    mRemList->addAction(mAction_GoToAllReminders);
    connect(mAction_GoToAllReminders, SIGNAL(triggered(bool)),
            this, SLOT(go_to_tab_all_reminders()));

    // `mReminderButtons`
    mButtonSetsView = new clUI_Board_ButtonSetsView(ui->horizontalLayout_RemButtons);
    connect(mButtonSetsView, SIGNAL(user_clicked_ending_button()),
            this, SLOT(On_user_close_reminder()));
    connect(mButtonSetsView, SIGNAL(to_add_record(QString)),
            this, SLOT(to_add_record(QString)));
    connect(mButtonSetsView, SIGNAL(user_clicked_a_button()),
            this, SLOT(On_user_clicked_a_rem_button()));

    // "alarm" button
    ui->pushButton_alarm->setVisible(false);

    mMenu_for_alarm_button = new QMenu(this);
    ui->pushButton_alarm->setMenu(mMenu_for_alarm_button);

    mAction_DelayAlarm = mMenu_for_alarm_button->addAction("snooze for ???");
    connect(mAction_DelayAlarm, SIGNAL(triggered(bool)),
            this, SLOT(On_user_delay_alarm()));

    //
    ui->tableWidget_recent_records->horizontalHeader()->setVisible(false);
    ui->tableWidget_recent_records->verticalHeader()->setVisible(false);
    ui->tableWidget_recent_records->horizontalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_recent_records->verticalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget_recent_records->setStyleSheet("font: 10pt");

    //`ui->plainTextEdit_quick_note`
    ui->plainTextEdit_quick_note->setStyleSheet("font: 10pt");
    ui->plainTextEdit_quick_note->installEventFilter(this);
}

clUI_Board::~clUI_Board()
{
    delete ui;

    //
    for(auto it=mAlternateBSInfos.begin(); it!=mAlternateBSInfos.end(); it++)
        delete it.value();
}

void clUI_Board::show_reminder(const int id)
{
    Q_ASSERT(pReminders->contains(id));
    const clReminder *reminder = (*pReminders)[id];

    //
    if(! mRemList->contains_reminder_title(id)) //not already active
    {
        // add to `mRemRecentRecords[]`
        Q_ASSERT(! mRemRecentRecords.contains(id));
        mRemRecentRecords.insert(id, QMultiMap<QDateTime,QString>());

        // add to `mAlternateBSInfos[]`
        Q_ASSERT(! mAlternateBSInfos.contains(id));
        mAlternateBSInfos.insert(id, new clAlternateBSInfo(reminder));


        /*
        // add to `mAlternateBSs[]` and `mAlternateBSStates[]`
        Q_ASSERT(! mAlternateBSs.contains(id));
        QList<clDataElem_ButtonSet_Abstract *> BSs = reminder->get_button_sets();

        QMap<int,clDataElem_ButtonSet_Alternate> alternate_BSs;
        QMap<int,int*> BS_states;
        for(int i=0; i<BSs.size(); i++)
        {
            if(BSs.at(i)->get_type() == clDataElem_ButtonSet_Abstract::Alternate)
            {
                alternate_BSs.insert(i,
                                 *dynamic_cast<clDataElem_ButtonSet_Alternate*>(BSs.at(i)));

                int *state_reg = new int;
                *state_reg = 0; //0: the first button is ON
                BS_states.insert(i, state_reg);
            }
        }
        mAlternateBSStates.insert(id, BS_states);
        mAlternateBSs.insert(id, alternate_BSs);
        */
    }

    //
    mRemList->add_reminder_title(id, reminder->get_title(), reminder->get_alarm());

    // start alarm
    if(! reminder->get_alarm().is_empty())
        mRemList->start_alarm_on_reminder(id);
}

void clUI_Board::hide_reminder(const int id)
{
    Q_ASSERT(pReminders->contains(id));
    if(mRemList->contains_reminder_title(id))
    {
        mRemList->remove_reminder_title(id);

        //
        Q_ASSERT(mRemRecentRecords.contains(id));
        mRemRecentRecords.remove(id);

        //
        Q_ASSERT(mAlternateBSInfos.contains(id));
        delete mAlternateBSInfos.take(id);
        /*
        //
        Q_ASSERT(mAlternateBSs.contains(id));
        mAlternateBSs.remove(id);

        QMap<int,int*> BS_state = mAlternateBSStates.take(id);
        for(auto it=BS_state.begin(); it!=BS_state.end(); it++)
            delete it.value();
            */
    }
}

void clUI_Board::reminder_updated(int id)
{
    if(! mRemList->contains_reminder_title(id))
        return;

    //
    mRemList->update_reminder_title(id, (*pReminders)[id]->get_title());

    //
    Q_ASSERT(mAlternateBSInfos.contains(id));
    mAlternateBSInfos[id]->update((*pReminders)[id]);

    /*
    // update `mAlternateBSs[id]` and `mRemButtonState[id]`
    Q_ASSERT(mAlternateBSs.contains(id));
    QMap<int,clDataElem_ButtonSet_Alternate> old_alternate_BSs = mAlternateBSs[id];
    QMap<int,clDataElem_ButtonSet_Alternate> new_alternate_BSs;
    {
        QList<clDataElem_ButtonSet_Abstract *> BSs = (*pReminders)[id]->get_button_sets();
        for(int i=0; i<BSs.size(); i++)
        {
            if(BSs.at(i)->get_type() == clDataElem_ButtonSet_Abstract::Alternate)
                new_alternate_BSs.insert(i,
                                    *dynamic_cast<clDataElem_ButtonSet_Alternate*>(BSs.at(i)));
        }
    }
    //.....
    mAlternateBSs[id] = new_alternate_BSs;
*/


    // update detail view
    if(mRemList->get_current_reminder_id() == id)
        update_detail_view(id);
}

void clUI_Board::situation_event_list_updated()
{
    ui->listWidget_all_situations_et_events->clear();

    for(auto it=pSituations->constBegin(); it!=pSituations->constEnd(); it++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::DisplayRole, *it);
        item->setData(Qt::ToolTipRole, "situation");
        ui->listWidget_all_situations_et_events->addItem(item);
    }

    for(auto it=pEvents->constBegin(); it!=pEvents->constEnd(); it++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::DisplayRole, *it);
        item->setData(Qt::ToolTipRole, "event");
        ui->listWidget_all_situations_et_events->addItem(item);
    }
}

void clUI_Board::current_date_changed()
{
//.........
}

/*
const clReminder *clUI_Board::get_current_selected_reminder() const
{
    int id = mRemList->get_current_reminder_id();
    //int id = get_current_selected_reminder_id();
    if(id == -1)
        return nullptr;
    else
        return pReminders->value(id);
}
*/

void clUI_Board::On_RemList_current_reminder_changed(
                                  int new_rem_id, clUI_Board_RemList::enAlarmState alarm_state)
{
    Q_ASSERT(mRemList->contains_reminder_title(new_rem_id));

    Q_UNUSED(alarm_state); //......[temp]
    update_detail_view(new_rem_id);
}

void clUI_Board::update_detail_view(const int rem_id)
//Fill the detail view with the data of `rem_id`.
//If `rem_id` < 0, clear the detail view.
{
    if(rem_id < 0)
    {
        ui->textBrowser_detail->setText("");
        ui->pushButton_alarm->setVisible(false);
        mButtonSetsView->clear();
        ui->plainTextEdit_quick_note->setPlainText("");
        ui->plainTextEdit_quick_note->setReadOnly(true);
        ui->tableWidget_recent_records->clear();
        //...

        return;
    }

    //
    Q_ASSERT(mRemList->contains_reminder_title(rem_id)); //reminder must be active

    // get reminder data --> `reminder`
    Q_ASSERT(pReminders->contains(rem_id));
    const clReminder *reminder = pReminders->value(rem_id);

    // detail
    QString str_detail = reminder->get_detail();
    if(str_detail.isEmpty())
        str_detail = " ";
    ui->textBrowser_detail->setText(str_detail);

    // "alarm" button
    //update_alarm_button(reminder, alarm_state); //
    update_alarm_button(reminder, clUI_Board_RemList::NoAlarm); //<------[temp]

    // button sets
    deploy_button_sets(rem_id);

    // recent records
    Q_ASSERT(mRemRecentRecords.contains(rem_id));
    update_recent_records_view(mRemRecentRecords[rem_id]);

    // quick note
    ui->plainTextEdit_quick_note->setPlainText(reminder->get_quick_note());
    ui->plainTextEdit_quick_note->setReadOnly(false);

    //...
}

void clUI_Board::On_RemList_current_reminder_alarm_state_changed(
                                  int rem_id, clUI_Board_RemList::enAlarmState new_alarm_state)
//not on changes of current reminder in the reminder list
{
    Q_ASSERT(pReminders->contains(rem_id));
    const clReminder *reminder = pReminders->value(rem_id);

    update_alarm_button(reminder, new_alarm_state);
}

void clUI_Board::go_to_tab_all_reminders()
{
    int id = mRemList->get_current_reminder_id();
    if(id < 0)
        return;
    emit to_show_reminder_in_tab_all_reminders(id);
}

void clUI_Board::update_alarm_button(const clReminder *reminder,
                         clUI_Board_RemList::enAlarmState alarm_state)
{
    if(alarm_state == clUI_Board_RemList::AlarmActive
       || alarm_state == clUI_Board_RemList::AlarmPaused)
    {
        ui->pushButton_alarm->setVisible(true);

        Q_ASSERT(! reminder->get_alarm().is_empty());
        int snooze_interval = reminder->get_alarm().get_pause_interval_sec();
        mAction_DelayAlarm->setText("snooze for "+print_xmxs(snooze_interval));

        mAction_DelayAlarm->setEnabled(alarm_state == clUI_Board_RemList::AlarmActive);
    }
    else
        ui->pushButton_alarm->setVisible(false);
}

void clUI_Board::deploy_button_sets(int rem_id)
//deploy the button sets of `reminder`
{
    mButtonSetsView->clear();

    const clReminder *reminder = (*pReminders)[rem_id];
    QList<clButtonSet *> button_sets = reminder->get_button_sets();

    Q_ASSERT(mAlternateBSInfos.contains(rem_id));
    for(int i=0; i<button_sets.size(); i++)
       mButtonSetsView->add_button(button_sets[i],
                                   mAlternateBSInfos[rem_id]->get_state_register(i));
}

void clUI_Board::On_user_delay_alarm()
{
    int id = mRemList->get_current_reminder_id();
    Q_ASSERT(pReminders->contains(id));

    mRemList->delay_alarm_on_reminder(id);
}

void clUI_Board::On_user_clicked_a_rem_button()
{
    int id = mRemList->get_current_reminder_id();
    Q_ASSERT(pReminders->contains(id));

    if(! pReminders->value(id)->get_alarm().is_empty())
        mRemList->stop_alarm_on_reminder(id);
}

void clUI_Board::On_user_close_reminder()
{
    int id = mRemList->get_current_reminder_id();
    hide_reminder(id);
}

void clUI_Board::to_add_record(QString log_text)
{
    if(log_text.isEmpty())
        return;

    //
    int id = mRemList->get_current_reminder_id();
    Q_ASSERT(id != -1);
    Q_ASSERT(pReminders->contains(id));
    Q_ASSERT(mRemRecentRecords.contains(id));

    QDateTime now = QDateTime::currentDateTime();

    add_to_recent_records_view(now, log_text);
    mRemRecentRecords[id].insert(now, log_text);
    emit to_add_reminder_record(id, now, log_text);
}

void clUI_Board::on_listWidget_all_situations_et_events_itemDoubleClicked(QListWidgetItem *item)
{
    Q_ASSERT(item != nullptr);

    const QString str_type = item->data(Qt::ToolTipRole).toString();
    const QString str_sit_or_event = item->data(Qt::DisplayRole).toString();
    Q_ASSERT(str_type == "situation" || str_type == "event");

    /// make g-event
    clDataElem_GEvent gevent;
    if(str_type == "situation")
    {
        if(ui->listWidget_selected_situations
             ->findItems(str_sit_or_event, Qt::MatchExactly).isEmpty()) //if not already selected
        {
            gevent.set(clDataElem_GEvent::StartOfSituation, str_sit_or_event);
        }
    }
    else //(str_type = "event")
        gevent.set(clDataElem_GEvent::Event, str_sit_or_event);

    if(gevent.is_empty())
        return;

    ///
    QDateTime now = QDateTime::currentDateTime();

    // add to "active situations" panel
    if(gevent.is_start_of_situation())
        ui->listWidget_selected_situations->addItem(str_sit_or_event);

    // add to "history" panel
    add_to_history(gevent, now);

    //
    emit gevent_happened(gevent, now);
}

void clUI_Board::add_to_history(const clDataElem_GEvent &gevent, const QDateTime &t)
{
    const int Nrecord_max  = 100;

    if(gevent.is_empty())
        return;

    // g-event string
    QString str_gevent;
    if(gevent.is_event())
        str_gevent = "event "+gevent.get_name();
    else if(gevent.is_start_of_situation())
        str_gevent = "sit. start "+gevent.get_name();
    else
        str_gevent = "sit. end "+gevent.get_name();

    // time string
    QString str_t = t.time().toString("hh:mm");

    //
    ui->tableWidget_history->insertRow(0);
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(str_gevent);
        ui->tableWidget_history->setItem(0, 0, item);
    }
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, str_t);
        item->setData(Qt::UserRole, t);
        ui->tableWidget_history->setItem(0, 1, item);
    }

    while(ui->tableWidget_history->rowCount() > Nrecord_max)
    {
        ui->tableWidget_history->removeRow(ui->tableWidget_history->rowCount()-1);
    }
}

void clUI_Board::update_history_time_display()
{
    QDateTime now = QDateTime::currentDateTime();

    QTableWidgetItem *item;
    for(int r=0; r<ui->tableWidget_history->rowCount(); r++)
    {
        item = ui->tableWidget_history->item(r, 1);
        Q_ASSERT(item != nullptr);
        QDateTime t = item->data(Qt::UserRole).toDateTime();
        QString str = print_time_relative_to_now(t, now);
        item->setData(Qt::DisplayRole, str);
    }

    // schedule next update
    int dt = now.time().secsTo(QTime(23, 59, 59));
    if(dt > 3600)
        dt = 3600;
    else
        dt += 2;

    mTimer.start(dt*1000);
}

void clUI_Board::on_listWidget_selected_situations_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidget_selected_situations->itemAt(pos);
    if(item == Q_NULLPTR)
        return;

    const QDateTime now = QDateTime::currentDateTime();
    const QString sit = item->data(Qt::DisplayRole).toString();

    //
    QPoint p = ui->listWidget_selected_situations->mapToGlobal(pos);
    QAction *action_selected = mContextMenu_for_SelectedSituations->exec(p);
    if(action_selected == mAction_EndSituation)
    {
        clDataElem_GEvent gevent(clDataElem_GEvent::EndOfSituation, sit);

        // remove from "active situations" panel
        delete item;

        // add to "history" panel
        add_to_history(gevent, now);

        //
        emit gevent_happened(gevent, now);
    }
}

void clUI_Board::on_pushButton_time_stamp_clicked()
{
    // set bold font
    QTextCursor cursor = ui->plainTextEdit_quick_note->textCursor();
    QTextCharFormat char_format = cursor.charFormat();
    char_format.setFontWeight(QFont::Bold);
    cursor.setCharFormat(char_format);
    ui->plainTextEdit_quick_note->setTextCursor(cursor);

    // insert current date-time
    QString str = QDateTime::currentDateTime().toString("<yyyy/M/d h:mm:ss>");
    ui->plainTextEdit_quick_note->insertPlainText(str);

    // set normal font weight
    char_format.setFontWeight(QFont::Normal);
    cursor.setCharFormat(char_format);
    ui->plainTextEdit_quick_note->setTextCursor(cursor);

    // set focus
    ui->plainTextEdit_quick_note->setFocus();
}

bool clUI_Board::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->plainTextEdit_quick_note)
    {
        if(event->type() == QEvent::FocusOut)
        {
            if(ui->plainTextEdit_quick_note->document()->isModified())
            {
                save_quick_note();
                ui->plainTextEdit_quick_note->document()->setModified(false);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void clUI_Board::save_quick_note()
{
    int id = mRemList->get_current_reminder_id();
    if(id < 0)
        return;

    Q_ASSERT(pReminders->contains(id));
    QString quick_note = ui->plainTextEdit_quick_note->toPlainText();
    clReminder new_data(-1);
    new_data.set_quick_note(quick_note);
    emit to_modify_reminder(id, clReminder::QuickNote, &new_data);
}

void clUI_Board::update_recent_records_view(const QMultiMap<QDateTime,QString> &recent_records)
{
    QStringList texts, times;
    QMapIterator<QDateTime,QString> it(recent_records);
    for(it.toBack(); it.hasPrevious(); )
    {
        it.previous();
        texts << it.value();
        times << it.key().toString("hh:mm:ss M/d");
    }

    fill_table_widget(*ui->tableWidget_recent_records,
                      QList<QStringList>() << texts << times,
                      QStringList() << "text" << "time");
}

void clUI_Board::add_to_recent_records_view(const QDateTime &t, const QString &log_text)
{
    insert_table_widget_row(*ui->tableWidget_recent_records, 0,
                            QStringList() << log_text << t.toString("hh:mm:ss M/d") );
}

////////////////////////////////////////////////////////////////////////////////////

clAlternateBSInfo::clAlternateBSInfo(const clReminder *reminder)
{
    set(reminder);
}

void clAlternateBSInfo::set(const clReminder *reminder)
//Set internal data according to `reminder`. All states will be set as 0.
{
    clear();

    //
    QList<clDataElem_ButtonSet_Abstract *> all_BSs = reminder->get_button_sets();
    for(int i=0; i<all_BSs.size(); i++)
    {
        if(all_BSs.at(i)->get_type() == clDataElem_ButtonSet_Abstract::Alternate)
        {
            mBSData << *dynamic_cast<clDataElem_ButtonSet_Alternate*>(all_BSs.at(i));
            mStateRegisters << new int(0);
            mBSNo_to_index.insert(i, mBSData.size()-1);
        }
    }
}

clAlternateBSInfo::~clAlternateBSInfo()
{
    clear();
}

void clAlternateBSInfo::clear()
{
    mBSNo_to_index.clear();

    mBSData.clear();

    for(auto it=mStateRegisters.begin(); it!=mStateRegisters.end(); it++)
        delete *it;
    mStateRegisters.clear();
}

int *clAlternateBSInfo::get_state_register(const int BSNo) const
//return `nullptr` if `BSNo` is not an alternate button set
{
    if(! mBSNo_to_index.contains(BSNo))
        return nullptr;

    int i = mBSNo_to_index[BSNo];
    Q_ASSERT(0 <= i && i < mStateRegisters.size());
    return mStateRegisters.at(i);
}

void clAlternateBSInfo::update(const clReminder *reminder)
{
    QList<clDataElem_ButtonSet_Alternate> old_altBSData = mBSData;
    QList<int> old_states;
    {
        for(auto it=mStateRegisters.begin(); it!=mStateRegisters.end(); it++)
            old_states << **it;
    }

    //
    set(reminder);

    // propagate old states to new registers
    for(int i=0; i<old_altBSData.size(); i++)
    {
        int j = mBSData.indexOf(old_altBSData.at(i));
        if(j != -1) //found j: mBSData[j] == old_altBSData[i]
            *mStateRegisters[j] = old_states[i];
    }
}
