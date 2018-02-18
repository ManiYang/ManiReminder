#ifndef CLREMINDER_H
#define CLREMINDER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include "ReminderSpec.h"
#include "DataElem_ButtonSet.h"
#include "DataElem_Alarm.h"

class clReminder : public QObject
{
    Q_OBJECT

public:
    clReminder(int id, QObject *parent = nullptr);
    clReminder(const clReminder &) = delete; //don't use copy constructor
    ~clReminder();

    void clear();

    // start-up
    //   1. call `parse_and_set(...)`
    //   2. make sure all connections are built properly
    //   3. get current time `t0` and get g-event history/record `hist`
    //   4. call `initialize(t0, hist, ...)`
    //   5. use `is_active()` to get whether the reminder is active via bindings and passes
    //      the filters
    bool parse_and_set(QDomElement &reminder_setting);
    QString parse_error_message;

    void initialize(const QDateTime &t0, const QMultiMap<QDateTime,clGEvent> &gevent_history,
                    const QSet<QString> &selected_and_induced_situations);

    // update spec
    //   1. make sure all connections are built properly
    //   2. get current time `t0` and get g-event history/record `hist`
    //   3. call `update_spec_setting(new_spec, t0, hist, ...)`
    //   4. use `is_active()` to get updated state
    void update_spec_setting(const clReminderSpec &new_spec,
                             const QDateTime &t0,
                             const QMultiMap<QDateTime,clGEvent> &gevent_history,
                             const QSet<QString> &selected_and_induced_situations);

    //
    bool is_active_via_bindings() const { return mSpec.is_active_via_bindings(); }
         //whether the reminder is active via bindings and passes the filters

    QList<QTime> get_triggering_times_on_date(const QDate &date) const;
                                              //returned list will be in ascending order
    QList<clUtil_HrMinRange> get_binding_hrmin_ranges_on_date(const QDate &base_date) const;
                             //Get the time binding ranges starting on date `base_date`.
                             //Returned list will be separated and in ascending order.

    bool has_date_setting() const { return mSpec.has_date_setting(); }
    bool is_due_date(const QDate &date) const;
    bool date_setting_includes(const QDate &date, int *Ndays_to_due) const;
    QList<QDate> get_due_dates_within(const QDate &d0, const QDate &d1) const;
    int get_precaution_day_counts() const; //*this must have nonempty date-setting

    //
    enum enModify {Title=1, Tags=2, Detail=4, Alarm=8,
                   Buttons=16, Records=32, QuickNote=64};
    void update_data(int modifyFlags, const clReminder *new_data);
         //Modify the data fields specified by `modifyFlags` (excluding the spec) to those of
         //`new_data`. To update the spec, use `update_spec_setting(...)`.

    //
    int get_id() const { return mID; }

    QString get_title() const { return mTitle; }
    void set_title(const QString &title);

    QStringList get_tags()  const { return mTags; }
    void set_tags(const QStringList &tags);

    QString get_detail() const { return mDetail; }
    void set_detail(const QString &detail);

    const clReminderSpec *get_spec() const { return &mSpec; }
    QString get_spec_print() const { return mSpec.print(); }
    QSet<QString> get_situations_involved() const { return mSpec.get_situations_involved(); }
    QSet<QString> get_events_involved() const { return mSpec.get_events_involved(); }

    QList<clDataElem_ButtonSet_Abstract *> get_button_sets() const { return mButtonSets; }
    QString get_button_sets_print() const;
    void remove_all_button_sets();
    void add_button_set(const clDataElem_ButtonSet_Abstract *button_set);
                        //`*button_set` will be copied

    clDataElem_ReminderAlarm get_alarm() const { return mAlarm; }

//    void clear_recent_records();
    void add_record(const QDateTime &t, const QString &log_text);
                   //add record to both `mRecords` and `mRecentRecords`
    QMultiMap<QDateTime, QString> get_records() const { return mRecords; }
//    QMultiMap<QDateTime, QString> get_recent_records() const { return mRecentRecords; }

    QString get_quick_note() const { return mQuickNote; }
    void set_quick_note(const QString &quick_note);

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

    // don't use:
    clReminder &operator = (const clReminder &) = delete;

    // [debug]
    QMultiMap<QDateTime,QString> get_scheduled_actions() const;

public slots:
    void On_gevent(const clDataElem_GEvent &gevent);
    void On_time_reached(const QDateTime &t);

signals:
    void set_alarm(int rem_id, QDateTime t);

    void become_active(int rem_id);
    void become_inactive(int rem_id);

//---------------

private slots:
    void from_clReminderSpec__set_alarm(QDateTime t);
    void from_clReminderSpec__become_active();
    void from_clReminderSpec__become_inactive();

private:
    int mID;
    clReminderSpec mSpec;
    QString mTitle;
    QStringList mTags; //each tag should not contain ','
    QString mDetail;
    clDataElem_ReminderAlarm mAlarm;
    QList<clDataElem_ButtonSet_Abstract *> mButtonSets;
    QMultiMap<QDateTime, QString> mRecords; //`mRecords[t]` is the log text
    QString mQuickNote;

    //
    bool has_ending_button() const;
};

#endif // CLREMINDER_H
