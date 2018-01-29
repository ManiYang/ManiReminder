#include "DataElem_ScheduleStatus.h"

clDataElem_ScheduleStatus::clDataElem_ScheduleStatus()
    : mStatus(Unscheduled)
{
}

bool clDataElem_ScheduleStatus::parse_and_set(const QString &S_)
{
    clear();
    QString S = S_.simplified();

    if(S == "unscheduled")
    {
        mStatus = Unscheduled;
    }
    else if(S.startsWith("scheduled "))
    {
        mStatus = Scheduled;

        S = S.mid(10);
        if(S.isEmpty())
            return false;
        QStringList tokens = S.split(',');
        foreach(QString str, tokens)
        {
            bool b_try = str.startsWith("(try)");
            if(b_try)
                str = str.mid(5);

            clUtil_HrMinRange hrmin;
            bool ch = hrmin.parse_and_set(str);
            if(!ch)
                return false;
            mScheduledTime << std::make_pair(hrmin, b_try);
        }
    }
    else if(S.startsWith("postponed "))
    {
        mStatus = Postponed;

        S = S.mid(10);
        QDate d = QDate::fromString(S, "yyyy/M/d");
        if(! d.isValid())
            return false;
        mPostponedTo = d;
    }
    else if(S == "skipped")
    {
        mStatus = Skipped;
    }
    else
        return false;

    //
    return true;
}

QString clDataElem_ScheduleStatus::print() const
{
    QString S;

    switch(mStatus)
    {
    case Unscheduled:
        S = "unscheduled";
        break;

    case Scheduled:
    {
        S = "scheduled ";
        QStringList ranges;
        for(int i=0; i<mScheduledTime.size(); i++)
        {
            std::pair<clUtil_HrMinRange,bool> time = mScheduledTime.at(i);

            QString str;
            if(std::get<1>(time))
                str = "(try)";
            str += std::get<0>(time).print(2); //"hr0:min0-hr1:min1"
            ranges << str;
        }
        S += ranges.join(',');
        break;
    }

    case Postponed:
    {
        S = "postponed ";
        Q_ASSERT(mPostponedTo.isValid());
        S += mPostponedTo.toString("yyyy/M/d");
        break;
    }

    case Skipped:
        S ="skipped";
        break;
    }

    return S;
}

bool clDataElem_ScheduleStatus::operator==(const clDataElem_ScheduleStatus &another) const
{
    if(mStatus != another.mStatus)
        return false;

    //
    if(mStatus == Scheduled)
        return mScheduledTime == another.mScheduledTime;
    else if(mStatus == Postponed)
        return mPostponedTo == another.mPostponedTo;
    else
        return true;
}

QList<std::pair<clUtil_HrMinRange,bool> > clDataElem_ScheduleStatus::
                                                                    get_scheduled_times() const
{
    Q_ASSERT(mStatus == Scheduled || mStatus == Unscheduled);
    return mScheduledTime;
}

QDate clDataElem_ScheduleStatus::get_date_postponed_to() const
{
    Q_ASSERT(mStatus == Postponed);
    return mPostponedTo;
}

void clDataElem_ScheduleStatus::set_scheduled
                             (const QList<std::pair<clUtil_HrMinRange,bool> > &scheduled_times)
{
    Q_ASSERT(! scheduled_times.isEmpty());

    mStatus = Scheduled;
    mScheduledTime = scheduled_times;
}

void clDataElem_ScheduleStatus::set_postponed(const QDate &postpone_to)
{
    Q_ASSERT(postpone_to.isValid());

    mStatus = Postponed;
    mPostponedTo = postpone_to;
    mScheduledTime.clear();
}

void clDataElem_ScheduleStatus::set_skipped()
{
    mStatus = Skipped;
    mScheduledTime.clear();
}

void clDataElem_ScheduleStatus::remove_ith_schedule(const int i)
//remove ith scheduled time range
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(mStatus == Scheduled);
    Q_ASSERT(i < mScheduledTime.size());
    mScheduledTime.removeAt(i);

    if(mScheduledTime.isEmpty())
        mStatus = Unscheduled;
}
