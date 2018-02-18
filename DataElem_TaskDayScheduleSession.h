#ifndef DATAELEM_TASKDAYSCHEDULESESSION_H
#define DATAELEM_TASKDAYSCHEDULESESSION_H

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include "Util_HrMinRange.h"
#include "Util_Task.h"

/* A scheduled session for a task for one day. */

class clDataElem_TaskDayScheduleSession
{
public:
    clDataElem_TaskDayScheduleSession() {}
    explicit clDataElem_TaskDayScheduleSession(const clUtil_HrMinRange &hrmin_range,
                                               QString prefix = QString(),
                                               QString state = QString());

    bool parse_and_set(const QString &S); //format: "$prefix; $hr-min-range; $state"

    void set(const clUtil_HrMinRange &hrmin_range,
             QString prefix = QString(), QString state = QString());
    void set_state(const QString &state) { mState = state; }

    //
    clUtil_HrMinRange get_hrmin_range() const { return mHrMinRange; }
    QString get_prefix() const { return mPrefix; }
    QString get_state() const { return mState; }

    QString print() const; //format: "$prefix; $hr-min-range; $state"

private:
    QString mPrefix;
    clUtil_HrMinRange mHrMinRange;
    QString mState;
};

////////////////////////////////////////////////////////////////////////////////////

/* An item for the day-schedule table in day-plan tab. */

class clDataElem_DayScheduleItem
{
public:
    clDataElem_DayScheduleItem() : mRemID(-1) {}

    bool parse_and_set(QDomElement &parent);

    void set_task(const clUtil_Task &task, const QString &title,
                  const QString &prefix, const clUtil_HrMinRange &session,
                  const QString &state = QString());
    void set_nontask_w_time_trigger(const int rem_id, const QString &title,
                                    const QString &prefix,
                                    const QTime &triggering_time,
                                    const QString &state = QString());
    void set_nontask_w_time_range_binding(const int rem_id, const QString &title,
                                          const QString &prefix,
                                          const clUtil_HrMinRange &time_range,
                                          const QString &state = QString());
    void set_temporary_reminder(const QString &title, const QString &prefix,
                                const clUtil_HrMinRange &time_range,
                                const QString &state = QString());

    //
    void set_title(const QString &title); //for temporary reminder only
    void set_prefix(const QString &prefix) { mPrefix = prefix; }
    void set_state(const QString &state) { mState = state; }
    void set_time_range(const clUtil_HrMinRange &hrmin_range);

    //
    bool is_empty() const { return mRemID == -1; }

    bool is_task() const;
    bool is_nontask_w_time_trigger() const;
    bool is_nontask_w_time_range_binding() const;
    bool is_temperary_reminder() const;

    clUtil_Task get_task() const;
    int get_reminder_ID() const { return mRemID; }
    QString get_title() const { return mTitle; }
    QString get_prefix() const { return mPrefix; }
    QTime get_triggering_time() const;
    clUtil_HrMinRange get_time_range() const;
    QString get_state() const { return mState; }

    QString print_time() const; //$prefix $hr:$min[-$hr:$min]
    QTime get_start_or_triggering_time() const;
    bool time_range_includes(const QTime &time) const; //accurate to seconds only

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

private:
    int mRemID; //-2: a temperary session
    QDate mDeadline; //null: non-task reminder

    QString mTitle;

    QString mPrefix;
    QTime mTriggeringTime; //only for non-task reminder with time trigger
    clUtil_HrMinRange mHrMinRange; //for other than non-task reminder with time trigger

    QString mState;

      // A day-schedule item can be for
      //   1. a session of a task,
      //   2. a time instant of a reminder with time trigger,
      //   3. a time range of a reminder with time-range binding,
      //   4. a temperary session (with a time-range).
};



#endif // DATAELEM_TASKDAYSCHEDULESESSION_H
