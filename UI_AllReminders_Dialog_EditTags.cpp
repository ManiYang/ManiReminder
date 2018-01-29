#include <QDebug>
#include <QMessageBox>
#include "UI_AllReminders_Dialog_EditTags.h"
#include "ui_UI_AllReminders_Dialog_EditTags.h"

clUI_AllReminders_Dialog_EditTags::clUI_AllReminders_Dialog_EditTags(const QStringList &tags,
                                                                     QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clUI_AllReminders_Dialog_EditTags)
{
    ui->setupUi(this);

    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->setSpacing(6);
    ui->listWidget->setResizeMode(QListView::Adjust); //layout item when the widget get resized

    //
    for(int i=0; i<tags.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::DisplayRole, tags.at(i));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }

    add_row_for_new_tag();

    //
    connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(On_item_modified(QListWidgetItem*)));
}


QStringList clUI_AllReminders_Dialog_EditTags::get_tags() const
{
    QStringList tags;
    for(int r=0; r<ui->listWidget->count()-1; r++)
        tags << ui->listWidget->item(r)->text();
    return tags;
}

clUI_AllReminders_Dialog_EditTags::~clUI_AllReminders_Dialog_EditTags()
{
    delete ui;
}

void clUI_AllReminders_Dialog_EditTags::On_item_modified(QListWidgetItem *item)
{
    disconnect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
               this, SLOT(On_item_modified(QListWidgetItem*)));

    //
    if(item->text().contains(','))
        QMessageBox::warning(this, "warning",
                             "A tag should not contain character ','. Please change it.");

    //
    int row = ui->listWidget->row(item);

    if(row == ui->listWidget->count()-1) //(last row)
    {
        if(item->text().simplified().isEmpty())
            item->setData(Qt::DisplayRole, QString("new tag"));
        else
        {
            item->setData(Qt::ForegroundRole, QBrush(QColor("#000")));
            add_row_for_new_tag();
        }
    }
    else
    {
        if(item->text().simplified().isEmpty())
            delete item;
    }

    //
    connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(On_item_modified(QListWidgetItem*)));
}

void clUI_AllReminders_Dialog_EditTags::add_row_for_new_tag()
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::DisplayRole, QString("new tag"));
    item->setData(Qt::ForegroundRole, QBrush(QColor("#888")));
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->listWidget->addItem(item);
}

void clUI_AllReminders_Dialog_EditTags::accept()
{
    QList<QString> temp_list;
    for(int r=0; r<ui->listWidget->count()-1; r++)
    {
        QString tag = ui->listWidget->item(r)->text().simplified();
        if(tag.contains(','))
        {
            QMessageBox::warning(this, "error",
                                 "Error. There is tag containing character ','." );
            return;
        }

        if(temp_list.contains(tag))
        {
            QMessageBox::warning(this, "error", "Error. There are duplicated tags." );
            return;
        }

        temp_list << tag;
    }

    QDialog::accept();
}
