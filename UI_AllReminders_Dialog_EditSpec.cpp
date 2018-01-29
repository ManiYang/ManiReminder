#include <QDebug>
#include <QTimer>
#include <QDesktopWidget>
#include <QMessageBox>
#include "UI_AllReminders_Dialog_EditSpec.h"
#include "ui_UI_AllReminders_Dialog_EditSpec.h"
#include "UI_AllReminders_Dialog_AddCustomButtonSet.h"
#include "UI_Dialog_SpecElementInsert.h"

extern const QStringList G_PredefinedButtonSets;

const QString gMonospaceFont("monospace");

clUI_AllReminders_Dialog_EditSpec::clUI_AllReminders_Dialog_EditSpec(
                                  const QString &reminder_title,
                                  const clReminderSpec *reminder_spec,
                                  const QList<clDataElem_ButtonSet_Abstract*> &button_sets,
                                  const QSet<QString> *p_Situations,
                                  const QSet<QString> *p_Events,
                                  QWidget *parent)
    : QDialog(parent), ui(new Ui::clUI_AllReminders_Dialog_EditSpec),
      pSituations(p_Situations), pEvents(p_Events),
      mAccepted(false)
{
    ui->setupUi(this);

    //
    resize(QApplication::desktop()->availableGeometry(this).size() * 0.68);
    setWindowTitle("Editing Spec/Alarm/Buttons");

    ui->label_reminder_title->setText(reminder_title);

    // copy `*reminder_spec`
    mSpec = *reminder_spec;

    //
    QString spec_xml = print_spec_XML();
    ui->plainTextEdit_spec->setPlainText(spec_xml);

    ui->plainTextEdit_spec->setStyleSheet(QString("font: 10pt \"%1\"").arg(gMonospaceFont));

    // copy `*button_sets[i]`
    for(auto it=button_sets.constBegin(); it!=button_sets.constEnd(); it++)
        mButtonSets << (*it)->clone();

    //
    ui->tableWidget_buttons->setColumnCount(2);
    ui->tableWidget_buttons->setRowCount(0);
    ui->tableWidget_buttons->horizontalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_buttons->verticalHeader()
                                         ->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_buttons->horizontalHeader()->setVisible(false);
    ui->tableWidget_buttons->verticalHeader()->setVisible(false);
    ui->tableWidget_buttons->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tableWidget_buttons->setStyleSheet("QTableWidget {font: 10pt}");

    build_buttons_table();

    //
    mHelperDialog = new clUI_Dialog_SpecInputHelper(this);
}

clUI_AllReminders_Dialog_EditSpec::~clUI_AllReminders_Dialog_EditSpec()
{
    delete ui;
}

void clUI_AllReminders_Dialog_EditSpec::get(
                                        clReminderSpec *spec,
                                        QList<clDataElem_ButtonSet_Abstract *> *button_sets)
//`(*button_sets)[i]` should be deleted properly.
{
    Q_ASSERT(mAccepted);

    *spec = mSpec;
    (*button_sets) = mButtonSets;
}

void clUI_AllReminders_Dialog_EditSpec::build_buttons_table()
//according to `mButtonSets`
{
    disconnect(ui->tableWidget_buttons, SIGNAL(cellChanged(int,int)),
               this, SLOT(On_buttons_table_cell_data_changed(int,int)));

    ui->tableWidget_buttons->setRowCount(0);
    mButtonsTableItem_to_Index.clear();

    // applied button sets
    QStringList applied_predefined_button_sets;

    for(int i=0; i<mButtonSets.size(); i++)
    {
        if(mButtonSets.at(i)->is_predefined())
        {
            add_row_to_buttons_table(i, mButtonSets.at(i)->get_predefined_name());
            applied_predefined_button_sets << mButtonSets.at(i)->get_predefined_name();
        }
        else
            add_row_to_buttons_table(i, mButtonSets.at(i)->print());
    }

    // predefined button sets not applied
    foreach(QString BS_name, G_PredefinedButtonSets)
    {
        if(! applied_predefined_button_sets.contains(BS_name))
            add_row_to_buttons_table(-1, BS_name);
    }

    // custom button sets templates
    add_row_to_buttons_table(-1, "custom: basic ...");
    add_row_to_buttons_table(-1, "custom: alternate ...");
    add_row_to_buttons_table(-1, "custom: menu ...");

    //
    connect(ui->tableWidget_buttons, SIGNAL(cellChanged(int,int)),
            this, SLOT(On_buttons_table_cell_data_changed(int,int)));
}

