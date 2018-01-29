#ifndef CLREMSPEC_GEVENTDURATIONBINDING_H
#define CLREMSPEC_GEVENTDURATIONBINDING_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include "DataElem_GEvent.h"
typedef clDataElem_GEvent clGEvent;


class clRemSpec_GEventDurationBinding : public QObject
{
    Q_OBJECT

public:
    explicit clRemSpec_GEventDurationBinding(QObject *parent = Q_NULLPTR);

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_gevent(...)`, `On_time_reached(...)` and signal `to_set_alarm()`
    //      are properly connected
    //   3. get current time `t0` and get g-event history/record `hist`
    //   4. call `initialize(t0, hist)`
    //   5. use `get_activeness()` to get current activeness
    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);
                       //Return false iff there's error in parsing.

    void initialize(const QDateTime &t0, const QMultiMap<QDateTime, clGEvent> &gevent_history);
                    //Set internal state to that at time `t0`.
                    //`gevent_history` should be the g-event record at `t0`.

    // update settings
    void update_settings(const clRemSpec_GEventDurationBinding &another,
                         const QDateTime &t0,
                         const QMultiMap<QDateTime,clGEvent> &gevent_history);
    bool update_settings(QList<std::tuple<clGEvent,int,int> > gevents_hrs_mins,
                         const QDateTime &t0,
                         const QMultiMap<QDateTime, clGEvent> &gevent_history,
                         QString &parse_error_msg); //returns false iff there's error
         //Will set internal state to that at time `t0`.
         //`gevent_history` should be the g-event record at `t0`.
         //All connections must be built properly before calling `update_settings(...)`.
         //Use `get_activeness()` afterward to get current activeness.

    //
    void On_gevent(const clGEvent &gevent); //Use `get_activeness()` to get new activeness.
    void On_time_reached(const QDateTime &t); //Use `get_activeness()` to get new activeness.

    //
    bool get_activeness() const { return mStartCount > 0; }

    // interface for settings only
    bool is_empty() const { return mGEvents.isEmpty(); }

    QSet<QString> get_situations_involved() const;
    QSet<QString> get_events_involved() const;

    QString print() const; //print settings
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const; //add settings

    bool operator == (const clRemSpec_GEventDurationBinding &another) const;
                     //compare settings only

    void operator = (const clRemSpec_GEventDurationBinding &another);
                    //Clear and copy settings only.
                    //[You can call `initialize(...)` to let `*this` start working properly,
                    // but all connections must be built beforehand.]

    //
    QMultiMap<QDateTime,QString> get_scheduled_actions() const;

signals:
    void to_set_alarm(QDateTime t);

private:
    QList<clGEvent> mGEvents; //should not have duplicates
    QList<int> mDurations; //mDurations[i], must be > 0, is the duration for mGEvents[i]
                           //in minutes

    //
    int mStartCount;
    QList<QDateTime> mEnds;
};

#endif // CLREMSPEC_GEVENTDURATIONBINDING_H
