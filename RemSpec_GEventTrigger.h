#ifndef REM_SPEC_GEVENT_TRIGGER_H
#define REM_SPEC_GEVENT_TRIGGER_H

#include <QDomDocument>
#include <QDomElement>
#include "DataElem_GEvent.h"
#include "DataElem_TimeSequence.h"
#include "RemSpec_Abstract.h"
typedef clDataElem_GEvent clGEvent;

class clRemSpec_GEventTrigger : public clRemSpec_Abstract
{
    Q_OBJECT

public:
    explicit clRemSpec_GEventTrigger(QObject *parent = nullptr);

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_gevent(...)`, `On_time_reached(...)` and signals `to_set_alarm()`,
    //      `to_trigger()` are properly connected
    //   3. get current time `t0` and get g-event history/record `hist`
    //   4. call `initialize(t0, hist)`

    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);

    void initialize(const QDateTime &t0, const QMultiMap<QDateTime, clGEvent> &gevent_history);
                    //Set internal state to that at time `t0`.
                    //`gevent_history` should be the g-event record at `t0`.

    // update settings
    void update_settings(const clRemSpec_GEventTrigger &another,
                         const QDateTime &t0, const QMap<QDateTime,clGEvent> &gevent_history);
    bool update_settings(const QStringList &gevent_trigger_settings,
                         const QDateTime &t0, const QMap<QDateTime,clGEvent> &gevent_history,
                         QString &parse_error_msg); //return false iff there's error
         //Will set internal state to that at time `t0`.
         //`gevent_history` should be the g-event record at `t0`.
         //All connections must be built properly before calling `update_settings(...)`.

    //
    void On_gevent(const clDataElem_GEvent &gevent);

    // interface for settings only
    bool is_empty() const { return mTriggeringGEvents.isEmpty(); }

    QSet<QString> get_situations_involved() const;
    QSet<QString> get_events_involved() const;

    QString print() const;
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

    bool operator == (const clRemSpec_GEventTrigger &another) const;
                     //compare settings only

    void operator = (const clRemSpec_GEventTrigger &another);
                     //Clear and copy settings only.
                     //[You can call `initialize(...)` to let `*this` start working properly,
                     // but all connections must be built beforehand.]

signals:
    void to_trigger();

protected:
    virtual void perform_actions(const QMultiMap<QDateTime,clAction> &actions);
    virtual QMultiMap<QDateTime,clAction> get_actions_within_time_range(const QDateTime &t0,
                                                                        const QDateTime &t1);
                                                                        //within (t0,t1]
    virtual QString get_action_name(const int action_code) const;

private:
    QList<clDataElem_GEvent> mTriggeringGEvents;
    QList<clDataElem_TimeSequence> mTimeSequences; //[i] is for `mTriggeringGEvents[i]`
    QList<clDataElem_GEvent> mClosingGEvents; //[i] is for `mTriggeringGEvents[i]`
      //
      // When `mTriggeringGEvents[i]` happens at t_i, triggers are issued at the times
      // specified by `mTimeSequences[i]`, relative to t_i, until `mClosingGEvents[i]` (if
      // not empty) happens. If `mClosingGEvents[i]` is empty, `mTimeSequences[i]` must be
      // finite.
      //
      // The start time of `mTimeSequences[i]` (must be >=0) is treated as the delay
      // time-interval between t_i and the first trigger due to that g-event. Thus, a
      // repeated occurrence of `mTriggeringGEvents[i]` overrides any previous occurrence
      // (that is, it cancels any trigger due to previous occurrences).

    //
    QMap<int,QDateTime> mActiveGEvents;
      //records the indices and happening time of the triggering event that have happened
      //and still have corresponding triggers to be issued
};

#endif // REM_SPEC_GEVENT_TRIGGER_H
