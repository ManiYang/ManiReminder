#include <QDebug>
#include <QMessageBox>
#include "Reminder.h"
#include "utility_DOM.h"
#include "utility_general.h"

clReminder::clReminder(int id, QObject *parent)
    : QObject(parent), mID(id)
{
    connect(&mSpec,   SIGNAL(to_clReminder__set_alarm(QDateTime)),
            this, SLOT(from_clReminderSpec__set_alarm(QDateTime)));
    connect(&mSpec,   SIGNAL(to_clReminder__become_active()),
            this, SLOT(from_clReminderSpec__become_active()));
    connect(&mSpec,   SIGNAL(to_clReminder__become_inactive()),
            this, SLOT(from_clReminderSpec__become_inactive()));
}

clReminder::~clReminder()
{
    clear();
}

void clReminder::clear()
{
    mSpec.clear();
    mTitle.clear();
    mTags.clear();
    mDetail.clear();

    remove_all_button_sets();

    mAlarm.clear();

    mRecords.clear();
    mQuickNote.clear();
}

bool clReminder::parse_and_set(QDomElement &reminder_setting)
//<spec> ... </spec>
//<title> text </title>
//<tags> tag1, tag2, ... </tags>
//<detail> detail </detail>
//<button-set> ... </button-set> ...
//<alarm> ... </alarm>
//<record> ... </record> ...
//<quick-note> ... </quick-note>
{
    clear();

    /// spec
    QDomElement elem_spec = reminder_setting.firstChildElement("spec");
    if(elem_spec.isNull())
    {
        parse_error_message = "Tag <spec> not found.";
        return false;
    }

    bool ch = mSpec.parse_and_set(elem_spec, parse_error_message);
    if(!ch)
        return false;

    /// title
    mTitle = nsDomUtil::get_text_of_first_child_element(reminder_setting, "title");
    if(mTitle.isNull())
    {
        parse_error_message = "Tag <title> not found.";
        return false;
    }

    /// tags
    QString str_tags
        = nsDomUtil::get_text_of_first_child_element(reminder_setting, "tags").simplified();
    if(! str_tags.isEmpty())
    {
        mTags = str_tags.split(',', QString::SkipEmptyParts);
        for(auto it=mTags.begin(); it!=mTags.end(); it++)
            *it = (*it).simplified();
    }

    /// detail
    mDetail = nsDomUtil::get_text_of_first_child_element(reminder_setting, "detail");
              //('mDetail' is null if <detail> not found)

    /// button sets
    for(int i=0; true; i++)
    {
        QDomElement e = nsDomUtil::get_ith_child_element(reminder_setting, i, "button-set");
        if(e.isNull())
            break;

        QString error_msg2;
        clDataElem_ButtonSet_Abstract *BS = parse_as_button_set(e, error_msg2);
        if(BS == 0)
        {
            parse_error_message = QString("Failed to parse the contents "
                                          "under %1 <button-set>.\n\n%2")
                                  .arg(print_ith(i+1)).arg(error_msg2);
            return false;
        }
        mButtonSets << BS;
    }

    /// alarm
    QString alarm_spec = nsDomUtil::get_text_of_first_child_element(reminder_setting, "alarm");
    if(! alarm_spec.isEmpty())
    {
        if(nsDomUtil::get_count_of_child_elements(reminder_setting, "alarm") > 1)
        {
            parse_error_message = "A reminder can only have one alarm setting.";
            return false;
        }
        QString err_msg;
        bool ch = mAlarm.parse_and_set(alarm_spec, err_msg);
        if(!ch)
        {
            parse_error_message = QString("Failed to parse the alarm setting \"%1\"."
                                          "\n\n%2").arg(alarm_spec).arg(err_msg);
            return false;
        }
    }

    /// records
    QStringList records = nsDomUtil::get_texts_of_child_elements(reminder_setting, "record");
    foreach(QString S, records)
    {
        QStringList tokens = S.split("; ");
        if(tokens.size() != 2)
        {
            parse_error_message = QString("Failed to parse the record setting \"%1\".").arg(S);
            return false;
        }

        QDateTime t = QDateTime::fromString(tokens.at(0), "yyyy/M/d.hh:mm:ss");
        if(!t.isValid())
        {
            parse_error_message
                = QString("Failed to parse the date-time in record setting \"%1\".").arg(S);
            return false;
        }

        QString label = tokens.at(1);

        mRecords.insert(t, label);
    }

    /// quick note
    mQuickNote = nsDomUtil::get_text_of_first_child_element(reminder_setting, "quick-note");

    //cannot have alarm if has date-setting...

    ///.......


    ///
    if(mSpec.has_triggers())
    {
        if(!has_ending_button())
        {
            parse_error_message = "A reminder with triggers should have an ending button.";
            return false;
        }
    }

    ///
    return true;
}

