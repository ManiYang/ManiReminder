#ifndef UI_BOARD_REMINDER_BUTTONS_H
#define UI_BOARD_REMINDER_BUTTONS_H

#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMenu>
#include "DataElem_ButtonSet.h"

class clUI_Board_ButtonSetsView : public QObject
{
    Q_OBJECT
public:
    explicit clUI_Board_ButtonSetsView(QHBoxLayout *layout);
                          //Buttons will be added in `layout`, which must have a parent widget.

    void clear();
    void add_button(const clDataElem_ButtonSet_Abstract *button_set, int *p_state = nullptr);

signals:
    void user_clicked_a_button();
    void user_clicked_ending_button();
    void to_add_record(QString log_text);

//-----------------

private slots:
    void On_button_clicked(QAbstractButton *button);
    void On_action_triggered(QAction *action);

private:
    QWidget *mParentWidget;

    QHBoxLayout *mLayout; //buttons will be added in this layout
    QButtonGroup *mButtonGroup;

    QList<QMenu*> mMenus;
    QMap<QAbstractButton*, QAbstractButton*> mAlternateButtons;
    QMap<QAbstractButton*, int*> mAlternateButton_StateRegister;
                                 //write to *mAlternateButton_StateRegister[button] when the
                                 //state  of the alternate button `button` changed
    QSet<QAbstractButton*> mEndingButtons;
    QSet<QAction*> mEndingActions;
};

#endif // UI_BOARD_REMINDER_BUTTONS_H
