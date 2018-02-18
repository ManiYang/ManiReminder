#ifndef CLREMSPEC_TIMERANGEBINDING_H
#define CLREMSPEC_TIMERANGEBINDING_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "RemSpec_Abstract.h"
#include "DataElem_TimeRangeCollection.h"

/* A reminder is active under time-range binding iff it has binding time-ranges and
 * the current time is within one of the binding time-ranges. */

class clRemSpec_TimeRangeBinding : public clRemSpec_Abstract
{
    Q_OBJECT

public:
    explicit clRemSpec_TimeRangeBinding(QObject *parent = nullptr);

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_time_reached(...)` and signal `to_set_alarm()` are properly connected
    //   3. call `initialize()`
    //   4. use `get_activeness()` to get current activeness
    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);
                       //Returns false iff there is error in parsing.

    void initialize(); //also start forth scheduling if `*this` is not empty

    // update settings
    //   * All connections must be built properly before calling `update_settings(...)`.
    //   * Use 'get_activeness()' afterward to get new activeness.
    void update_settings(const clRemSpec_TimeRangeBinding &another);
    bool update_settings(const QStringList &time_range_collection_settings,
                         QString &parse_error_msg); //returns false iff there's error

    //
    bool get_activeness() const { return mState; }

    // interface for settings only
    bool is_empty() const { return mTimeRangeCollections.isEmpty(); }

    QList<clUtil_HrMinRange> get_time_ranges_on_date(const QDate &date) const;
                             //Get time ranges starting on date `date`.
                             //Returned list will be separated and in ascending order.

    QString print() const; //print the settings
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const; //add the settings

    bool operator == (const clRemSpec_TimeRangeBinding &another) const;
                     //Compare settings only.
                     //Return false if the settings are not literally the same, even if they are
                     //effectively the same.

    void operator = (const clRemSpec_TimeRangeBinding &another);
                    //Clear and copy settings only.
                    //[You can call `initialize()` to let `*this` start working, but all
                    // connections must be built properly beforehand.]

protected:
    virtual void perform_actions(const QMultiMap<QDateTime,clAction> &actions);
    virtual QMultiMap<QDateTime,clAction> get_actions_within_time_range(const QDateTime &t0,
                                                                        const QDateTime &t1);
                                                                        //within (t0,t1]
    virtual QString get_action_name(const int action_code) const;

private:
    QList<clDataElem_TimeRangeCollection> mTimeRangeCollections; //binding time-ranges

    //
    enum enAction {Activate, Deactivate};
    bool mState;
    void determine_state(const QDateTime &at);
};

#endif // CLREMSPEC_TIMERANGEBINDING_H
