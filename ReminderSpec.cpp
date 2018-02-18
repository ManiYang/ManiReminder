#include <QDebug>
#include "ReminderSpec.h"
#include "utility_DOM.h"
#include "utility_general.h"

clReminderSpec::clReminderSpec(QObject *parent)
    : QObject(parent),
      mPassingFilters(false), mActiveByBindings(false)
{
    // connects
    connect(&mTimeRangeBinding, SIGNAL(to_set_alarm(QDateTime)),
            this, SLOT(set_alarm(QDateTime)));

    connect(&mGEventDurBinding, SIGNAL(to_set_alarm(QDateTime)),
            this, SLOT(set_alarm(QDateTime)));

    connect(&mTimeTrigger, SIGNAL(to_set_alarm(QDateTime)),
            this, SLOT(set_alarm(QDateTime)));
    connect(&mTimeTrigger, SIGNAL(to_trigger()),
            this, SLOT(triggered()));

    connect(&mGEventTrigger, SIGNAL(to_set_alarm(QDateTime)),
            this, SLOT(set_alarm(QDateTime)));
    connect(&mGEventTrigger, SIGNAL(to_trigger()),
            this, SLOT(triggered()));

    connect(&mTimeRangeFilter, SIGNAL(to_set_alarm(QDateTime)),
            this, SLOT(set_alarm(QDateTime)));

    //......

}

bool clReminderSpec::parse_and_set(QDomElement &spec_setting, QString &parse_error_message)
{
    clear();

    /// -------- bindings --------
    bool have_bindings = false;

    // look for settings of situation bindings
    bool ch = mSituationBinding.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    if(! mSituationBinding.is_empty())
        have_bindings = true;

    // look for settings of time-range bindings
    ch = mTimeRangeBinding.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    if(! mTimeRangeBinding.is_empty())
        have_bindings = true;

    // look for settings of g-event-duration bindings
    ch = mGEventDurBinding.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    if(! mGEventDurBinding.is_empty())
        have_bindings = true;

    /// -------- triggers --------
    bool have_triggers = false;

    // look for settings of time triggers
    ch = mTimeTrigger.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    if(! mTimeTrigger.is_empty())
        have_triggers = true;

    // look for settings of g-event triggers
    ch = mGEventTrigger.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    if(! mGEventTrigger.is_empty())
        have_triggers = true;

    /// -------- date setting --------
    ch = mDateSetting.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    /// -------- filters --------
    // look for settings of time-range filters
    ch = mTimeRangeFilter.parse_and_set(spec_setting, parse_error_message);
    if(!ch)
        return false;

    //......

    ///
    if(have_bindings)
    {
        if(have_triggers)
        {
            parse_error_message = "A reminder cannot have both bindings and triggers.";
            return false;
        }
        if(! mDateSetting.is_empty())
        {
            parse_error_message = "A reminder cannot have both bindings and date-setting.";
            return false;
        }
    }
    if(have_triggers)
    {
        if(! mDateSetting.is_empty())
        {
            parse_error_message = "A reminder cannot have both triggers and date-setting.";
            return false;
        }
    }

    //if have date-settings: cannot have filters other than date-filter....

    //if have filters only...


    //
    return true;
}

void clReminderSpec::initialize(const QDateTime &t0,
                                const QMultiMap<QDateTime,clGEvent> &gevent_history,
                                const QSet<QString> &selected_and_induced_situations)
{
    mSituationBinding.initialize(selected_and_induced_situations);
    mTimeRangeBinding.initialize();
    mGEventDurBinding.initialize(t0, gevent_history);
    mTimeTrigger.initialize();
    mGEventTrigger.initialize(t0, gevent_history);
    mTimeRangeFilter.initialize();
    // ...

    //
    update_filter_passing();
    update_activeness_from_bindings();
}

