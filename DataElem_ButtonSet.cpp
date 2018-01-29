#include <QDebug>
#include "DataElem_ButtonSet.h"
#include "utility_DOM.h"
#include "utility_general.h"
typedef clUtil_Button clButton;

//// predefined button sets ////

// list the names here:
extern const QStringList G_PredefinedButtonSets = QStringList()
        << "close" << "check" << "half-check" << "fail" << "violate"
        << "proceed/pause"
        << "close:done,skip" << "close:proceed,skip";

//
clDataElem_ButtonSet_Basic
    gButtonClose    (clButton("Close", "",               true,  "closed"),       "close"),
    gButtonCheck    (clButton("",      "check.png",      false, "checked"),      "check"),
    gButtonHalfCheck(clButton("",      "half-check.png", false, "half-checked"), "half-check"),
    gButtonFail     (clButton("",      "cross.png",      false, "failed"),       "fail"),
    gButtonViolate  (clButton("",      "cross.png",      false, "violated"),     "violate");

clDataElem_ButtonSet_Alternate
    gAlternateButtons_ProceedPause(clButton("", "proceed.png", false, "proceed"),
                                   clButton("", "pause.png",   false, "paused"),
                                   "proceed/pause");

clDataElem_ButtonSet_Menu
    gClosingMenuButton_DoneSkip(clButton("Close", "", false, ""),
                                QList<clButton>()
                                    << clButton("Done", "check.png", true, "done")
                                    << clButton("Skip", "cross.png", true, "skipped"),
                                "close:done,skip"),
    gClosingMenuButton_ProceedSkip(
                                clButton("Close", "", false, ""),
                                QList<clButton>()
                                    << clButton("Proceed", "proceed.png", true, "proceed")
                                    << clButton("Skip",    "cross.png",   true, "skipped"),
                                "close:proceed,skip");

/////////////////////////////////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Abstract::clDataElem_ButtonSet_Abstract(
                                                             const QString &predefined_name)
{
    if(! predefined_name.isEmpty())
        Q_ASSERT(G_PredefinedButtonSets.contains(predefined_name));
    mPredefined = predefined_name;
}

/////////////////////////////////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Basic::clDataElem_ButtonSet_Basic(const clUtil_Button &button,
                                                             const QString &predefined_name)
    : clDataElem_ButtonSet_Abstract(predefined_name)
{
    mButton = button;
}

clDataElem_ButtonSet_Basic::clDataElem_ButtonSet_Basic(
                                                 const clDataElem_ButtonSet_Basic &another)
    : clDataElem_ButtonSet_Abstract(another)
{
    mButton = another.mButton;
}

void clDataElem_ButtonSet_Basic::set_button(const clUtil_Button &button)
{
    mButton = button;
}

bool clDataElem_ButtonSet_Basic::parse_and_set(QDomElement &parent, QString &error_msg)
//under `parent`:
//  <button> button settings </button>
{
    if(nsDomUtil::get_count_of_child_elements(parent, "button") != 1)
    {
        error_msg = "Should have one and only one <button>.";
        return false;
    }

    QDomElement child = parent.firstChildElement("button");
    bool ch = mButton.parse_and_set(child);
    if(!ch)
    {
        error_msg = "Failed parse the contents under <button>.";
        return false;
    }

    return true;
}

void clDataElem_ButtonSet_Basic::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    QDomElement elem_button = nsDomUtil::add_child_element(doc, parent, "button");
    mButton.add_to_XML(doc, elem_button);
}

QString clDataElem_ButtonSet_Basic::print() const
{
    if(! mPredefined.isEmpty())
        return mPredefined;

    //
    QString S = "custom: basic\n";
    S += QString("|  button (%1)").arg(mButton.print());
    return S;
}

clDataElem_ButtonSet_Abstract *clDataElem_ButtonSet_Basic::clone() const
{
    clDataElem_ButtonSet_Basic *BS = new clDataElem_ButtonSet_Basic;
    BS->mButton = mButton;
    BS->mPredefined = mPredefined;
    return BS;
}

bool clDataElem_ButtonSet_Basic::operator == (const clDataElem_ButtonSet_Basic &another) const
{
    return mButton == another.mButton;
}

/////////////////////////////////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Menu::clDataElem_ButtonSet_Menu(const clUtil_Button &base_button,
                                                   const QList<clUtil_Button> &menu_buttons,
                                                   const QString &predefined_name)
    : clDataElem_ButtonSet_Abstract(predefined_name)
{
    Q_ASSERT(! base_button.is_ending());
    mBaseButton = base_button;
    mMenuButtons = menu_buttons;
}