void clUI_AllReminders_Dialog_EditSpec::add_row_to_buttons_table(
                                                           const int applied_button_set_index,
                                                           const QString &text)
//If `text` is for an applied button set, `applied_button_set_index` should be the
//corresponding index of `mButtonSets[]`. Otherwise, `applied_button_set_index` should be -1.
{
    const bool checked = (applied_button_set_index >= 0);
    QTableWidgetItem *item_check_box = new QTableWidgetItem("");
    item_check_box->setFlags(item_check_box->flags() | Qt::ItemIsUserCheckable);
    item_check_box->setCheckState(checked ? Qt::Checked : Qt::Unchecked);

    QTableWidgetItem *item_text = new QTableWidgetItem(text);

    //
    int Nrows = ui->tableWidget_buttons->rowCount();
    ui->tableWidget_buttons->setRowCount(++Nrows);

    ui->tableWidget_buttons->setItem(Nrows-1, 0, item_check_box);
    ui->tableWidget_buttons->setItem(Nrows-1, 1, item_text);

    //
    if(applied_button_set_index >= 0)
        mButtonsTableItem_to_Index.insert(item_check_box, applied_button_set_index);
}

void clUI_AllReminders_Dialog_EditSpec::On_buttons_table_cell_data_changed(int row, int col)
{
    if(col != 0 || row < 0)
        return;

    //
    QTableWidgetItem *item_check_box = ui->tableWidget_buttons->item(row, 0);
    bool checked = item_check_box->checkState() == Qt::Checked;
    QString text = ui->tableWidget_buttons->item(row, 1)->text();

    if(! checked) //item unchecked -> remove from `mButtonSets[]`
    {
        Q_ASSERT(mButtonsTableItem_to_Index.contains(item_check_box));
        int i = mButtonsTableItem_to_Index[item_check_box];

        Q_ASSERT(i<mButtonSets.size());
        mButtonSets.removeAt(i);
    }
    else //item checked -> add to `mButtonSets[]`
    {
        if(G_PredefinedButtonSets.contains(text)) //is predefined
        {
            auto ptr = create_predefined_button_set(text);
            Q_ASSERT(ptr != 0);
            mButtonSets << ptr;
        }
        else //adding custom button set
        {
            clDataElem_ButtonSet_Abstract::enType type;
            Q_ASSERT(text.startsWith("custom: "));
            text = text.mid(8);
            if(text.startsWith("basic"))
                type = clDataElem_ButtonSet_Abstract::Basic;
            else if(text.startsWith("alternate"))
                type = clDataElem_ButtonSet_Abstract::Alternate;
            else if(text.startsWith("menu"))
                type = clDataElem_ButtonSet_Abstract::Menu;
            else
                Q_ASSERT(false);

            clUI_AllReminders_Dialog_AddCustomButtonSet dialog(type);
            int r = dialog.exec();
            if(r == QDialog::Accepted)
                mButtonSets << dialog.get_button_set();
        }
    }

    //
    build_buttons_table();
}

bool clUI_AllReminders_Dialog_EditSpec::has_ending_button() const
//according to `mButtonSets`
{
    for(auto it=mButtonSets.begin(); it!=mButtonSets.end(); it++)
    {
        if((*it)->has_ending_button())
            return true;
    }
    return false;
}