void clReminderSpec::update_settings(const clReminderSpec &new_spec,
                                     const QDateTime &t0,
                                     const QMultiMap<QDateTime,clGEvent> &gevent_history,
                                     const QSet<QString> &selected_and_induced_situations)
{
    clear();

    //
    mSituationBinding.update_settings(new_spec.mSituationBinding,
                                      selected_and_induced_situations);
    mTimeRangeBinding.update_settings(new_spec.mTimeRangeBinding);
    mGEventDurBinding.update_settings(new_spec.mGEventDurBinding, t0, gevent_history);
    mTimeTrigger.update_settings(new_spec.mTimeTrigger);
    mGEventTrigger.update_settings(new_spec.mGEventTrigger, t0, gevent_history);
    mTimeRangeFilter.update_settings(new_spec.mTimeRangeFilter);

    mDateSetting = new_spec.mDateSetting;

    //
    update_filter_passing();
    update_activeness_from_bindings();
}

void clReminderSpec::clear()
{
    mSituationBinding.clear();
    mTimeRangeBinding.clear();
    mGEventDurBinding.clear();
    mTimeTrigger.clear();
    mGEventTrigger.clear();
    mTimeRangeFilter.clear();
    mDateSetting.clear();
    //....

    mActiveByBindings = false;
    mPassingFilters = true;
}

QString clReminderSpec::print() const
{
    QString S;
    if(! mSituationBinding.is_empty())
        S += mSituationBinding.print();

    if(! mTimeRangeBinding.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mTimeRangeBinding.print();
    }

    if(! mGEventDurBinding.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mGEventDurBinding.print();
    }

    if(! mTimeTrigger.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mTimeTrigger.print();
    }

    if(! mGEventTrigger.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mGEventTrigger.print();
    }

    if(! mTimeRangeFilter.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mTimeRangeFilter.print();
    }

    if(! mDateSetting.is_empty())
    {
        if(! S.isEmpty())
            S += '\n';
        S += mDateSetting.print();
    }
    // ....

    return S;
}

void clReminderSpec::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
#define ADD_TO_XML(obj) if(! obj.is_empty()) \
                            obj.add_to_XML(doc, parent);
    ADD_TO_XML(mSituationBinding);
    ADD_TO_XML(mTimeRangeBinding);
    ADD_TO_XML(mGEventDurBinding);
    ADD_TO_XML(mTimeTrigger);
    ADD_TO_XML(mGEventTrigger);
    ADD_TO_XML(mTimeRangeFilter);
    ADD_TO_XML(mDateSetting);
    // ....

#undef ADD_TO_XML
}

void clReminderSpec::operator = (const clReminderSpec &another)
//clear and copy settings
{
    clear();

#define COPY(obj) obj = another.obj;
    COPY(mSituationBinding);
    COPY(mTimeRangeBinding);
    COPY(mGEventDurBinding);
    COPY(mTimeTrigger);
    COPY(mGEventTrigger);
    COPY(mTimeRangeFilter);
    COPY(mDateSetting);
    // ...

#undef COPY
}

QSet<QString> clReminderSpec::get_situations_involved() const
{
    QSet<QString> sits;
    sits |= mSituationBinding.get_situations_involved();
    sits |= mGEventDurBinding.get_situations_involved();
    sits |= mGEventTrigger.get_situations_involved();
    //....

    return sits;
}

QSet<QString> clReminderSpec::get_events_involved() const
{
    QSet<QString> events;
    events |= mGEventDurBinding.get_events_involved();
    events |= mGEventTrigger.get_events_involved();
    // ...

    return events;
}

bool clReminderSpec::has_triggers() const
{
    return ! mTimeTrigger.is_empty() || ! mGEventTrigger.is_empty(); // ...
}

bool clReminderSpec::has_bindings() const
{
    return     ! mSituationBinding.is_empty()
            || ! mTimeRangeBinding.is_empty()
            || ! mGEventDurBinding.is_empty(); //...
}

void clReminderSpec::On_gevent(const clDataElem_GEvent &gevent)
{
    // inform all relevant units about the occurrence of 'gevent'
    mSituationBinding.On_gevent(gevent);
    mGEventDurBinding.On_gevent(gevent);
    mGEventTrigger.On_gevent(gevent);
    //.......

    //
    update_activeness_from_bindings();
    //.......
}

