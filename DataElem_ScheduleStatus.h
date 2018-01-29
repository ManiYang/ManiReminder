#ifndef DATA_ELEMENT_SCHEDULE_STATUS_H
#define DATA_ELEMENT_SCHEDULE_STATUS_H

#include "Util_HrMinRange.h"

/* Scheduling status for a reminder on a day. */

class clDataElem_ScheduleStatus
{
public:
    enum enStatus { Unscheduled, Scheduled, Postponed, Skipped };

    clDataElem_ScheduleStatus(); //set as unscheduled

    void clear() { mStatus = Unscheduled; mScheduledTime.clear(); }

    bool parse_and_set(const QString &S);

    void set_unscheduled() { clear(); }
    void set_scheduled(const QList<std::pair<clUtil_HrMinRange, bool> > &scheduled_times);
    void set_postponed(const QDate &postpone_to);
    void set_skipped();

    void remove_ith_schedule(const int i); //remove ith scheduled time range

    //
    enStatus get_status() const { return mStatus; }
    QList<std::pair<clUtil_HrMinRange,bool> > get_scheduled_times() const;
    QDate get_date_postponed_to() const;

    QString print() const;

    bool operator == (const clDataElem_ScheduleStatus &another) const;

private:
    enStatus mStatus;
    QList<std::pair<clUtil_HrMinRange,bool> > mScheduledTime;
                                              //meaningful only if `mStatus` = `Scheduled`
    QDate mPostponedTo; //meaningful only if `mStatus` = `Postponed`
};

#endif // DATA_ELEMENT_SCHEDULE_STATUS_H
