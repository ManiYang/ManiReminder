#ifndef UI_ALL_REMINDERS_DIALOG_EDIT_TAGS_H
#define UI_ALL_REMINDERS_DIALOG_EDIT_TAGS_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class clUI_AllReminders_Dialog_EditTags;
}

class clUI_AllReminders_Dialog_EditTags : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_AllReminders_Dialog_EditTags(const QStringList &tags, QWidget *parent = 0);
    ~clUI_AllReminders_Dialog_EditTags();

    QStringList get_tags() const;

public slots:
    virtual void accept();

private slots:
    void On_item_modified(QListWidgetItem *item);

private:
    Ui::clUI_AllReminders_Dialog_EditTags *ui;

    void add_row_for_new_tag();
};

#endif // UI_ALL_REMINDERS_DIALOG_EDIT_TAGS_H
