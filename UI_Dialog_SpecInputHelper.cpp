#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QDebug>
#include <QStringList>
#include "UI_Dialog_SpecInputHelper.h"
#include "ui_UI_Dialog_SpecInputHelper.h"
#include "utility_DOM.h"

const QString gMonospaceFont("monospace");

clUI_Dialog_SpecInputHelper::clUI_Dialog_SpecInputHelper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clUI_Dialog_SpecInputHelper)
{
    ui->setupUi(this);

    setWindowTitle("Spec Input Helper");

    //
    ui->comboBox_group->setCurrentIndex(-1);
    connect(ui->comboBox_group, SIGNAL(currentIndexChanged(int)),
            this, SLOT(On_comboBox_group_currentIndexChanged(int)));

    //
    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->plainTextEdit->setStyleSheet(QString("font: 10pt \"%1\"").arg(gMonospaceFont));

    // read file
    const QString fname = ":/files/spec_setting_format.xml";
    QFile F(fname);
    if(! F.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "error",
                             QString("Could not open \"%1\" for reading.").arg(fname));
        close();
    }

    QDomDocument doc;
    bool ch = doc.setContent(&F);
    Q_ASSERT(ch);
    F.close();

    mRoot = doc.firstChildElement("root");
    Q_ASSERT(! mRoot.isNull());
}

clUI_Dialog_SpecInputHelper::~clUI_Dialog_SpecInputHelper()
{
    delete ui;
}

void clUI_Dialog_SpecInputHelper::On_comboBox_group_currentIndexChanged(int index)
{
    disconnect(ui->comboBox_spec, SIGNAL(currentIndexChanged(int)),
               this, SLOT(On_comboBox_spec_currentIndexChanged(int)));
               //to prevent crash (likely a bug of QComboBox::addItems())

    switch(index)
    {
    case 0: //bindings
        ui->comboBox_spec->clear();
        ui->comboBox_spec->addItems(QStringList()
                                         << "situation binding"
                                         << "time-range binding"
                                         << "g-event-duration binding"
                                         << "time-range filter");
        break;

    case 1: //triggers
        ui->comboBox_spec->clear();
        ui->comboBox_spec->addItems(QStringList()
                                         << "time trigger"
                                         << "g-event trigger"
                                         << "time-range filter");
        break;

    case 2: //date-setting
        ui->comboBox_spec->clear();
        ui->comboBox_spec->addItems(QStringList()
                                         << "date-setting");
        break;
    }

    ui->comboBox_spec->setCurrentIndex(-1);

    connect(ui->comboBox_spec, SIGNAL(currentIndexChanged(int)),
            this, SLOT(On_comboBox_spec_currentIndexChanged(int)));

    ui->comboBox_spec->setCurrentIndex(0);
}

void clUI_Dialog_SpecInputHelper::On_comboBox_spec_currentIndexChanged(int index)
{
    int group = ui->comboBox_group->currentIndex();
    QString text;
    if(group == 0) //bindings
    {
        if(index == 0) //situation binding
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "situation-binding");
        else if(index == 1) //time-range binding
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "time-range-binding");
        else if(index == 2) //g-event-duration binding
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "g-event-duration-binding");
        else if(index == 3) //time-range filter
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "time-range-filter");
    }
    else if(group == 1) //triggers
    {
        if(index == 0) //time trigger
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "time-trigger");
        else if(index == 1) //g-event trigger
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "g-event-trigger");
        else if(index == 2) //time-range filter
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "time-range-filter");
    }
    else if(group == 2) //date-setting
    {
        if(index == 0) //date-setting
            text = nsDomUtil::get_text_of_first_child_element(mRoot, "date-setting");
    }

    //
    if(text.at(0) == QChar('\n'))
        text = text.mid(1);
    ui->plainTextEdit->setPlainText(text);
}