void clUI_AllReminders_Dialog_EditSpec::accept()
{
    // parse spec (if not yet)
    if(ui->pushButton_preview->text() == "Preview")
    {
        if(! parse_spec()) //sets `mSpec`
            return;
    }

    // check
    if(mSpec.has_bindings())
    {
        // cannot have ending button set
        if(has_ending_button())
        {
            QMessageBox::warning(this, "error",
                                 "A reminder with bindings cannot have ending button.");
            return;
        }
    }
    else if(mSpec.has_triggers())
    {
        // must have ending button set
        if(! has_ending_button())
        {
            QMessageBox::warning(this, "error",
                                 "A reminder with triggers must have an ending button.");
            return;
        }

    }
    else if(mSpec.has_date_setting())
    {
        //...
    }

    //
    mAccepted = true;
    QDialog::accept();
}

bool clUI_AllReminders_Dialog_EditSpec::parse_spec()
//parse the spec setting and set `mSpec`
{
    QString text = ui->plainTextEdit_spec->toPlainText();
    QDomDocument doc;
    QString err_msg;
    int err_line;
    bool ch = doc.setContent(text, false, &err_msg, &err_line);
    if(!ch)
    {
        QMessageBox::warning(this, "warning",
                             QString("Error in parsing spec text as XML.\n\n"
                                     "Line %1: %2.").arg(err_line).arg(err_msg));
        return false;
    }

    QDomElement elem_spec = doc.firstChildElement();
    if(elem_spec.isNull())
    {
        QMessageBox::warning(this, "warning",
                             "Error in parsing spec: No root element found.");
        return false;
    }

    ch = mSpec.parse_and_set(elem_spec, err_msg);
    if(!ch)
    {
        QMessageBox::warning(this, "warning",
                             QString("Error in parsing spec.\n\n%1").arg(err_msg));
        return false;
    }

    return true;
}

QString clUI_AllReminders_Dialog_EditSpec::print_spec_XML()
//print `mSpec` in XML
{
    QDomDocument doc;
    QDomElement elem_spec = doc.createElement("spec");
    doc.appendChild(elem_spec);

    mSpec.add_to_XML(doc, elem_spec);
    return doc.toString(2);
}

void clUI_AllReminders_Dialog_EditSpec::on_pushButton_preview_clicked()
{
    if(ui->pushButton_preview->text() == "Preview")
    {
        if(! parse_spec())
            return;

        ui->plainTextEdit_spec->setPlainText(mSpec.print());

        ui->plainTextEdit_spec->setReadOnly(true);
        ui->plainTextEdit_spec->setStyleSheet("font: 10pt");

        ui->pushButton_preview->setText("Edit");
        ui->pushButton_insert->setEnabled(false);
    }
    else //"Edit"
    {
        QString spec_xml = print_spec_XML();
        ui->plainTextEdit_spec->setPlainText(spec_xml);

        ui->plainTextEdit_spec->setReadOnly(false);
        ui->plainTextEdit_spec->setStyleSheet(QString("font: 10pt \"%1\"")
                                              .arg(gMonospaceFont));

        ui->pushButton_preview->setText("Preview");
        ui->pushButton_insert->setEnabled(true);
    }
}

void clUI_AllReminders_Dialog_EditSpec::on_pushButton_show_helper_clicked()
{
    mHelperDialog->show();
    mHelperDialog->move(this->pos() + QPoint(600,0));
}

void clUI_AllReminders_Dialog_EditSpec::on_pushButton_insert_clicked()
{
    clUI_Dialog_SpecElementInsert dialog(pSituations, pEvents);
    int r = dialog.exec();
    if(r == QDialog::Accepted)
    {
        QString text = dialog.get();
        ui->plainTextEdit_spec->insertPlainText(text);
        ui->plainTextEdit_spec->setFocus();
    }
}