clDataElem_ButtonSet_Menu::clDataElem_ButtonSet_Menu(
                                                  const clDataElem_ButtonSet_Menu &another)
    : clDataElem_ButtonSet_Abstract(another)
{
    mBaseButton = another.mBaseButton;
    mMenuButtons = another.mMenuButtons;
}

bool clDataElem_ButtonSet_Menu::has_ending_button() const
{
    foreach(clUtil_Button b, mMenuButtons)
    {
        if(b.is_ending())
            return true;
    }
    return false;
}

void clDataElem_ButtonSet_Menu::set_base_button(const clUtil_Button &base_button)
{
    Q_ASSERT(! base_button.is_ending());
    mBaseButton = base_button;
}

void clDataElem_ButtonSet_Menu::set_menu_buttons(const QList<clUtil_Button> &menu_buttons)
{
    mMenuButtons = menu_buttons;
}

void clDataElem_ButtonSet_Menu::set(const clUtil_Button &base_button,
                                       const QList<clUtil_Button> &menu_buttons)
{
    Q_ASSERT(! base_button.is_ending());
    mBaseButton = base_button;
    mMenuButtons = menu_buttons;
}

bool clDataElem_ButtonSet_Menu::parse_and_set(QDomElement &parent, QString &error_msg)
//under `parent`:
//   <base-button> button settings </base-button>
//   <menu-button> button settings </menu-button>
//   <menu-button> button settings </menu-button>
//   ...
{
    mMenuButtons.clear();

    //
    if(nsDomUtil::get_count_of_child_elements(parent, "base-button") != 1)
    {
        error_msg = "Should have one and only one <base-button>.";
        return false;
    }

    QDomElement child = parent.firstChildElement("base-button");
    bool ch = mBaseButton.parse_and_set(child);
    if(!ch)
    {
        error_msg = "Failed to parse the contents under <base-button>.";
        return false;
    }

    //
    clButton button;
    child = parent.firstChildElement("menu-button");
    int i = 1;
    for( ; !child.isNull(); child = child.nextSiblingElement("menu-button"))
    {
        bool ch = button.parse_and_set(child);
        if(!ch)
        {
            error_msg = QString("Failed to parse the contents under %1 <menu-button>.")
                        .arg(print_ith(i));
            return false;
        }

        mMenuButtons << button;

        //
        i++;
    }

    //
    return true;
}

void clDataElem_ButtonSet_Menu::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    QDomElement elem_base_button = nsDomUtil::add_child_element(doc, parent, "base-button");
    mBaseButton.add_to_XML(doc, elem_base_button);
    foreach(clUtil_Button button, mMenuButtons)
    {
        QDomElement elem_menu_button
                = nsDomUtil::add_child_element(doc, parent, "menu-button");
        button.add_to_XML(doc, elem_menu_button);
    }
}

QString clDataElem_ButtonSet_Menu::print() const
{
    if(! mPredefined.isEmpty())
        return mPredefined;

    //
    QString S = "custom: menu\n";
    S += QString("|  base-button (%1)").arg(mBaseButton.print());
    for(int i=0; i<mMenuButtons.size(); i++)
        S += QString("\n|  menu-button (%1)").arg(mMenuButtons.at(i).print());
    return S;
}

clDataElem_ButtonSet_Abstract *clDataElem_ButtonSet_Menu::clone() const
{
    clDataElem_ButtonSet_Menu *BS = new clDataElem_ButtonSet_Menu;
    BS->mBaseButton = mBaseButton;
    BS->mMenuButtons = mMenuButtons;
    BS->mPredefined = mPredefined;
    return BS;
}

bool clDataElem_ButtonSet_Menu::operator == (const clDataElem_ButtonSet_Menu &another) const
{
    return mBaseButton == another.mBaseButton
            && mMenuButtons == another.mMenuButtons;
}

/////////////////////////////////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Alternate::clDataElem_ButtonSet_Alternate(
                                                               const clUtil_Button &button1,
                                                               const clUtil_Button &button2,
                                                               const QString &predefined_name)
    : clDataElem_ButtonSet_Abstract(predefined_name)
{
    Q_ASSERT(! button1.is_ending());
    Q_ASSERT(! button2.is_ending());
    mButton1 = button1;
    mButton2 = button2;
}

