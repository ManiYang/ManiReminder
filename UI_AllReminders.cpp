#include <QListWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include "UI_AllReminders.h"
#include "ui_UI_AllReminders.h"
#include "utility_general.h"
#include "utility_widgets.h"
#include "UI_AllReminders_Dialog_EditTags.h"
#include "UI_AllReminders_Dialog_EditSpec.h"

clUI_AllReminders::clUI_AllReminders(const QMap<int, const clReminder *> *p_reminders,
                                     const QSet<QString> *p_situations,
                                     const QSet<QString> *p_events,
                                     QWidget *parent)
    : QWidget(parent),
      ui(new Ui::clUI_AllReminders),
      pReminders(p_reminders), pSituations(p_situations), pEvents(p_events)
{
    ui->setupUi(this);

    //
    ui->listWidget_reminders->setContextMenuPolicy(Qt::CustomContextMenu);

    mContextMenu_RemTitles = new QMenu(this);
    mAction_EditRemTitle = mContextMenu_RemTitles->addAction("Edit Reminder Title");

    //
    ui->label_tags->setText("");
    ui->label_tags->setStyleSheet("Font: 10pt");

    //
    ui->textEdit_detail->setReadOnly(true);
    ui->textEdit_detail->setStyleSheet("Font: 11pt");
    ui->textEdit_detail->installEventFilter(this);

    //
    ui->tableWidget_records->setStyleSheet("Font: 10pt");
    ui->tableWidget_records->horizontalHeader()->setVisible(false);
    ui->tableWidget_records->verticalHeader()->setVisible(false);
    ui->tableWidget_records->horizontalHeader()
                                       ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_records->verticalHeader()
                                       ->setSectionResizeMode(QHeaderView::ResizeToContents);

    //
    ui->plainTextEdit_quick_note->setReadOnly(true);
    ui->plainTextEdit_quick_note->setStyleSheet("Font: 10pt");
    ui->plainTextEdit_quick_note->installEventFilter(this);

    //
    ui->label_spec->setText("");
    ui->label_alarm->setText("");
    ui->label_button_sets->clear();
}

clUI_AllReminders::~clUI_AllReminders()
{
    delete ui;
}

void clUI_AllReminders::update_all_reminders_view()
{
    ui->listWidget_reminders->clear();
    clear_detail_view_contents();

    //
    for(auto it=pReminders->cbegin(); it!=pReminders->cend(); it++)
    {
        const int id = it.key();
        const clReminder *reminder = it.value();

        // add to `ui->listWidget_reminders`
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::DisplayRole, reminder->get_title());
        item->setData(Qt::UserRole, id);
        item->setData(Qt::ToolTipRole, QString::number(id));

        ui->listWidget_reminders->addItem(item);
        mReminderID_to_Item.insert(id, item);
    }

    // update detail view
    QListWidgetItem *cur_item = ui->listWidget_reminders->currentItem();
    if(cur_item != Q_NULLPTR)
    {
        int id = cur_item->data(Qt::UserRole).toInt();
        const clReminder *R = (*pReminders)[id];
        set_detail_view_contents(R);
    }
}

void clUI_AllReminders::update_reminder_view(const int id)
{
    Q_ASSERT(pReminders->contains(id));
    const clReminder *reminder = (*pReminders)[id];

    // update title
    Q_ASSERT(mReminderID_to_Item.contains(id));
    mReminderID_to_Item[id]->setData(Qt::DisplayRole, reminder->get_title());

    // update detail view if selected
    if(mReminderID_to_Item[id]->isSelected())
        set_detail_view_contents(reminder);
}

void clUI_AllReminders::add_reminder(const int id)
//and select it
{
    Q_ASSERT(pReminders->contains(id));

    // add to `ui->listWidget_reminders`
    QString title = (*pReminders)[id]->get_title();

    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::DisplayRole, title);
    item->setData(Qt::UserRole, id);
    item->setData(Qt::ToolTipRole, QString::number(id));

    ui->listWidget_reminders->addItem(item);
    mReminderID_to_Item.insert(id, item);

    //
    ui->listWidget_reminders->setCurrentItem(item);
    set_detail_view_contents((*pReminders)[id]);
}

