#ifndef UI_ALL_REMINDERS_DIALOG_ADD_CUSTOM_BUTTON_SET_H
#define UI_ALL_REMINDERS_DIALOG_ADD_CUSTOM_BUTTON_SET_H

#include <QDialog>
#include <QVBoxLayout>
#include "DataElem_ButtonSet.h"

namespace Ui {
class clUI_AllReminders_Dialog_AddCustomButtonSet;
}

class clUI_AllReminders_Dialog_AddCustomButtonSet : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_AllReminders_Dialog_AddCustomButtonSet(
                                           const clDataElem_ButtonSet_Abstract::enType type,
                                           QWidget *parent = 0);
    ~clUI_AllReminders_Dialog_AddCustomButtonSet();

    clDataElem_ButtonSet_Abstract *get_button_set();

public slots:
    void accept();

private slots:
    void on_pushButton_add_menu_button_clicked();
    void on_pushButton_remove_menu_button_clicked();
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn,
                                           int previousRow, int previousColumn);


private:
    Ui::clUI_AllReminders_Dialog_AddCustomButtonSet *ui;

    clDataElem_ButtonSet_Abstract::enType mType;
    bool mAccepted;

    void add_button_setting_form(const QString &label, const bool no_ending);
    clUtil_Button get_button_from_form(const int row);
};

#endif // UI_ALL_REMINDERS_DIALOG_ADD_CUSTOM_BUTTON_SET_H
