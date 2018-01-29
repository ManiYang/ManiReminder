#include "RemSpec_SituationBinding.h"
#include "utility_DOM.h"

clRemSpec_SituationBinding::clRemSpec_SituationBinding()
{
}

bool clRemSpec_SituationBinding::parse_and_set(QDomElement &spec_setting,
                                               QString &parse_error_msg)
//Returns false iff there is error.
{
    clear();

    //directly under 'parent':
    //   <situation-binding> Sit1, Sit2, ... </situation-binding>
    //   <situation-binding> Sit1a, Sit2a, ... </situation-binding>
    //   ...
    QStringList SL = nsDomUtil::get_texts_of_child_elements(spec_setting, "situation-binding");
    foreach(QString SitList, SL)
    {
        QStringList tokens = SitList.split(',');
        foreach(QString sit, tokens)
        {
            sit = sit.simplified();
            if(sit.isEmpty())
            {
                parse_error_msg
                        = QString("Error in parsing situation list \"%1\".").arg(SitList);
                return false;
            }

            mBindingSituations << sit;
            mBindingSitStarted << false;
        }
    }

    //
    return true;
}

void clRemSpec_SituationBinding::initialize(
                                          const QSet<QString> &selected_and_induced_situations)
{
    for(int i=0; i<mBindingSituations.size(); i++)
    {
        QString binding_sit = mBindingSituations.at(i);
        mBindingSitStarted[i] = selected_and_induced_situations.contains(binding_sit);
    }
}

void clRemSpec_SituationBinding::clear()
{
    mBindingSitStarted.clear();
    mBindingSitStarted.clear();
}

void clRemSpec_SituationBinding::On_gevent(const clDataElem_GEvent &gevent)
//Call this to inform *this that the g-event just happened.
{
    if(is_empty())
        return;

    if(!gevent.is_start_of_situation() && !gevent.is_end_of_situation())
        return;

    //
    const QString sit = gevent.get_name();
    if(! mBindingSituations.contains(sit)) //('sit' is not among the binding situations)
        return;

    // (Now 'sit' is one of the binding situations.)
    int i = mBindingSituations.indexOf(sit);
    Q_ASSERT(i != -1);
    if(gevent.is_start_of_situation())
        mBindingSitStarted[i] = true;
    else
        mBindingSitStarted[i] = false;
}

QSet<QString> clRemSpec_SituationBinding::get_situations_involved() const
{
    return QSet<QString>::fromList(mBindingSituations);
}

QString clRemSpec_SituationBinding::print() const
{
    QString S = "situation binding: ";
    QStringList binding_sits(mBindingSituations);
    S += binding_sits.join(", ");
    return S;
}

void clRemSpec_SituationBinding::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    if(is_empty())
        return;

    QString S = QStringList(mBindingSituations).join(", ");
    nsDomUtil::add_child_element_w_text(doc, parent, "situation-binding", S);
}

void clRemSpec_SituationBinding::update_settings(
                                         const clRemSpec_SituationBinding &another,
                                         const QSet<QString> &selected_and_induced_situations)
{
    mBindingSituations = another.mBindingSituations;
    initialize(selected_and_induced_situations);
}

bool clRemSpec_SituationBinding::get_activeness() const
//get activeness according to `mBindingSitStarted[]`
{
    //Return true iff `mBindingSitStarted[]` is non-empty and one of `mBindingSitStarted[]`
    //is true.
    foreach(bool started, mBindingSitStarted)
    {
        if(started)
            return true;
    }
    return false;
}

void clRemSpec_SituationBinding::operator =(const clRemSpec_SituationBinding &another)
{
    mBindingSituations = another.mBindingSituations;
    mBindingSitStarted = another.mBindingSitStarted;
}

bool clRemSpec_SituationBinding::operator ==(const clRemSpec_SituationBinding &another) const
//compare only the settings
{
    QSet<QString> binding_sits = QSet<QString>::fromList(mBindingSituations);
    QSet<QString> binding_sits_2 = QSet<QString>::fromList(another.mBindingSituations);
    return binding_sits == binding_sits_2;
}
