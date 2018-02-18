#include "DataElem_TaskDayScheduleSession.h"
#include <utility_DOM.h>

clDataElem_TaskDayScheduleSession::clDataElem_TaskDayScheduleSession(
                                                 const clUtil_HrMinRange &hrmin_range,
                                                 QString prefix, QString state)
{
    Q_ASSERT(! hrmin_range.is_empty());
    mHrMinRange = hrmin_range;
    mPrefix = prefix;
    mState = state;
}

void clDataElem_TaskDayScheduleSession::set(const clUtil_HrMinRange &hrmin_range,
                                            QString prefix, QString state)
{
    Q_ASSERT(! hrmin_range.is_empty());
    mHrMinRange = hrmin_range;
    mPrefix = prefix;
    mState = state;
}

bool clDataElem_TaskDayScheduleSession::parse_and_set(const QString &S)
//format: "$prefix; $hr-min-range; $state"
{
    QStringList tokens = S.split(';');
    if(tokens.size() != 3)
        return false;

    //
    mPrefix = tokens.at(0).simplified();

    //
    bool ch = mHrMinRange.parse_and_set(tokens.at(1));
    if(!ch)
        return false;

    //
    mState = tokens.at(2).simplified();

    //
    return true;
}

QString clDataElem_TaskDayScheduleSession::print() const
//format: "$prefix; $hr-min-range; $state"
{
    Q_ASSERT(! mHrMinRange.is_empty());

    QString S;
    S += mPrefix;
    S += "; ";
    S += mHrMinRange.print(2);
    S += "; ";
    S += mState;

    return S;
}

////////////////////////////////////////////////////////////////////////////////////

void clDataElem_DayScheduleItem::set_task(const clUtil_Task &task,
                                          const QString &title, const QString &prefix,
                                          const clUtil_HrMinRange &session,
                                          const QString &state)
{
    Q_ASSERT(task.mRemID >= 0 && task.mDeadline.isValid());
    Q_ASSERT(! session.is_empty());

    mRemID = task.mRemID;
    mDeadline = task.mDeadline;
    mTitle = title;
    mPrefix = prefix;
    mTriggeringTime = QTime();
    mHrMinRange = session;
    mState = state;
}

void clDataElem_DayScheduleItem::set_nontask_w_time_trigger(
                                                   const int rem_id, const QString &title,
                                                   const QString &prefix,
                                                   const QTime &triggering_time,
                                                   const QString &state)
{
    Q_ASSERT(triggering_time.isValid());

    mRemID = rem_id;
    mDeadline = QDate();
    mTitle = title;
    mPrefix = prefix;
    mTriggeringTime = triggering_time;
    mHrMinRange.clear();
    mState = state;
}

void clDataElem_DayScheduleItem::set_nontask_w_time_range_binding(const int rem_id, const QString &title,
                                                         const QString &prefix,
                                                         const clUtil_HrMinRange &time_range,
                                                         const QString &state)
{
    Q_ASSERT(! time_range.is_empty());

    mRemID = rem_id;
    mDeadline = QDate();
    mTitle = title;
    mPrefix = prefix;
    mTriggeringTime = QTime();
    mHrMinRange = time_range;
    mState = state;
}

void clDataElem_DayScheduleItem::set_temporary_reminder(const QString &title, const QString &prefix,
                                               const clUtil_HrMinRange &time_range,
                                               const QString &state)
{
    Q_ASSERT(! time_range.is_empty());

    mRemID = -2;
    mDeadline = QDate();
    mTitle = title;
    mPrefix = prefix;
    mTriggeringTime = QTime();
    mHrMinRange = time_range;
    mState = state;
}

bool clDataElem_DayScheduleItem::is_task() const
{
    return mRemID >= 0 && mDeadline.isValid();
}

bool clDataElem_DayScheduleItem::is_nontask_w_time_trigger() const
{
    return mRemID >= 0 && !mDeadline.isValid() && mTriggeringTime.isValid();
}

bool clDataElem_DayScheduleItem::is_nontask_w_time_range_binding() const
{
    return mRemID >= 0 && !mDeadline.isValid() && !mTriggeringTime.isValid();
}

bool clDataElem_DayScheduleItem::is_temperary_reminder() const
{
    return mRemID == -2;
}

clUtil_Task clDataElem_DayScheduleItem::get_task() const
{
    Q_ASSERT(is_task());
    return clUtil_Task(mRemID, mDeadline);
}