void clUI_AllReminders::on_listWidget_reminders_currentItemChanged(QListWidgetItem *current,
                                                                   QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if(current == Q_NULLPTR)
    {
        clear_detail_view_contents();
        return;
    }

    //
    int rem_id = current->data(Qt::UserRole).toInt();
    Q_ASSERT(pReminders->contains(rem_id));
    const clReminder *reminder = (*pReminders)[rem_id];

    set_detail_view_contents(reminder);
}

void clUI_AllReminders::clear_detail_view_contents()
{
    ui->label_tags->clear();
    ui->textEdit_detail->clear();
    ui->tableWidget_records->setRowCount(0);
    ui->plainTextEdit_quick_note->setPlainText("");
    ui->label_spec->setText("");
    ui->label_alarm->setText("");
    ui->label_button_sets->clear();
    //...
}

void clUI_AllReminders::set_detail_view_contents(const clReminder *reminder)
{
    QString tags = reminder->get_tags().join(", ");
    ui->label_tags->setText(tags);

    QString detail = reminder->get_detail();
    ui->textEdit_detail->setText(detail);

    QMultiMap<QDateTime, QString> records = reminder->get_records();
    set_records_table(records);

    QString quick_note = reminder->get_quick_note();
    ui->plainTextEdit_quick_note->setPlainText(quick_note);

    QString spec_str = reminder->get_spec_print();
    ui->label_spec->setText(spec_str);

    QString alarm_str = reminder->get_alarm().print();
    ui->label_alarm->setText(alarm_str);

    QString button_sets_str = reminder->get_button_sets_print();
    ui->label_button_sets->setText(button_sets_str);

    //...
}

void clUI_AllReminders::set_records_table(const QMultiMap<QDateTime, QString> &records)
{
    QStringList log_texts, times;
    QMapIterator<QDateTime, QString> it(records);
    for(it.toBack(); it.hasPrevious(); )
    {
        it.previous();
        log_texts << it.value();
        times << it.key().toString("yyyy/M/d hh:mm:ss");
    }

    fill_table_widget(*ui->tableWidget_records,
                      QList<QStringList>() << times << log_texts,
                      QStringList() << "time" << "log text");
}

void clUI_AllReminders::set_current_reminder(const int id)
{
    Q_ASSERT(mReminderID_to_Item.contains(id));
    ui->listWidget_reminders->setCurrentItem(mReminderID_to_Item[id]);
}

int clUI_AllReminders::get_current_reminder_id() const
//Return -1 if no reminder is selected.
{
    QListWidgetItem *item = ui->listWidget_reminders->currentItem();
    if(item == Q_NULLPTR)
        return -1;

    bool ok;
    int id = item->data(Qt::UserRole).toInt(&ok);
    Q_ASSERT(ok);
    return id;
}

void clUI_AllReminders::on_listWidget_reminders_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidget_reminders->itemAt(pos);
    if(item == nullptr)
        return;

    int rem_id = item->data(Qt::UserRole).toInt();

    //
    QAction *selected_action = mContextMenu_RemTitles->exec(ui->listWidget_reminders
                                                              ->viewport()->mapToGlobal(pos));
    if(selected_action == mAction_EditRemTitle)
    {
        QString orig_title = item->data(Qt::DisplayRole).toString();

        QInputDialog dialog;
        dialog.setLabelText("Reminder title:");
        dialog.setInputMode(QInputDialog::TextInput);
        dialog.setTextValue(orig_title);

        dialog.resize(300,100);
        int r = dialog.exec();
        if(r == QDialog::Accepted)
        {
            QString new_title = dialog.textValue().simplified();
            if(new_title.isEmpty())
            {
                QMessageBox::warning(this, "warning", "Reminder title cannot be empty.");
                return;
            }

            clReminder new_data(-1);
            new_data.set_title(new_title);
            emit to_modify_reminder(rem_id, clReminder::Title, &new_data);
        }
    }
}

