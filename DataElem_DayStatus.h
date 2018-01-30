#ifndef DATA_ELEMENT_SCHEDULE_STATUS_H
#define DATA_ELEMENT_SCHEDULE_STATUS_H

#include <QDomDocument>
#include <QDomElement>
#include "Util_HrMinRange.h"

class clRemDayScheduleSession;

//////

/* Day status for a reminder (with date-setting) on a day. */

class clDataElem_RemDayStatus
{
public:
    enum enState { Todo, Done, Postponed, Skipped };

    explicit clDataElem_RemDayStatus(const QDate source_date = QDate()); //set state as Todo

    bool parse_and_set(QDomElement &parent, QString &error_message);

    //
    void set_state(const enState new_state, const QDate &postpone_to = QDate());
    enState get_state() const { return mState; }
    QDate get_date_postponed_to() const { return mPostponedTo; }

    //
    void set_scheduled_sessions(const QList<clRemDayScheduleSession> &scheduled_sessions);
    QList<clRemDayScheduleSession> get_scheduled_sessions() const;

    //
    QDate get_source_date() const { return mSourceDate; }

    //
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

private:
    QDate mSourceDate;
    enState mState;
    QDate mPostponedTo; //valid only when `mStatus` = `Postponed`
    QList<clRemDayScheduleSession> mScheduledSessions;
};

/////////////////////////////////////////////////////////////////////////////////

class clRemDayScheduleSession
{
public:
    clRemDayScheduleSession() : mDone(false) {}
    explicit clRemDayScheduleSession(const clUtil_HrMinRange &hrmin_range,
                                     QString prefix = QString());

    bool parse_and_set(const QString &S); //format: "$prefix; $hr-min-range; {done, undone}"

    void set(const clUtil_HrMinRange &hrmin_range, QString prefix = QString()); //set as undone
    void set_done() { mDone = true; }
    void set_undone() { mDone = false; }

    //
    clUtil_HrMinRange get_hrmin_range() const { return mHrMinRange; }
    QString get_prefix() const { return mPrefix; }
    bool is_done() const { return mDone; }

    QString print() const; //format: "$prefix; $hr-min-range; {done, undone}"

private:
    clUtil_HrMinRange mHrMinRange;
    QString mPrefix;
    bool mDone;
};

#endif // DATA_ELEMENT_SCHEDULE_STATUS_H