clDataElem_ButtonSet_Alternate::clDataElem_ButtonSet_Alternate(
                                             const clDataElem_ButtonSet_Alternate &another)
    : clDataElem_ButtonSet_Abstract(another)
{
    mButton1 = another.mButton1;
    mButton2 = another.mButton2;
}

std::tuple<clUtil_Button,clUtil_Button> clDataElem_ButtonSet_Alternate::get_buttons() const
{
    return std::make_tuple(mButton1, mButton2);
}

void clDataElem_ButtonSet_Alternate::set_button1(const clUtil_Button &button1)
{
    Q_ASSERT(! button1.is_ending());
    mButton1 = button1;
}

void clDataElem_ButtonSet_Alternate::set_button2(const clUtil_Button &button2)
{
    Q_ASSERT(! button2.is_ending());
    mButton2 = button2;
}

void clDataElem_ButtonSet_Alternate::set(const clUtil_Button &button1,
                                            const clUtil_Button &button2)
{
    Q_ASSERT(! button1.is_ending());
    Q_ASSERT(! button2.is_ending());
    mButton1 = button1;
    mButton2 = button2;
}

bool clDataElem_ButtonSet_Alternate::parse_and_set(QDomElement &parent, QString &error_msg)
//under `parent`:
//     <button> button 1 settings </button>
//     <button> button 2 settings </button>
{
    if(nsDomUtil::get_count_of_child_elements(parent, "button") != 2)
    {
        error_msg = "Should have exactly two <button>'s.";
        return false;
    }

    QDomElement child = parent.firstChildElement("button");
    bool ch = mButton1.parse_and_set(child);
    if(!ch)
    {
        error_msg = "Could not parse the contents under 1st <button>.";
        return false;
    }

    child = child.nextSiblingElement("button");
    ch = mButton2.parse_and_set(child);
    if(!ch)
    {
        error_msg = "Could not parse the contents under 2nd <button>.";
        return false;
    }

    //
    return true;
}

void clDataElem_ButtonSet_Alternate::add_to_XML(QDomDocument &doc,
                                                   QDomElement &parent) const
{
    QDomElement elem1 = nsDomUtil::add_child_element(doc, parent, "button");
    mButton1.add_to_XML(doc, elem1);

    QDomElement elem2 = nsDomUtil::add_child_element(doc, parent, "button");
    mButton2.add_to_XML(doc, elem2);
}

QString clDataElem_ButtonSet_Alternate::print() const
{
    if(! mPredefined.isEmpty())
        return mPredefined;

    //
    QString S = "custom: alternate\n";
    S += QString("|  button-1 (%1)\n").arg(mButton1.print());
    S += QString("|  button-2 (%1)").arg(mButton2.print());
    return S;
}

clDataElem_ButtonSet_Abstract *clDataElem_ButtonSet_Alternate::clone() const
{
    clDataElem_ButtonSet_Alternate *BS = new clDataElem_ButtonSet_Alternate;
    BS->mButton1 = mButton1;
    BS->mButton2 = mButton2;
    BS->mPredefined = mPredefined;
    return BS;
}

bool clDataElem_ButtonSet_Alternate::operator == (
                                         const clDataElem_ButtonSet_Alternate &another) const
{
    return mButton1 == another.mButton1 && mButton2 == another.mButton2;
}

//////////////////////////////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Abstract *parse_as_button_set(QDomElement &e, QString &error_msg)
//`e` must be one of the following:
//
// + <button-set type="Predefined-Button-Set"/>
//   where "Predefined-Button-Set" can be
//      "close", "check", "half-check", "fail", "violate",
//      "proceed/pause",
//      "close:done,skip", "close:proceed,skip".
//
// + <button-set type="custom: basic">
//      <button> button settings </button>
//   </button-set>
//
// + <button-set type="custom: menu">
//     <base-button> button settings </base-button>
//     <menu-button> button settings </menu-button>
//     <menu-button> button settings </menu-button>
//     ...
//   </button-set>
//
// + <button-set type="custom: alternate">
//     <button> button 1 settings </button>
//     <button> button 2 settings </button>
//   </button-set>
//
//The returned object must be deleted properly.
//Returns 0 iff there's error.
{
    Q_ASSERT(e.tagName() == "button-set");

    //
    QString type_str = e.attribute("type");
    if(type_str.isEmpty())
    {
        error_msg = "Could not find attribute named \"type\" in <button-set>.";
        return 0;
    }

    // predefined type
    clDataElem_ButtonSet_Abstract *ptr = create_predefined_button_set(type_str);
    if(ptr != 0)
        return ptr;

    // custom type
    if(! type_str.startsWith("custom:"))
    {
        error_msg = QString("Unrecognized value \"%1\" of attribute \"type\" in <button-set>.")
                    .arg(type_str);
        return 0;
    }

    clDataElem_ButtonSet_Abstract *BS;
    QString custom_type = type_str.mid(7).simplified();
    if(custom_type == "basic")
        BS = new clDataElem_ButtonSet_Basic;
    else if(custom_type == "menu")
        BS = new clDataElem_ButtonSet_Menu;
    else if(custom_type == "alternate")
        BS = new clDataElem_ButtonSet_Alternate;
    else
    {
        error_msg = QString("Unrecognized value \"%1\" of attribute \"type\" in <button-set>.")
                    .arg(type_str);
        return 0;
    }

    QString error_msg2;
    bool ch = BS->parse_and_set(e, error_msg2);
    if(!ch)
    {
        error_msg = QString("Failed to parse the contents under <button-set> "
                            "as %1 button-set.\n\n%2")
                    .arg(custom_type).arg(error_msg2);
        return 0;
    }
    return BS;
}

