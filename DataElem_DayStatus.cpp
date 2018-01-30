#include "DataElem_DayStatus.h"
#include "utility_DOM.h"

clDataElem_RemDayStatus::clDataElem_RemDayStatus(const QDate source_date)
    : mSourceDate(source_date), mState(Todo)
//set state as Todo
{
}

void clDataElem_RemDayStatus::set_state(const enState new_state, const QDate &postpone_to)
{
    if(new_state == Postponed)
        Q_ASSERT(postpone_to.isValid());
    else
        Q_ASSERT(postpone_to.isNull());

    mState = new_state;
    if(mState == Postponed)
        mPostponedTo = postpone_to;
}

void clDataElem_RemDayStatus::set_scheduled_sessions(
                                const QList<clRemDayScheduleSession> &scheduled_sessions)
{
    mScheduledSessions = scheduled_sessions;
}

QList<clRemDayScheduleSession> clDataElem_RemDayStatus::get_scheduled_sessions() const
{
    return mScheduledSessions;
}

void clDataElem_RemDayStatus::add_to_XML(QDomDocument &doc, QDomElement &parent) const
//add under `parent`:
//  <day-status state="state">
//    [<source-date>...</source-date>]
//    [<postponed-to>...</postponed-to>]
//    [<scheduled-session>...</scheduled-session> ...]
//  </day-status>
{
    QDomElement elem_DS = doc.createElement("day-status");
    parent.appendChild(elem_DS);

    switch(mState)
    {
    case Todo:
        elem_DS.setAttribute("state", "todo");
        break;

    case Done:
        elem_DS.setAttribute("state", "done");
        break;

    case Postponed:
        elem_DS.setAttribute("state", "postponed");
        break;

    case Skipped:
        elem_DS.setAttribute("state", "skipped");
        break;
    }

    //
    if(mSourceDate.isValid())
        nsDomUtil::add_child_element_w_text(doc, elem_DS, "sousrce-date",
                                            mSourceDate.toString("yyyy/M/d"));

    //
    if(mState == Postponed)
        nsDomUtil::add_child_element_w_text(doc, elem_DS, "postponed-to",
                                            mPostponedTo.toString("yyyy/M/d"));

    //
    for(int i=0; i<mScheduledSessions.size(); i++)
        nsDomUtil::add_child_element_w_text(doc, elem_DS, "scheduled-session",
                                            mScheduledSessions.at(i).print());
}

bool clDataElem_RemDayStatus::parse_and_set(QDomElement &parent, QString &error_message)
//`parent` must be
//  <day-status state="state">
//    [<source-date>...</source-date>]
//    [<postponed-to>...</postponed-to>]
//    [<scheduled-session>...</scheduled-session> ...]
//  </day-status>
{
    Q_ASSERT(parent.tagName() == "day-status");

    mScheduledSessions.clear();

    //
    QString state_str = parent.attribute("state");

    if(state_str == "todo")
        mState = Todo;
    else if(state_str == "done")
        mState = Done;
    else if(state_str == "postponed")
        mState = Postponed;
    else if(state_str == "skipped")
        mState = Skipped;
    else
    {
        if(state_str.isEmpty())
            error_message = "Attribute \"state\" not found for element <day-status>";
        else
            error_message = QString("Unrecognized value \"%1\"of attribute \"state\""
                                    " of element <day-status>").arg(state_str);
        return false;
    }

    //
    mSourceDate = QDate();
    QString S = nsDomUtil::get_text_of_first_child_element(parent, "source-date");
    if(! S.isEmpty())
    {
        mSourceDate = QDate::fromString(S, "yyyy/M/d");
        if(! mSourceDate.isValid())
        {
            error_message = QString("Could not parse \"%1\" as date.").arg(S);
            return false;
        }
    }

    //
    if(mState == Postponed)
    {
        QString S = nsDomUtil::get_text_of_first_child_element(parent, "postponed-to");
        if(S.isEmpty())
        {
            error_message = "Tag <postponed-to> not found.";
            return false;
        }

        mPostponedTo = QDate::fromString(S, "yyyy/M/d");
        if(! mPostponedTo.isValid())
        {
            error_message = QString("Could not parse \"%1\" as date.").arg(S);
            return false;
        }
    }

    //
    QStringList str_sessions
            = nsDomUtil::get_texts_of_child_elements(parent, "scheduled-session");
    for(int i=0; i<str_sessions.size(); i++)
    {
        clRemDayScheduleSession session;
        bool ch = session.parse_and_set(str_sessions.at(i));
        if(!ch)
        {
            error_message
                    = QString("Could not parse the day-schedule-session setting \"%1\".")
                      .arg(str_sessions.at(i));
            return false;
        }
        mScheduledSessions << session;
    }

    //
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

clRemDayScheduleSession::clRemDayScheduleSession(const clUtil_HrMinRange &hrmin_range,
                                                 QString prefix)
{
    mHrMinRange = hrmin_range;
    mPrefix = prefix;
    mDone = false;
}

void clRemDayScheduleSession::set(const clUtil_HrMinRange &hrmin_range, QString prefix)
//set as undone
{
    mHrMinRange = hrmin_range;
    mPrefix = prefix;
    mDone = false;
}

bool clRemDayScheduleSession::parse_and_set(const QString &S)
//format: "$prefix; $hr-min-range; {done, undone}"
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
    if(tokens.at(2).simplified() == "done")
        mDone = true;
    else if(tokens.at(2).simplified() == "undone")
        mDone = false;
    else
        return false;

    //
    return true;
}

QString clRemDayScheduleSession::print() const
//format: "$prefix; $hr-min-range; {done, undone}"
{
    Q_ASSERT(! mHrMinRange.is_empty());

    QString S;
    S += mPrefix;
    S += "; ";
    S += mHrMinRange.print(2);
    S += "; ";
    S += mDone ? "done" : "undone";

    return S;
}
