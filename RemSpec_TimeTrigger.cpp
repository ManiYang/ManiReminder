#include <QSet>
#include "RemSpec_TimeTrigger.h"
#include "utility_DOM.h"

const int ForthSchedulingDuration = 55; //(minute)

clRemSpec_TimeTrigger::clRemSpec_TimeTrigger(QObject *parent)
    : clRemSpec_Abstract(ForthSchedulingDuration, parent)
{
}

bool clRemSpec_TimeTrigger::parse_and_set(QDomElement &spec_setting, QString &parse_error_msg)
//Return false iff there's parsing error, in which case the error message will
//be copied to `parse_error_msg`.
{
    clear();

    // find directly under `spec_setting':
    //   <time-trigger> time collection 1 </time-trigger>
    //   <time-trigger> time collection 2 </time-trigger>
    //   ...
    QStringList time_collection_settings
            = nsDomUtil::get_texts_of_child_elements(spec_setting, "time-trigger");

    foreach(QString TC_setting, time_collection_settings)
    {
        clDataElem_TimeCollection TC;
        bool ch = TC.parse_and_set(TC_setting);
        if(!ch)
        {
            parse_error_msg = QString("Could not parse the time-collection setting \"%1\".")
                              .arg(TC_setting);
            return false;
        }

        if(! TC.is_empty())
        {
            if(mTimeCollections.contains(TC))
            {
                parse_error_msg = QString("Duplicated time-collection settings in \"%1\".")
                                  .arg(TC_setting);
                return false;
            }
            mTimeCollections << TC;
        }
    }

    //
    return true;
}

void clRemSpec_TimeTrigger::initialize()
{
    if(! mTimeCollections.isEmpty())
        start_forth_scheduling();
}

void clRemSpec_TimeTrigger::update_settings(const clRemSpec_TimeTrigger &another)
{
    clear();
    mTimeCollections = another.mTimeCollections;
    initialize();
}

bool clRemSpec_TimeTrigger::update_settings(const QStringList &time_collection_settings,
                                            QString &parse_error_msg)
//return false iff there's error
{
    clear(); //all scheduled actions and forth-scheduling are canceled

    foreach(QString S, time_collection_settings)
    {
        clDataElem_TimeCollection TC;
        bool ch = TC.parse_and_set(S);
        if(!ch)
        {
            parse_error_msg = QString("Could not parse the time-collection setting \"%1\".")
                              .arg(S);
            return false;
        }

        if(! TC.is_empty())
        {
            if(mTimeCollections.contains(TC))
            {
                parse_error_msg = QString("Duplicated time-collection settings in \"%1\".")
                                  .arg(S);
                return false;
            }
            mTimeCollections << TC;
        }
    }

    //
    initialize();

    //
    return true;
}

void clRemSpec_TimeTrigger::clear()
{
    mTimeCollections.clear();
    clRemSpec_Abstract::clear();
}

QString clRemSpec_TimeTrigger::print() const
{
    if(is_empty())
        return QString();

    QString S;
    for(int i=0; i<mTimeCollections.count(); i++)
    {
        if(i > 0)
            S += '\n';
        S += "time-trigger: ";
        S += mTimeCollections.at(i).print();
    }
    return S;
}

void clRemSpec_TimeTrigger::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    for(int i=0; i<mTimeCollections.size(); i++)
    {
        nsDomUtil::add_child_element_w_text(doc, parent, "time-trigger",
                                            mTimeCollections[i].print());
    }
}

bool clRemSpec_TimeTrigger::operator ==(const clRemSpec_TimeTrigger &another) const
//compare settings only
{
    int n = mTimeCollections.size();
    if(another.mTimeCollections.size() != n)
        return false;

    for(int i=0; i<n; i++)
    {
        if(! another.mTimeCollections.contains(mTimeCollections.at(i)))
            return false;
    }
    return true;
}

void clRemSpec_TimeTrigger::operator = (const clRemSpec_TimeTrigger &another)
//clear and copy settings
{
    clear();
    mTimeCollections = another.mTimeCollections;
}

////

void clRemSpec_TimeTrigger::perform_actions(const QMultiMap<QDateTime,clAction> &actions)
{
    for(int i=0; i<actions.size(); i++)
        emit to_trigger();
}

QMultiMap<QDateTime,clAction> clRemSpec_TimeTrigger::get_actions_within_time_range(
                                                      const QDateTime &t0, const QDateTime &t1)
//within (t0, t1]
{
    QSet<QDateTime> triggering_times;
    foreach(clDataElem_TimeCollection TC, mTimeCollections)
    {
        QList<QDateTime> times = TC.get_times_within_time_range(t0, t1); //(`t0`, `t1`]
        triggering_times += QSet<QDateTime>::fromList(times);
    }

    QMultiMap<QDateTime,clAction> actions;
    foreach(QDateTime t, triggering_times)
        actions.insert(t, clAction());

    return actions;
}

QString clRemSpec_TimeTrigger::get_action_name(const int action_code) const
{
    Q_UNUSED(action_code);
    return "trigger";
}
