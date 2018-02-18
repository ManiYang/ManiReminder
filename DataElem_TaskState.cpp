#include "DataElem_TaskState.h"
#include "utility_DOM.h"

clDataElem_TaskState::clDataElem_TaskState()
    : mState(Todo)
//initialize state as Todo
{
}

void clDataElem_TaskState::set_state(const enState new_state, const QDate &shifted_date)
{
    if(new_state == DateShifted)
        Q_ASSERT(shifted_date.isValid());
    else
        Q_ASSERT(shifted_date.isNull());

    mState = new_state;
    if(mState == DateShifted)
        mShiftedDate = shifted_date;
}

QDate clDataElem_TaskState::get_shifted_date() const
{
    Q_ASSERT(mState == DateShifted);
    return mShiftedDate;
}

bool clDataElem_TaskState::operator==(const clDataElem_TaskState &another) const
{
    if(mState != another.mState)
        return false;

    if(mState != DateShifted)
        return true;
    else
        return mShiftedDate == another.mShiftedDate;
}

bool clDataElem_TaskState::operator != (const clDataElem_TaskState &another) const
{
    return !(*this == another);
}

bool clDataElem_TaskState::parse_and_set(const QString &S_)
{
    QString S = S_.simplified();

    if(S == "todo")
        mState = Todo;
    else if(S == "done")
        mState = Done;
    else if(S == "skipped")
        mState = Skipped;
    else if(S.startsWith("date-shifted-to-"))
    {
        mShiftedDate = QDate::fromString(S.mid(16), "yyyy/M/d");
        if(! mShiftedDate.isValid())
            return false;
        mState = DateShifted;
    }
    else
        return false;

    //
    return true;
}

QString clDataElem_TaskState::print() const
{
    QString S;
    switch(mState)
    {
    case Todo:
        S = "todo";
        break;

    case Done:
        S = "done";
        break;

    case Skipped:
        S = "skipped";
        break;

    case DateShifted:
        S = "date-shifted-to-"+mShiftedDate.toString("yyyy/M/d");
        break;
    }
    return S;
}