void clReminder::initialize(const QDateTime &t0,
                            const QMultiMap<QDateTime, clGEvent> &gevent_history,
                            const QSet<QString> &selected_and_induced_situations)
{
    mSpec.initialize(t0, gevent_history, selected_and_induced_situations);
}

void clReminder::update_spec_setting(const clReminderSpec &new_spec, const QDateTime &t0,
                                     const QMultiMap<QDateTime, clGEvent> &gevent_history,
                                     const QSet<QString> &selected_and_induced_situations)
//`gevent_history` must be the g-event record at `t0`.
{
    mSpec.update_settings(new_spec, t0, gevent_history, selected_and_induced_situations);
}

void clReminder::add_record(const QDateTime &t, const QString &log_text)
//add record to both `mRecords` and `mRecentRecords`
{
//    mRecentRecords.insert(t, log_text);

    //
    const int Nrecords_max = 300;
    mRecords.insert(t, log_text);
    while(mRecords.size() > Nrecords_max)
        mRecords.erase(mRecords.begin());
}

void clReminder::set_quick_note(const QString &quick_note)
{
    mQuickNote = quick_note;
}

void clReminder::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    // spec
    QDomElement elem_spec = nsDomUtil::add_child_element(doc, parent, "spec");
    mSpec.add_to_XML(doc, elem_spec);

    // title
    nsDomUtil::add_child_element_w_text(doc, parent, "title", mTitle);

    // tags
    if(! mTags.isEmpty())
    {
        QString str_tags;
        foreach(QString tag, mTags)
        {
            if(tag.contains(','))
            {
                QString msg = QString("[Warning] Tag \"%1\" contains ',' and will be ignored!")
                              .arg(tag);
                qDebug("%s", msg.toLatin1().data());
                continue;
            }
            if(! str_tags.isEmpty())
                str_tags += ", ";
            str_tags += tag;
        }
        nsDomUtil::add_child_element_w_text(doc, parent, "tags", str_tags);
    }

    // detail
    if(! mDetail.isEmpty())
        nsDomUtil::add_child_element_w_text(doc, parent, "detail", mDetail, true);

    // buttons
    for(int i=0; i<mButtonSets.size(); i++)
        add_button_set_to_XML(doc, parent, mButtonSets[i]);

    // alarm
    if(! mAlarm.is_empty())
        nsDomUtil::add_child_element_w_text(doc, parent, "alarm", mAlarm.print());

    // records
    if(! mRecords.isEmpty())
    {
        for(auto it=mRecords.constBegin(); it!=mRecords.constEnd(); it++)
        {
            QString S = it.key().toString("yyyy/M/d.hh:mm:ss");
            S += "; ";
            S += it.value();

            nsDomUtil::add_child_element_w_text(doc, parent, "record", S);
        }
    }

    // quick note
    if(! mQuickNote.isEmpty() )
        nsDomUtil::add_child_element_w_text(doc, parent, "quick-note", mQuickNote, true);
}

void clReminder::from_clReminderSpec__set_alarm(QDateTime t)
{
    emit set_alarm(mID, t);
}