void add_button_set_to_XML(QDomDocument &doc, QDomElement &parent,
                           const clDataElem_ButtonSet_Abstract *button_set)
{
    QDomElement elem = nsDomUtil::add_child_element(doc, parent, "button-set");

    //
    QString predefined_name = button_set->get_predefined_name();
    if(! predefined_name.isEmpty()) //(`button_set` is a predefined button-set)
    {
        elem.setAttribute("type", predefined_name);
        return;
    }

    //
    clDataElem_ButtonSet_Abstract::enType type = button_set->get_type();
    if(type == clDataElem_ButtonSet_Abstract::Basic)
        elem.setAttribute("type", "custom: basic");
    else if(type == clDataElem_ButtonSet_Abstract::Menu)
        elem.setAttribute("type", "custom: menu");
    else //(type == clDataElement_ButtonSet_Abstract::Alternate)
        elem.setAttribute("type", "custom: alternate");

    button_set->add_to_XML(doc, elem);
}

clDataElem_ButtonSet_Abstract *create_predefined_button_set(const QString &predefined_name)
//Return `nullptr` if `predefined_name` is not recognized as a name of predefined button set.
{
    if(predefined_name == "close")
        return new clDataElem_ButtonSet_Basic(gButtonClose);
    else if(predefined_name == "check")
        return new clDataElem_ButtonSet_Basic(gButtonCheck);
    else if(predefined_name == "half-check")
        return new clDataElem_ButtonSet_Basic(gButtonHalfCheck);
    else if(predefined_name == "fail")
        return new clDataElem_ButtonSet_Basic(gButtonFail);
    else if(predefined_name == "violate")
        return new clDataElem_ButtonSet_Basic(gButtonViolate);
    else if(predefined_name == "proceed/pause")
        return new clDataElem_ButtonSet_Alternate(gAlternateButtons_ProceedPause);
    else if(predefined_name == "close:done,skip")
        return new clDataElem_ButtonSet_Menu(gClosingMenuButton_DoneSkip);
    else if(predefined_name == "close:proceed,skip")
        return new clDataElem_ButtonSet_Menu(gClosingMenuButton_ProceedSkip);
    else
        return nullptr;
}

bool button_sets_are_equal(const clDataElem_ButtonSet_Abstract *button_set1,
                           const clDataElem_ButtonSet_Abstract *button_set2)
{
    if(button_set1->get_type() != button_set2->get_type())
        return false;

    switch(button_set1->get_type())
    {
    case clDataElem_ButtonSet_Abstract::Basic :
        return *dynamic_cast<const clDataElem_ButtonSet_Basic *>(button_set1)
                == *dynamic_cast<const clDataElem_ButtonSet_Basic *>(button_set2);

    case clDataElem_ButtonSet_Abstract::Alternate :
        return *dynamic_cast<const clDataElem_ButtonSet_Alternate *>(button_set1)
                == *dynamic_cast<const clDataElem_ButtonSet_Alternate *>(button_set2);

    case clDataElem_ButtonSet_Abstract::Menu :
        return *dynamic_cast<const clDataElem_ButtonSet_Menu *>(button_set1)
                == *dynamic_cast<const clDataElem_ButtonSet_Menu *>(button_set2);

    default:
        Q_ASSERT(false);
        return false;
    }
}
