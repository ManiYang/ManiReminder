#ifndef REM_SPEC_ABSTRACT_H
#define REM_SPEC_ABSTRACT_H

#include <QObject>
#include <QDateTime>
#include <QMultiMap>

class clAction;

class clRemSpec_Abstract : public QObject
{
    Q_OBJECT

public:
    explicit clRemSpec_Abstract(const int forth_scheduling_duration_minutes,
                                QObject *parent = nullptr);

    virtual void clear();

    //
    void On_time_reached(const QDateTime &t);

    //
    virtual bool is_empty() const = 0; //if empty, won't perform any action

    QMultiMap<QDateTime,QString> get_scheduled_actions() const;

signals:
    void to_set_alarm(QDateTime t);

protected:
    /* ====== The Forth-Scheduling Mechanism Interface ======
     *
     * After `start_forth_scheduling()` is called at time t, `get_actions_within_time_range()`
     * will be called to get the actions within the time interval (t, t+Dt], at the end of which
     * it will be called again to get the actions within the following consecutive time interval. This process
     * is repeated until `stop_forth_scheduling()` is called.
     *
     * Every time the actions are obtained, they are scheduled so that `perform_actions()` will
     * be called when it's time to perform the action (specified in the function argument),
     * after which the actions are removed from being scheduled.
     *
     * Additional actions can be inserted at any time using
     * `insert_actions_up_to_next_forth_scheduling_time()`, but any action after the next
     * forth-scheduling time is ignored.
     *
     * Scheduled actions can be canceled using, e.g., `cancel_actions()`, without stoping
     * forth-scheduling. Note that `stop_forth_scheduling()` does not cancel the actions that
     * remain scheduled. */

    // provided by subclass:
    virtual void perform_actions(const QMultiMap<QDateTime,clAction> &actions) = 0;
    virtual QMultiMap<QDateTime,clAction> get_actions_within_time_range(
                                                  const QDateTime &t0, const QDateTime &t1) = 0;
                                                  //within (t0,t1]
                                                  //This function can stop forth scheduling.
    virtual QString get_action_name(const int action_code) const = 0;

    // used by subclass:
    void start_forth_scheduling();
    void stop_forth_scheduling(); //remaining scheduled actions are NOT canceled
    bool is_forth_scheduling_started() const;
    QDateTime get_next_forth_scheduling_time() const; //forth-scheduling must have been started
    void insert_actions_up_to_next_forth_scheduling_time(
                                                  const QMultiMap<QDateTime,clAction> &actions);
    void cancel_actions_w_action_code(const int action_code);
                                 //Cancel all currently scheduled actions with the action code.
    void cancel_actions_w_source(const int source);
                                 //Cancel all currently scheduled actions with the source.
    void cancel_actions_w_action_code_et_source(const int action_code, const int source);
                   //Cancel all currently scheduled actions with the action code and the source.
    void cancel_actions(); //Cancel all currently scheduled actions, but forth-scheduling is
                           //NOT stopped.

private:
    int mForthSchedulingDuration; //(minute)

    QDateTime mNextForthSchedulingTime;
    QMultiMap<QDateTime,clAction> mActions; //does not include forth-scheduling

    void forth_schedule(const QDateTime &from);
};

////

class clAction
{
public:
    clAction(int code_=0, int source_=0) : code(code_), source(source_) {}

    int code; //action code
    int source; //who causes this action?
};

#endif // REM_SPEC_ABSTRACT_H
