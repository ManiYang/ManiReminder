#ifndef CLREMINDERSPEC_H
#define CLREMINDERSPEC_H

#include <QObject>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include "DataElem_GEvent.h"
#include "RemSpec_SituationBinding.h"
#include "RemSpec_TimeRangeBinding.h"
#include "RemSpec_GEventDurationBinding.h"
#include "RemSpec_TimeTrigger.h"
#include "RemSpec_GEventTrigger.h"
#include "RemSpec_TimeRangeFilter.h"
#include "RemSpec_DateSetting.h"

class clReminderSpec : public QObject
{
    Q_OBJECT

public:
    explicit clReminderSpec(QObject *parent = Q_NULLPTR);

    void clear();

    // start-up
    //   1. call `parse_and_set(...)`
    //   2. make sure all connections are built properly
    //   3. get current time `t0` and get g-event history/record `hist`
    //   4. call `initialize(t0, hist, ...)`
    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_message);

    void initialize(const QDateTime &t0, const QMultiMap<QDateTime, clGEvent> &gevent_history,
                    const QSet<QString> &selected_and_induced_situations);

    // update settings
    //  1. make sure all connections are built properly
    //  2. get current time `t0` and get g-event history/record `hist`
    //  3. call `updatge_settings(another_spec, t0, hist, ...)`
    //  4. use `is_active_via_bindings()` to get updated state
    void update_settings(const clReminderSpec &new_spec, //copy only the settings
                         const QDateTime &t0,
                         const QMultiMap<QDateTime,clGEvent> &gevent_history,
                         const QSet<QString> &selected_and_induced_situations);

    //
    bool is_active_via_bindings() const { return mActiveByBindings && mPassingFilters; }
    bool date_setting_includes(const QDate &date, int *Ndays_to_due) const;

    //
    QSet<QString> get_situations_involved() const;
    QSet<QString> get_events_involved() const;

    bool has_bindings() const;
    bool has_triggers() const;
    bool has_date_setting() const { return ! mDateSetting.is_empty(); }

    QString print() const;
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

    void operator = (const clReminderSpec &another); //clear and copy settings only

    // [debug]
    QMultiMap<QDateTime,QString> get_scheduled_actions() const;

public slots:
    void On_gevent(const clDataElem_GEvent &gevent);
    void On_time_reached(const QDateTime &t);

signals:
    void to_clReminder__set_alarm(QDateTime t);
    void to_clReminder__become_active();
    void to_clReminder__become_inactive();

private slots:
    void set_alarm(const QDateTime &t);
    void triggered();

private:
    clRemSpec_SituationBinding mSituationBinding;
    clRemSpec_TimeRangeBinding mTimeRangeBinding;
    clRemSpec_GEventDurationBinding mGEventDurBinding;

    clRemSpec_TimeTrigger mTimeTrigger;
    clRemSpec_GEventTrigger mGEventTrigger;

    clRemSpec_TimeRangeFilter mTimeRangeFilter;

    clRemSpec_DateSetting mDateSetting;

      // All bindings/triggers are OR'ed together.
      // All filters are AND'ed together.

    //
    bool mPassingFilters;
    bool mActiveByBindings;

    void update_filter_passing();
    void update_activeness_from_bindings(); //also emit signals become_active() or
                                            //become_inactive() if necessary
};

#endif // CLREMINDERSPEC_H
