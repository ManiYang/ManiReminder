#include "DataElem_GEvent.h"

clDataElem_GEvent::clDataElem_GEvent(enType type, const QString &event_or_situation_name)
    : mType(type), mName(event_or_situation_name)
{
    Q_ASSERT(type==Event || type==StartOfSituation || type==EndOfSituation);
    Q_ASSERT(! event_or_situation_name.isEmpty());
}

bool clDataElem_GEvent::parse_and_set(const QString &S_)
//format:
//  start of situation <situation>
//  end of situation <situation>
//  event <event>
{
    mName.clear();

    QString S = S_.simplified();

    QString event_or_sit;
    if(S.startsWith("start of situation "))
    {
        mType = StartOfSituation;
        event_or_sit = S.mid(19);
    }
    else if(S.startsWith("end of situation "))
    {
        mType = EndOfSituation;
        event_or_sit = S.mid(17);
    }
    else if(S.startsWith("event "))
    {
        mType = Event;
        event_or_sit = S.mid(6);
    }
    else
        return false;

    //
    if(event_or_sit.isEmpty())
        return false;

    mName = event_or_sit;
    return true;
}

QString clDataElem_GEvent::print() const
{
    Q_ASSERT(! is_empty());

    QString S;
    switch(mType)
    {
    case Event:
        S = "event "+mName;
        break;

    case StartOfSituation:
        S = "start of situation "+mName;
        break;

    case EndOfSituation:
        S = "end of situation "+mName;
        break;

    default:
        Q_ASSERT(false);
    }

    return S;
}

void clDataElem_GEvent::set(enType type, const QString &event_or_situation_name)
{
    Q_ASSERT(type==Event || type==StartOfSituation || type==EndOfSituation);
    Q_ASSERT(! event_or_situation_name.isEmpty());

    mType = type;
    mName = event_or_situation_name;
}

void clDataElem_GEvent::set_as_event(const QString &event_name)
{
    Q_ASSERT(! event_name.isEmpty());

    mType = Event;
    mName = event_name;
}

void clDataElem_GEvent::set_as_start_of_situation(const QString &situation_name)
{
    Q_ASSERT(! situation_name.isEmpty());

    mType = StartOfSituation;
    mName = situation_name;
}

void clDataElem_GEvent::set_as_end_of_situation(const QString &situation_name)
{
    Q_ASSERT(! situation_name.isEmpty());

    mType = EndOfSituation;
    mName = situation_name;
}

bool clDataElem_GEvent::is_event() const
{
    if(mName.isEmpty())
        return false;

    return mType == Event;
}

bool clDataElem_GEvent::is_the_event(const QString &event) const
{
    if(mName.isEmpty())
        return false;

    Q_ASSERT(! event.isEmpty());
    return mType == Event && mName == event;
}

bool clDataElem_GEvent::is_start_of_situation() const
{
    if(mName.isEmpty())
        return false;

    return mType == StartOfSituation;
}

bool clDataElem_GEvent::is_start_of_the_situation(const QString &situation) const
{
    if(mName.isEmpty())
        return false;

    Q_ASSERT(! situation.isEmpty());
    return mType == StartOfSituation && mName == situation;
}

bool clDataElem_GEvent::is_end_of_situation() const
{
    if(mName.isEmpty())
        return false;

    return mType == EndOfSituation;
}

bool clDataElem_GEvent::is_end_of_the_situation(const QString &situation) const
{
    if(mName.isEmpty())
        return false;

    Q_ASSERT(! situation.isEmpty());
    return mType == EndOfSituation && mName == situation;
}

bool clDataElem_GEvent::operator == (const clDataElem_GEvent &another) const
//empty g-events are considered equal
{
    if(is_empty())
        return another.is_empty();
    return mType == another.mType && mName == another.mName;
}

bool clDataElem_GEvent::operator != (const clDataElem_GEvent &another) const
//empty g-events are considered equal
{
    return !(*this == another);
}

clDataElem_GEvent::enType clDataElem_GEvent::get_type() const
{
    Q_ASSERT(! mName.isEmpty());
    return mType;
}

QString clDataElem_GEvent::get_name() const
{
    Q_ASSERT(! mName.isEmpty());
    return mName;
}

//QSet<QString> clDataElem_GEvent::get_induced_names() const
////Example: If `mName` is "A:B:C", the returned string list will be {"A", "A:B", "A:B:C"}.
//{
//    Q_ASSERT(! mName.isEmpty());

//    QSet<QString> set;
//    QStringList tokens = mName.split(':');
//    QString induced;
//    foreach(QString token, tokens)
//    {
//        if(! induced.isEmpty())
//            induced += ':';
//        induced += token;

//        set << induced;
//    }

//    return set;
//}