void clUI_AllReminders::on_pushButton_edit_tags_clicked()
{
    const int id = get_current_reminder_id();
    if(id == -1)
        return;

    clUI_AllReminders_Dialog_EditTags dialog((*pReminders)[id]->get_tags());
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        QStringList new_tags = dialog.get_tags();
        clReminder new_data(-1);
        new_data.set_tags(new_tags);
        emit to_modify_reminder(id, clReminder::Tags, &new_data);
    }
}

void clUI_AllReminders::on_pushButton_edit_detail_clicked()
{
    const int id = get_current_reminder_id();
    if(id == -1)
        return;

    ui->pushButton_edit_detail->setEnabled(false);

    QString detail = (*pReminders)[id]->get_detail();
    ui->textEdit_detail->setPlainText(detail);
    ui->textEdit_detail->setReadOnly(false);
    ui->textEdit_detail->setFocus();
}

bool clUI_AllReminders::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->textEdit_detail)
    {
        if(event->type() == QEvent::FocusOut)
        {
            if(! ui->textEdit_detail->isReadOnly()) //(finish editing)
                finish_editing_detail();
        }
    }
    else if(watched == ui->plainTextEdit_quick_note)
    {
        if(event->type() == QEvent::FocusOut)
        {
            if(! ui->plainTextEdit_quick_note->isReadOnly())
                finish_editing_quickNote();
        }
    }

    //
    return QWidget::eventFilter(watched, event);
}

void clUI_AllReminders::finish_editing_detail()
{
    const int id = get_current_reminder_id();
    if(id != -1)
    {
        QString new_detail = ui->textEdit_detail->toPlainText();
        clReminder new_data(-1);
        new_data.set_detail(new_detail);
        emit to_modify_reminder(id, clReminder::Detail, &new_data);
    }

    //
    ui->textEdit_detail->setReadOnly(true);
    ui->pushButton_edit_detail->setEnabled(true);
}

void clUI_AllReminders::on_pushButton_edit_quickNote_clicked()
{
    const int id = get_current_reminder_id();
    if(id == -1)
        return;

    ui->pushButton_edit_quickNote->setEnabled(false);
    ui->plainTextEdit_quick_note->setReadOnly(false);
    ui->plainTextEdit_quick_note->setFocus();
}

void clUI_AllReminders::finish_editing_quickNote()
{
    const int id = get_current_reminder_id();
    if(id != -1)
    {
        QString new_quick_note = ui->plainTextEdit_quick_note->toPlainText();
        clReminder new_data(-1);
        new_data.set_quick_note(new_quick_note);
        emit to_modify_reminder(id, clReminder::QuickNote, &new_data);
    }

    //
    ui->plainTextEdit_quick_note->setReadOnly(true);
    ui->pushButton_edit_quickNote->setEnabled(true);
}

void clUI_AllReminders::on_pushButton_edit_spec_alarm_buttons_clicked()
{
    const int id = get_current_reminder_id();
    if(id == -1)
        return;

    const clReminder *reminder = (*pReminders)[id];

    //
    clUI_AllReminders_Dialog_EditSpec dialog(reminder->get_title(), reminder->get_spec(),
                                             reminder->get_button_sets(),
                                             pSituations, pEvents);
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        clReminderSpec new_spec;
        QList<clDataElem_ButtonSet_Abstract*> new_button_sets;
        dialog.get(&new_spec, &new_button_sets); //`button_sets[i]` should be deleted properly

        {
            // button sets
            clReminder new_data(-1);
            for(int i=0; i<new_button_sets.size(); i++)
            {
                new_data.add_button_set(new_button_sets[i]);
                delete new_button_sets[i];
            }
            emit to_modify_reminder(id, clReminder::Buttons, &new_data);
        }
        {
            // spec
            emit to_modify_reminder_spec(id, &new_spec);
        }
    }
}
