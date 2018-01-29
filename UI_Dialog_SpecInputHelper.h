#ifndef UI_DIALOG_SPECINPUTHELPER_H
#define UI_DIALOG_SPECINPUTHELPER_H

#include <QDialog>
#include <QDomElement>
#include <QComboBox>

namespace Ui {
class clUI_Dialog_SpecInputHelper;
}

class clUI_Dialog_SpecInputHelper : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_Dialog_SpecInputHelper(QWidget *parent = nullptr);
    ~clUI_Dialog_SpecInputHelper();

private slots:
    void On_comboBox_group_currentIndexChanged(int index);
    void On_comboBox_spec_currentIndexChanged(int index);

private:
    Ui::clUI_Dialog_SpecInputHelper *ui;

    QDomElement mRoot;
};

#endif // UI_DIALOG_SPECINPUTHELPER_H