void clReminderSpec::On_time_reached(const QDateTime &t)
{
    // inform all relevant units that 't' is reached
    mTimeRangeBinding.On_time_reached(t);
    mGEventDurBinding.On_time_reached(t);
    mTimeTrigger.On_time_reached(t);
    mGEventTrigger.On_time_reached(t);
    mTimeRangeFilter.On_time_reached(t);
    //.......

    //
    update_filter_passing();
    update_activeness_from_bindings();
}

////////

void clReminderSpec::set_alarm(const QDateTime &t)
{
    emit to_clReminder__set_alarm(t);
}

void clReminderSpec::triggered()
{
    if(mPassingFilters)
        emit to_clReminder__become_active();
}

void clReminderSpec::update_activeness_from_bindings()
//also emit signals become_active() or become_inactive() if necessary
{
    bool new_activeness =    mSituationBinding.get_activeness()
                          || mTimeRangeBinding.get_activeness()
                          || mGEventDurBinding.get_activeness();

    //
    if(mActiveByBindings && !new_activeness)
    {
        mActiveByBindings = false;
        emit to_clReminder__become_inactive();

    }
    else if(!mActiveByBindings && new_activeness)
    {
        mActiveByBindings = true;
        if(mPassingFilters)
            emit to_clReminder__become_active();
    }
}

void clReminderSpec::update_filter_passing()
{
    bool new_passing = mTimeRangeFilter.passing(); // && ...

    //
    if(mPassingFilters && !new_passing)
    {
        mPassingFilters = false;
        emit to_clReminder__become_inactive();
    }
    else if(!mPassingFilters && new_passing)
    {
        mPassingFilters = true;
        if(mActiveByBindings)
            emit to_clReminder__become_active();
    }
}

QMultiMap<QDateTime,QString> clReminderSpec::get_scheduled_actions() const
{
    QMultiMap<QDateTime,QString> actions, actions1;

#define ADD_ACTIONS(obj, obj_str) actions1 = obj.get_scheduled_actions(); \
                                  for(auto it=actions1.begin(); it!=actions1.end(); it++) \
                                      it.value() = obj_str ": "+it.value(); \
                                  actions += actions1;

    ADD_ACTIONS(mTimeRangeBinding, "time-range-binding");
    ADD_ACTIONS(mGEventDurBinding, "gevent-duration-binding");
    ADD_ACTIONS(mTimeTrigger, "time-trigger");
    ADD_ACTIONS(mGEventTrigger, "event-trigger");
    ADD_ACTIONS(mTimeRangeFilter, "time-range-filter");

    return actions;

#undef ADD_ACTIONS
}

bool clReminderSpec::is_due_date(const QDate &date) const
{
    return mDateSetting.is_due_date(date);
}

bool clReminderSpec::date_setting_includes(const QDate &date, int *Ndays_to_due) const
{
    return mDateSetting.includes(date, Ndays_to_due);
}

QList<QDate> clReminderSpec::get_due_dates_within(const QDate &d0, const QDate &d1) const
{
    Q_ASSERT(d0.isValid() && d1.isValid());
    Q_ASSERT(d1 >= d0);
    return mDateSetting.get_due_dates_within(d0, d1);
}

int clReminderSpec::get_precaution_day_counts() const
{
    Q_ASSERT(! mDateSetting.is_empty());
    return mDateSetting.get_precaution_day_counts();
}

QList<QTime> clReminderSpec::get_triggering_times_on_date(const QDate &date) const
//returned list will be in ascending order
{
    return mTimeTrigger.get_times_on_date(date);
}

QList<clUtil_HrMinRange> clReminderSpec::get_binding_hrmin_ranges_on_date(
                                                                 const QDate &base_date) const
//Get the time binding ranges starting on date `base_date`.
//Returned list will be separated and in ascending order.
{
    return mTimeRangeBinding.get_time_ranges_on_date(base_date);
}
