#ifndef DATAELEM_GEVENT_H
#define DATAELEM_GEVENT_H

#include <QString>
#include <QStringList>
#include <QSet>

/* A "GEvent" (generalized event) is an event, start of a situation, or end of a situation. */

class clDataElem_GEvent
{
public:
    enum enType {Event, StartOfSituation, EndOfSituation};

    clDataElem_GEvent() {}
    clDataElem_GEvent(enType type, const QString &event_or_situation_name);

    bool parse_and_set(const QString &S); //format:
                                          //  start of situation <situation>
                                          //  end of situation <situation>
                                          //  event <event>

    void set_as_event(const QString &event_name);
    void set_as_start_of_situation(const QString &situation_name);
    void set_as_end_of_situation(const QString &situation_name);
    void set(enType type, const QString &event_or_situation_name);

    //
    bool is_empty() const { return mName.isEmpty(); }

    bool is_event() const;
    bool is_the_event(const QString &event) const;

    bool is_start_of_situation() const;
    bool is_start_of_the_situation(const QString &situation) const;
    bool is_end_of_situation() const;
    bool is_end_of_the_situation(const QString &situation) const;

    enType get_type() const;
    QString get_name() const;

    //QSet<QString> get_induced_names() const;
           //Example: If `mName` is "A:B:C", the returned will be {"A", "A:B", "A:B:C"}.

    QString print() const;

    bool operator == (const clDataElem_GEvent &another) const;
    bool operator != (const clDataElem_GEvent &another) const;
                     //empty g-events are considered equal

private:
    enType mType;
    QString mName; //'mName' is empty <=> *this is empty
};


#endif // DATAELEM_GEVENT_H
