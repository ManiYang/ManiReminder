#ifndef DATAELEMENT_TASKSTATE_H
#define DATAELEMENT_TASKSTATE_H

#include <QDomDocument>
#include <QDomElement>
#include "Util_HrMinRange.h"

/* State for a task (i.e., a reminder-deadline pair). */

class clDataElem_TaskState
{
public:
    enum enState { Todo, Done, Skipped, DateShifted };
    clDataElem_TaskState(); //initialize state as `Todo`

    bool parse_and_set(const QString &S);
    void set_state(const enState new_state, const QDate &shifted_date = QDate());

    enState get_state() const { return mState; }
    bool is_todo() const { return mState == Todo; }
    bool is_done() const { return mState == Done; }
    bool is_skipped() const { return mState == Skipped; }
    bool is_date_shifted() const { return mState == DateShifted; }
    QDate get_shifted_date() const; //state must be `DateShifted`

    QString print() const;

    bool operator == (const clDataElem_TaskState &another) const;
    bool operator != (const clDataElem_TaskState &another) const;

private:
    enState mState;
    QDate mShiftedDate; //is valid only when `mStatus` = `DateShifted`
};

#endif // DATAELEMENT_TASKSTATE_H
