#include "Util_Task.h"

clUtil_Task::clUtil_Task(const int rem_id, const QDate &deadline)
{
    mRemID = rem_id;
    mDeadline = deadline;
}

bool clUtil_Task::operator == (const clUtil_Task &another) const
{
    return mRemID == another.mRemID && mDeadline == another.mDeadline;
}

bool clUtil_Task::operator < (const clUtil_Task &another) const
{
    if(mRemID < another.mRemID)
        return true;
    else if(mRemID > another.mRemID)
        return false;
    else
        return mDeadline < another.mDeadline;
}
