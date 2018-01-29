#ifndef CLREMSPEC_SITUATIONBINDING_H
#define CLREMSPEC_SITUATIONBINDING_H

#include <QStringList>
#include <QMap>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>
#include "DataElem_GEvent.h"

/* A reminder is active under situation binding iff it has binding situations and
 * one of the binding situations has started (selected or induced).
 * Otherwise, it is inactive under situation binding. */

class clRemSpec_SituationBinding
{
public:
    clRemSpec_SituationBinding();

    void clear();

    // start-up
    // recommended usage:
    //   1. call `parse_and_set(...)`
    //   2. make sure `On_gevent(...)` is called whenever a g-event happens
    //   3. call `initialize(...)`
    //   4. use `get_activeness()` to get current activeness

    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);
                       //Returns false iff there is error.

    void initialize(const QSet<QString> &selected_and_induced_situations);

    // update binding situations
    void update_settings(const clRemSpec_SituationBinding &another, //use settings only
                         const QSet<QString> &selected_and_induced_situations);
                         //Use 'get_activeness()' to get new activeness.

    //
    void On_gevent(const clDataElem_GEvent &gevent);
                   //Call this to inform `*this` that the g-event just happened.
                   //Use `get_activeness()` to get new activeness of the reminder.

    //
    bool get_activeness() const; //get activeness according to `mBindingSitStarted[]`

    // interface for settings only
    bool is_empty() const { return mBindingSituations.isEmpty(); }
    QSet<QString> get_situations_involved() const;

    QString print() const; //print the settings
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const; //add the settings

    bool operator == (const clRemSpec_SituationBinding &another) const; //compare only settings

    //
    void operator = (const clRemSpec_SituationBinding &another);

private:
    QList<QString> mBindingSituations; //binding situations

    //
    QList<bool> mBindingSitStarted; //`mBindingSitStarted[i]` is whether `mBindingSituations[i]`
                                    //has started (selected or induced).
};

#endif // CLREMSPEC_SITUATIONBINDING_H