QTime clDataElem_DayScheduleItem::get_triggering_time() const
{
    Q_ASSERT(mTriggeringTime.isValid());
    return mTriggeringTime;
}

clUtil_HrMinRange clDataElem_DayScheduleItem::get_time_range() const
{
    Q_ASSERT(! mHrMinRange.is_empty());
    return mHrMinRange;
}

void clDataElem_DayScheduleItem::set_title(const QString &title)
//for temporary reminder only
{
    Q_ASSERT(is_temperary_reminder());
    mTitle = title;
}

void clDataElem_DayScheduleItem::set_time_range(const clUtil_HrMinRange &hrmin_range)
{
    Q_ASSERT(! is_nontask_w_time_trigger());
    Q_ASSERT(! hrmin_range.is_empty());
    mHrMinRange = hrmin_range;
}

bool clDataElem_DayScheduleItem::time_range_includes(const QTime &time) const
//accurate to seconds only
{
    Q_ASSERT(! is_nontask_w_time_trigger());
    return mHrMinRange.includes(time.hour(), time.minute(), time.second());
}

QTime clDataElem_DayScheduleItem::get_start_or_triggering_time() const
{
    Q_ASSERT(! is_empty());
    if(mTriggeringTime.isValid())
        return mTriggeringTime;
    else
        return QTime(mHrMinRange.get_start_hr(), mHrMinRange.get_start_min());
}

void clDataElem_DayScheduleItem::add_to_XML(QDomDocument &doc, QDomElement &parent) const
//Normally only tasks and temperary sessions need to be saved.
//Add under `parent`:
//<schedule-item>
//  ...
//</schedule-item>
{
    Q_ASSERT(! is_empty());

    QDomElement elem_session = nsDomUtil::add_child_element(doc, parent, "schedule-item");

    //
    nsDomUtil::add_child_element_w_text(doc, elem_session, "id", QString::number(mRemID));

    if(mDeadline.isValid())
        nsDomUtil::add_child_element_w_text(doc, elem_session, "deadline",
                                            mDeadline.toString("yyyy/M/d"));

    nsDomUtil::add_child_element_w_text(doc, elem_session, "title", mTitle);

    nsDomUtil::add_child_element_w_text(doc, elem_session, "prefix", mPrefix);

    if(mTriggeringTime.isValid())
        nsDomUtil::add_child_element_w_text(doc, elem_session, "triggering-time",
                                            mTriggeringTime.toString("hh:mm"));
    else
        nsDomUtil::add_child_element_w_text(doc, elem_session, "hr-min-range",
                                            mHrMinRange.print(2));

    if(!mState.isEmpty())
        nsDomUtil::add_child_element_w_text(doc, elem_session, "state", mState);
}

bool clDataElem_DayScheduleItem::parse_and_set(QDomElement &parent)
//`parent` must be
//<schedule-item>
//  ...
//</schedule-item>
{
    QString S = nsDomUtil::get_text_of_first_child_element(parent, "id");
    bool ch;
    mRemID = S.toInt(&ch);
    if(!ch)
        return false;

    mDeadline = QDate();
    S = nsDomUtil::get_text_of_first_child_element(parent, "deadline");
    if(! S.isEmpty())
    {
        mDeadline = QDate::fromString(S, "yyyy/M/d");
        if(! mDeadline.isValid())
            return false;
    }

    mTitle = nsDomUtil::get_text_of_first_child_element(parent, "title");

    mPrefix = nsDomUtil::get_text_of_first_child_element(parent, "prefix");

    mTriggeringTime = QTime();
    S = nsDomUtil::get_text_of_first_child_element(parent, "triggering-time");
    if(! S.isEmpty())
    {
        mTriggeringTime = QTime::fromString(S, "hh:mm");
        if(! mTriggeringTime.isValid())
            return false;
    }

    mHrMinRange.clear();
    S = nsDomUtil::get_text_of_first_child_element(parent, "hr-min-range");
    if(! S.isEmpty())
    {
        ch = mHrMinRange.parse_and_set(S);
        if(!ch)
            return false;
    }

    mState = nsDomUtil::get_text_of_first_child_element(parent, "state");

    //
    return true;
}

QString clDataElem_DayScheduleItem::print_time() const
//$prefix $hr:$min[-$hr:$min]
{
    Q_ASSERT(! is_empty());

    QString S;
    if(!mPrefix.isEmpty())
        S += mPrefix + ' ';
    if(mTriggeringTime.isValid())
        S += mTriggeringTime.toString("hh:mm");
    else
        S += mHrMinRange.print(3);

    return S;
}
