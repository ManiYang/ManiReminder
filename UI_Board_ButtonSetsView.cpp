#include <QDebug>
#include "UI_Board_ButtonSetsView.h"
typedef clDataElem_ButtonSet_Abstract clButtonSet;

clUI_Board_ButtonSetsView::clUI_Board_ButtonSetsView(QHBoxLayout *layout)
    : QObject(layout->parentWidget()), mParentWidget(layout->parentWidget()),
      mLayout(layout)
{
    Q_ASSERT(layout->parentWidget() != 0);

    mButtonGroup = new QButtonGroup(this);
    mButtonGroup->setExclusive(false);
    connect(mButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(On_button_clicked(QAbstractButton*)));

    //
    mLayout->addStretch();
}

void clUI_Board_ButtonSetsView::clear()
{
    // remove items from `mButtonGroup` and from `mLayout`
    QLayoutItem *item;
    while((item = mLayout->takeAt(0)) != 0)
    {
        if(item->widget() != nullptr) //(`item` is a push-button)
        {
            QPushButton *pb = dynamic_cast<QPushButton *>(item->widget());
            mButtonGroup->removeButton(pb);
            pb->deleteLater();
        }
        else
            delete item;
    }

    mLayout->addStretch();

    // remove all menus
    for(auto it=mMenus.begin(); it!=mMenus.end(); it++)
        (*it)->deleteLater();
    mMenus.clear();

    //
    mAlternateButtons.clear();
    mAlternateButton_StateRegister.clear();
    mEndingActions.clear();
    mEndingButtons.clear();
}

static void set_push_button(QPushButton *push_button, const clUtil_Button &button_spec)
//set the text, icon, and tool-tip of `push_button` according to `button_spec`
{
    push_button->setText(button_spec.get_text());
    push_button->setIcon(QIcon(button_spec.get_icon_file()));
    push_button->setToolTip(button_spec.get_log_text());
}

void clUI_Board_ButtonSetsView::add_button(const clDataElem_ButtonSet_Abstract *button_set,
                                           int *p_state)
{
    // build push-button --> `push_button`
    QPushButton *push_button = new QPushButton(mParentWidget);
    QPushButton *push_button_alternate = nullptr;

    clButtonSet::enType BS_type = button_set->get_type();
    if(BS_type == clButtonSet::Basic)
    {
        clUtil_Button button_spec
                = dynamic_cast<const clDataElem_ButtonSet_Basic*>(button_set)
                  ->get_button();

        set_push_button(push_button, button_spec);
        if(button_spec.is_ending())
            mEndingButtons.insert(push_button);
    }
    else if(BS_type == clButtonSet::Menu)
    {
        const clDataElem_ButtonSet_Menu *menu_BS
                = dynamic_cast<const clDataElem_ButtonSet_Menu*>(button_set);
        clUtil_Button button_spec = menu_BS->get_base_button();

        set_push_button(push_button, button_spec);

        // menu
        QMenu *menu = new QMenu(mParentWidget);
        mMenus << menu;
        push_button->setMenu(menu);

        QList<clUtil_Button> menu_button_specs = menu_BS->get_menu_buttons();
        foreach(clUtil_Button button_spec, menu_button_specs)
        {
            QAction *action = menu->addAction(button_spec.get_text());

            action->setIcon(QIcon(button_spec.get_icon_file()));
            action->setToolTip(button_spec.get_log_text());

            if(button_spec.is_ending())
            {
                mEndingActions.insert(action);
            }
        }

        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(On_action_triggered(QAction*)));
    }
    else if(BS_type == clButtonSet::Alternate)
    {
        clUtil_Button button_spec1, button_spec2;
        std::tie(button_spec1, button_spec2)
                = dynamic_cast<const clDataElem_ButtonSet_Alternate*>(button_set)
                  ->get_buttons();

        set_push_button(push_button, button_spec1);

        //
        push_button_alternate = new QPushButton(mParentWidget);
        set_push_button(push_button_alternate, button_spec2);
        push_button_alternate->setVisible(false);

        //
        mAlternateButtons.insert(push_button, push_button_alternate);
        mAlternateButtons.insert(push_button_alternate, push_button);

        Q_ASSERT(p_state != nullptr);
        mAlternateButton_StateRegister.insert(push_button, p_state);
        mAlternateButton_StateRegister.insert(push_button_alternate, p_state);
    }

    // add push-button to layout
    mLayout->addWidget(push_button);
    mButtonGroup->addButton(push_button);

    if(BS_type == clButtonSet::Alternate)
    {
        Q_ASSERT(push_button_alternate != nullptr);
        mLayout->addWidget(push_button_alternate);
        mButtonGroup->addButton(push_button_alternate);

        if(*p_state)
        {
            push_button->setVisible(false);
            push_button_alternate->setVisible(true);
        }
    }
}

void clUI_Board_ButtonSetsView::On_button_clicked(QAbstractButton *button)
{
    emit user_clicked_a_button();

    //
    bool ending = mEndingButtons.contains(button);

    //
    if(mAlternateButtons.contains(button))
    {
        //button->setVisible(false);
        //mAlternateButtons[button]->setVisible(true);
        *mAlternateButton_StateRegister[button] = !(*mAlternateButton_StateRegister[button]);
    }

    //
    QString log_text = button->toolTip();
    if(! log_text.isEmpty())
        emit to_add_record(log_text); //will update `*this`

    //
    if(ending)
        emit user_clicked_ending_button();
}

void clUI_Board_ButtonSetsView::On_action_triggered(QAction *action)
{
    bool ending = mEndingActions.contains(action);

    QString log_text = action->toolTip();
    if(!log_text.isEmpty())
        emit to_add_record(log_text); //will update `*this`

    //
    if(ending)
        emit user_clicked_ending_button();
}
