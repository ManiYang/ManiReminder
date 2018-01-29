#ifndef UI_ALL_REMINDERS_DIALOG_EDIT_SPEC_H
#define UI_ALL_REMINDERS_DIALOG_EDIT_SPEC_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QTimer>
#include "DataElem_ButtonSet.h"
#include "ReminderSpec.h"
#include "UI_Dialog_SpecInputHelper.h"

namespace Ui {
class clUI_AllReminders_Dialog_EditSpec;
}

class clUI_AllReminders_Dialog_EditSpec : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_AllReminders_Dialog_EditSpec(
                                    const QString &reminder_title,
                                    const clReminderSpec *reminder_spec,
                                    const QList<clDataElem_ButtonSet_Abstract*> &button_sets,
                                    const QSet<QString> *p_Situations,
                                    const QSet<QString> *p_Events,
                                    QWidget *parent = 0);
    ~clUI_AllReminders_Dialog_EditSpec();

    void get(clReminderSpec *spec, QList<clDataElem_ButtonSet_Abstract*> *button_sets);
             //`(*button_sets)[i]` should be deleted properly (after that, do not use the
             //object of this dialog anymore)

public slots:
    void accept();

private slots:
    void on_pushButton_insert_clicked();
    void on_pushButton_show_helper_clicked();
    void on_pushButton_preview_clicked();
    void On_buttons_table_cell_data_changed(int row, int col);
    void build_buttons_table(); //according to `mButtonSets`


private:
    Ui::clUI_AllReminders_Dialog_EditSpec *ui;

    // data (read only) //
    const QSet<QString> *pSituations;
    const QSet<QString> *pEvents;

    //
    bool mAccepted;
    clReminderSpec mSpec;
    QList<clDataElem_ButtonSet_Abstract*> mButtonSets; //applied (checked) button sets
    QMap<QTableWidgetItem*,int> mButtonsTableItem_to_Index; //value: index of `mButtonSets[]`

    clUI_Dialog_SpecInputHelper *mHelperDialog;

    //
    bool parse_spec(); //parse the spec setting and set `mSpec`
    QString print_spec_XML(); //print `mSpec` in XML

    void add_row_to_buttons_table(const int applied_button_set_index, const QString &text);
         //If `text` is for an applied button set, `applied_button_set_index` should be the
         //corresponding index of `mButtonSets[]`. Otherwise, `applied_button_set_index` should
         //be -1.

    bool has_ending_button() const; //according to `mButtonSets`
};

#endif // UI_ALL_REMINDERS_DIALOG_EDIT_SPEC_H