void clReminder::from_clReminderSpec__become_active()
{
    emit become_active(mID);
}

void clReminder::from_clReminderSpec__become_inactive()
{
    emit become_inactive(mID);
}

void clReminder::On_gevent(const clDataElem_GEvent &gevent)
{
    mSpec.On_gevent(gevent);
}

void clReminder::On_time_reached(const QDateTime &t)
{
    mSpec.On_time_reached(t);
}

bool clReminder::has_ending_button() const
{
    for(auto it=mButtonSets.begin(); it!=mButtonSets.end(); it++)
    {
        if((*it)->has_ending_button())
            return true;
    }
    return false;
}

QMultiMap<QDateTime,QString> clReminder::get_scheduled_actions() const
{
    return mSpec.get_scheduled_actions();
}

bool clReminder::is_due_date(const QDate &date) const
{
    return mSpec.is_due_date(date);
}

bool clReminder::date_setting_includes(const QDate &date, int *Ndays_to_due) const
{
    return mSpec.date_setting_includes(date, Ndays_to_due);
}

QList<QDate> clReminder::get_due_dates_within(const QDate &d0, const QDate &d1) const
{
    Q_ASSERT(d0.isValid() && d1.isValid());
    Q_ASSERT(d1 >= d0);
    return mSpec.get_due_dates_within(d0, d1);
}

int clReminder::get_precaution_day_counts() const
//*this must have nonempty date-setting
{
    Q_ASSERT(mSpec.has_date_setting());
    return mSpec.get_precaution_day_counts();
}

void clReminder::set_title(const QString &title)
{
    Q_ASSERT(! title.isEmpty());
    mTitle = title;
}

void clReminder::set_tags(const QStringList &tags)
{
    Q_ASSERT(! tags.join(' ').contains(','));
    mTags = tags;
}

void clReminder::set_detail(const QString &detail)
{
    mDetail = detail;
}

QString clReminder::get_button_sets_print() const
{
    QString S;
    for(int i=0; i<mButtonSets.size(); i++)
    {
        if(! S.isEmpty())
            S += '\n';
        S += mButtonSets.at(i)->print();
    }
    return S;
}

void clReminder::remove_all_button_sets()
{
    for(auto it=mButtonSets.begin(); it!=mButtonSets.end(); it++)
        delete *it;
    mButtonSets.clear();
}

void clReminder::add_button_set(const clDataElem_ButtonSet_Abstract *button_set)
//`*button_set` will be copied
{
    mButtonSets << button_set->clone();
}

void clReminder::update_data(int modifyFlags, const clReminder *new_data)
//Modify the data fields specified by `modifyFlags` (excluding the spec) to those of
//`new_data`. To update the spec, use `update_spec_setting(...)`.
{

    if(modifyFlags & Title)
        mTitle = new_data->mTitle;

    if(modifyFlags & Tags)
        mTags = new_data->mTags;

    if(modifyFlags & Detail)
        mDetail = new_data->mDetail;

    if(modifyFlags & Alarm)
    {
        //...
    }

    if(modifyFlags & Buttons)
    {
        remove_all_button_sets();
        for(int i=0; i<new_data->mButtonSets.size(); i++)
            mButtonSets << new_data->mButtonSets[i]->clone();
    }

    if(modifyFlags & Records)
        mRecords = new_data->mRecords;

    if(modifyFlags & QuickNote)
        mQuickNote = new_data->mQuickNote;
}

QList<QTime> clReminder::get_triggering_times_on_date(const QDate &date) const
//returned list will be in ascending order
{
    return mSpec.get_triggering_times_on_date(date);
}

QList<clUtil_HrMinRange> clReminder::get_binding_hrmin_ranges_on_date(
                                                                const QDate &base_date) const
//Get the time binding ranges starting on date `base_date`.
//Returned list will be separated and in ascending order.
{
    return mSpec.get_binding_hrmin_ranges_on_date(base_date);
}
