#ifndef DATA_ELEMENT_BUTTON_SET_H
#define DATA_ELEMENT_BUTTON_SET_H

#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include "Util_Button.h"

class clDataElem_ButtonSet_Abstract
{
public:
    explicit clDataElem_ButtonSet_Abstract(const QString &predefined_name);
    virtual ~clDataElem_ButtonSet_Abstract() {}

    enum enType {Basic, Menu, Alternate};
    virtual enType get_type() const = 0;
    virtual bool has_ending_button() const = 0;
    virtual bool parse_and_set(QDomElement &parent, QString &error_msg) = 0;
    virtual void add_to_XML(QDomDocument &doc, QDomElement &parent) const = 0;
    virtual QString print() const = 0;
    virtual clDataElem_ButtonSet_Abstract *clone() const = 0; //should create new object

    bool is_predefined() const { return !mPredefined.isEmpty(); }
    QString get_predefined_name() const { return mPredefined; }
            //If `*this` is a predefined button-set, return the name of the button-set.
            //Otherwise, return "".

protected:
    QString mPredefined; //If `*this` is a predefined button-set, `mPredefined` should be
                         //the name of the button-set. Otherwise, `mPredefined` should be
                         //empty.
};


/// --------------- basic button set ----------------------

class clDataElem_ButtonSet_Basic : public clDataElem_ButtonSet_Abstract
{
public:
    clDataElem_ButtonSet_Basic() : clDataElem_ButtonSet_Abstract("") {}
    explicit clDataElem_ButtonSet_Basic(const clUtil_Button &button,
                                           const QString &predefined_name = "");
    clDataElem_ButtonSet_Basic(const clDataElem_ButtonSet_Basic &another);
    virtual ~clDataElem_ButtonSet_Basic() {}

    virtual enType get_type() const { return Basic; }
    virtual bool has_ending_button() const { return mButton.is_ending(); }
    clUtil_Button get_button() const { return mButton; }
    void set_button(const clUtil_Button &button);

    virtual bool parse_and_set(QDomElement &parent, QString &error_msg);
                               //under `parent`:
                               //  <button> button settings </button>
    virtual void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    virtual QString print() const;

    virtual clDataElem_ButtonSet_Abstract *clone() const; //creates new object

    bool operator == (const clDataElem_ButtonSet_Basic &another) const;

private:
    clUtil_Button mButton;
};


/// --------------- menu button set ----------------------

class clDataElem_ButtonSet_Menu : public clDataElem_ButtonSet_Abstract
{
public:
    clDataElem_ButtonSet_Menu() : clDataElem_ButtonSet_Abstract("") {}
    clDataElem_ButtonSet_Menu(const clUtil_Button &base_button,
                                 const QList<clUtil_Button> &menu_buttons,
                                 const QString &predefined_name = "");
    clDataElem_ButtonSet_Menu(const clDataElem_ButtonSet_Menu &another);
    virtual ~clDataElem_ButtonSet_Menu() {}

    virtual enType get_type() const { return Menu; }
    virtual bool has_ending_button() const;

    clUtil_Button get_base_button() const { return mBaseButton; }
    QList<clUtil_Button> get_menu_buttons() const { return mMenuButtons; }

    void set_base_button(const clUtil_Button &base_button);
    void set_menu_buttons(const QList<clUtil_Button> &menu_buttons);
    void set(const clUtil_Button &base_button, const QList<clUtil_Button> &menu_buttons);

    virtual bool parse_and_set(QDomElement &parent, QString &error_msg);
                               //under `parent`:
                               //   <base-button> button settings </base-button>
                               //   <button> button settings </button>
                               //   <button> button settings </button>
                               //   ...
    virtual void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    virtual QString print() const;

    virtual clDataElem_ButtonSet_Abstract *clone() const; //creates new object

    bool operator == (const clDataElem_ButtonSet_Menu &another) const;

private:
    clUtil_Button mBaseButton; //cannot be ending
    QList<clUtil_Button> mMenuButtons;
};


/// --------------- alternate button set ----------------------

class clDataElem_ButtonSet_Alternate : public clDataElem_ButtonSet_Abstract
{
public:
    clDataElem_ButtonSet_Alternate() : clDataElem_ButtonSet_Abstract("") {}
    clDataElem_ButtonSet_Alternate(const clUtil_Button &button1,
                                      const clUtil_Button &button2,
                                      const QString &predefined_name = "");
    clDataElem_ButtonSet_Alternate(const clDataElem_ButtonSet_Alternate &another);
    virtual ~clDataElem_ButtonSet_Alternate() {}

    virtual enType get_type() const { return Alternate; }
    virtual bool has_ending_button() const { return false; }

    std::tuple<clUtil_Button,clUtil_Button> get_buttons() const;
    clUtil_Button get_button1() const { return mButton1; }
    clUtil_Button get_button2() const { return mButton2; }

    void set_button1(const clUtil_Button &button1);
    void set_button2(const clUtil_Button &button2);
    void set(const clUtil_Button &button1, const clUtil_Button &button2);

    virtual bool parse_and_set(QDomElement &parent, QString &error_msg);
                               //under `parent`:
                               //     <button> button 1 settings </button>
                               //     <button> button 2 settings </button>
    virtual void add_to_XML(QDomDocument &doc, QDomElement &parent) const;
    virtual QString print() const;

    virtual clDataElem_ButtonSet_Abstract *clone() const; //creates new object

    bool operator == (const clDataElem_ButtonSet_Alternate &another) const;

private:
    clUtil_Button mButton1; //cannot be ending
    clUtil_Button mButton2; //cannot be ending
      // `mButton1` appears first.
};


////////////////////////////////////////////////////////////////

clDataElem_ButtonSet_Abstract *parse_as_button_set(QDomElement &e, QString &error_msg);
//`e` must be one of the following:
//
// + <button-set type="$Predefined-Button-Set"/>
//   where "$Predefined-Button-Set" can be
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
//See "util_button.h" for "button setting".
//
//The returned object must be deleted properly.
//Returns 0 iff there's error.

void add_button_set_to_XML(QDomDocument &doc, QDomElement &parent,
                           const clDataElem_ButtonSet_Abstract *button_set);

clDataElem_ButtonSet_Abstract *create_predefined_button_set(const QString &predefined_name);
//Return `nullptr` if `predefined_name` is not recognized as a name of predefined button set.

bool button_sets_are_equal(const clDataElem_ButtonSet_Abstract *button_set1,
                           const clDataElem_ButtonSet_Abstract *button_set2);

#endif // DATA_ELEMENT_BUTTON_SET_H
