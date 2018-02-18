#ifndef UTIL_TASK_H
#define UTIL_TASK_H

#include <QDate>

/* A task is a reminder-deadline pair (the reminder must have date-setting). */

class clUtil_Task
{
public:
    clUtil_Task() : mRemID(-1) {}
    clUtil_Task(const int rem_id, const QDate &deadline);

    int mRemID;
    QDate mDeadline;
    void set(const int id, const QDate &deadline) { mRemID = id; mDeadline = deadline; }

    bool operator == (const clUtil_Task &another) const;
    bool operator < (const clUtil_Task &another) const;
};

#endif // UTIL_TASK_H
