#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include "UI_AllReminders_Dialog_AddCustomButtonSet.h"
#include "ui_UI_AllReminders_Dialog_AddCustomButtonSet.h"

clUI_AllReminders_Dialog_AddCustomButtonSet::clUI_AllReminders_Dialog_AddCustomButtonSet(
                                           const clDataElem_ButtonSet_Abstract::enType type,
                                           QWidget *parent)
    : QDialog(parent),
      ui(new Ui::clUI_AllReminders_Dialog_AddCustomButtonSet),
      mType(type), mAccepted(false)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    //
    switch(type)
    {
    case clDataElem_ButtonSet_Abstract::Basic :
        ui->label_type->setText("Basic Button:");
        add_button_setting_form("button", false);
        ui->pushButton_add_menu_button->setVisible(false);
        ui->pushButton_remove_menu_button->setVisible(false);
        break;

    case clDataElem_ButtonSet_Abstract::Alternate :
        ui->label_type->setText("Alternate Button Set:");
        add_button_setting_form("button 1", true);
        add_button_setting_form("button 2", true);
        ui->pushButton_add_menu_button->setVisible(false);
        ui->pushButton_remove_menu_button->setVisible(false);
        break;

    case clDataElem_ButtonSet_Abstract::Menu :
        ui->label_type->setText("Menu Button Set:");
        add_button_setting_form("base button", true);
        add_button_setting_form("sub-button", false);
        ui->pushButton_remove_menu_button->setEnabled(false);
        break;
    }
}

clUI_AllReminders_Dialog_AddCustomButtonSet::~clUI_AllReminders_Dialog_AddCustomButtonSet()
{
    delete ui;
}

void clUI_AllReminders_Dialog_AddCustomButtonSet::add_button_setting_form(
                                                    const QString &label, const bool no_ending)
{
    int Nrows = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(++Nrows);

    //
    QTableWidgetItem *item;

    item = new QTableWidgetItem(label);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    ui->tableWidget->setItem(Nrows-1, 0, item);

    item = new QTableWidgetItem("");
    ui->tableWidget->setItem(Nrows-1, 1, item);

    item = new QTableWidgetItem("");
    ui->tableWidget->setItem(Nrows-1, 2, item);
    QComboBox *combobox = new QComboBox;
    combobox->addItems(QStringList() << "no icon" << "check.png" << "cross.png"
                                     <<"half-check.png" << "pause.png" << "proceed.png");
    ui->tableWidget->setCellWidget(Nrows-1, 2, combobox);

    item = new QTableWidgetItem("");
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    if(no_ending)
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->tableWidget->setItem(Nrows-1, 3, item);

    item = new QTableWidgetItem("");
    ui->tableWidget->setItem(Nrows-1, 4, item);
}

void clUI_AllReminders_Dialog_AddCustomButtonSet::on_pushButton_add_menu_button_clicked()
{
    add_button_setting_form("sub-button", false);
}

void clUI_AllReminders_Dialog_AddCustomButtonSet::on_pushButton_remove_menu_button_clicked()
{
    int row = ui->tableWidget->currentRow();
    if(row >= 1)
        ui->tableWidget->removeRow(row);

    if(ui->tableWidget->rowCount() == 1)
        add_button_setting_form("sub-button", false);
}

void clUI_AllReminders_Dialog_AddCustomButtonSet::on_tableWidget_currentCellChanged(
                                                           int currentRow, int currentColumn,
                                                           int previousRow, int previousColumn)
{
    Q_UNUSED(previousColumn);
    Q_UNUSED(previousRow);

    if(currentRow < 1 || currentColumn < 0)
        ui->pushButton_remove_menu_button->setEnabled(false);
    else
        ui->pushButton_remove_menu_button->setEnabled(true);
}

void clUI_AllReminders_Dialog_AddCustomButtonSet::accept()
{
    for(int r=0; r<ui->tableWidget->rowCount(); r++)
    {
        QString text = ui->tableWidget->item(r, 1)->text();
        if(text.isEmpty())
        {
            QWidget *w = ui->tableWidget->cellWidget(r, 2);
            QString icon = dynamic_cast<QComboBox*>(w)->currentText();
            if(icon == "no icon")
            {
                QMessageBox::warning(this, "warning",
                                     "A button cannot have no text and no icon.");
                return;
            }
        }
    }

    mAccepted = true;
    QDialog::accept();
}

clDataElem_ButtonSet_Abstract *clUI_AllReminders_Dialog_AddCustomButtonSet::get_button_set()
{
    Q_ASSERT(mAccepted); //must have been accepted before this function is called

    //
    clDataElem_ButtonSet_Abstract *BS = 0;

    switch(mType)
    {
    case clDataElem_ButtonSet_Abstract::Basic :
        BS = new clDataElem_ButtonSet_Basic(get_button_from_form(0));
        break;

    case clDataElem_ButtonSet_Abstract::Alternate :
        BS = new clDataElem_ButtonSet_Alternate(get_button_from_form(0),
                                                   get_button_from_form(1));
        break;

    case clDataElem_ButtonSet_Abstract::Menu :
    {
        clUtil_Button base_button = get_button_from_form(0);
        QList<clUtil_Button> menu_buttons;
        for(int r=1; r<ui->tableWidget->rowCount(); r++)
            menu_buttons << get_button_from_form(r);
        BS = new clDataElem_ButtonSet_Menu(base_button, menu_buttons);
        break;
    }
    }

    //
    return BS;
}

clUtil_Button clUI_AllReminders_Dialog_AddCustomButtonSet::get_button_from_form(const int row)
{
    Q_ASSERT(row < ui->tableWidget->rowCount());

    //
    QString text = ui->tableWidget->item(row, 1)->text().simplified();

    QString icon = dynamic_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2))->currentText();
    if(icon == "no icon")
        icon = "";

    bool ending = (ui->tableWidget->item(row, 3)->checkState() == Qt::Checked);

    QString log_text = ui->tableWidget->item(row, 4)->text().simplified();

    //
    return clUtil_Button(text, icon, ending, log_text);
}
