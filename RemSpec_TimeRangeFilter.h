#ifndef REM_SPEC_TIME_RANGE_FILTER_H
#define REM_SPEC_TIME_RANGE_FILTER_H

#include <QDomDocument>
#include <QDomElement>
#include "RemSpec_Abstract.h"
#include "DataElem_TimeRangeCollection.h"

class clRemSpec_TimeRangeFilter : public clRemSpec_Abstract
{
    Q_OBJECT
public:
    explicit clRemSpec_TimeRangeFilter(QObject *parent = nullptr);

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_time_reached(...)` and signal `to_set_alarm()` are properly connected
    //   3. call `initialize()`
    //   4. use `passing()` to get current state
    bool parse_and_set(QDomElement &spec_setting, QString &error_msg);

    void initialize();

    // update settings
    void update_settings(const clRemSpec_TimeRangeFilter &another);
    bool update_settings(const QStringList &time_range_collection_settings,
                         QString &parse_error_msg);
         //All connections must be built properly before calling this.
         //Use 'passing()' afterward to get new state.

    //
    bool passing() const { return mPassing; }

    // interface for settings only
    bool is_empty() const { return mTimeRangeCollections.isEmpty(); }

    QString print() const;
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

    bool operator == (const clRemSpec_TimeRangeFilter &another) const;
                      //Compare settings only.
                      //Return false if the settings are not literally the same, even if they
                      //are effectively the same.

    void operator = (const clRemSpec_TimeRangeFilter &another);
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
    QList<clDataElem_TimeRangeCollection> mTimeRangeCollections;
      // Time t passes the filter of `*this` iff t is in the intersection of all time-ranges
      // in `mTimeRangeCollections[]`, if `mTimeRangeCollections[]` is not empty.
      // If `mTimeRangeCollections[]` is empty, any time t passes the filter.

    enum enAction {FilterIn, FilterOut};
    bool mPassing; //whether current time passes the filter
    void determine_state(const QDateTime &at); //determine `mPassing`
};

#endif // REM_SPEC_TIME_RANGE_FILTER_H
