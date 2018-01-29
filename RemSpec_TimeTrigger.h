#ifndef REM_SPEC_TIME_TRIGGER_H
#define REM_SPEC_TIME_TRIGGER_H

#include <QDomDocument>
#include <QDomElement>
#include "RemSpec_Abstract.h"
#include "DataElem_TimeCollection.h"

class clRemSpec_TimeTrigger : public clRemSpec_Abstract
{
    Q_OBJECT

public:
    explicit clRemSpec_TimeTrigger(QObject *parent = nullptr);

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_time_reached(...)` and signals `to_trigger()`, `to_set_alarm()`
    //      are properly connected
    //   3. call `initialize()`
    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);
         //Return false iff there's parsing error, in which case the error message will
         //be copied to `parse_error_msg`.

    void initialize();

    // update settings
    void update_settings(const clRemSpec_TimeTrigger &another);
    bool update_settings(const QStringList &time_collection_settings,
                         QString &parse_error_msg); //return false iff there's error
         //All connections must be built properly before calling `update_settings(...)`

    // interface for settings only
    bool is_empty() const { return mTimeCollections.isEmpty(); }

    QString print() const; //print settings
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const; //add settings

    bool operator == (const clRemSpec_TimeTrigger &another) const;
                     //compare settings only

    void operator = (const clRemSpec_TimeTrigger &another);
                    //Clear and copy settings only.
                    //[You can call `initialize()` to let `*this` start working, but all
                    // connections must be built properly beforehand.]

signals:
    void to_trigger();

protected:
    virtual void perform_actions(const QMultiMap<QDateTime,clAction> &actions);
    virtual QMultiMap<QDateTime,clAction> get_actions_within_time_range(const QDateTime &t0,
                                                                        const QDateTime &t1);
                                                                        //within (t0,t1]
    virtual QString get_action_name(const int action_code) const;

private:
    QList<clDataElem_TimeCollection> mTimeCollections; //should not have duplicates
};

#endif // REM_SPEC_TIME_TRIGGER_H
