#ifndef DIALOG_SPECELEMENTINSERT_H
#define DIALOG_SPECELEMENTINSERT_H

#include <QDialog>
#include <QSet>
#include <QString>
#include <QDate>

namespace Ui {
class clUI_Dialog_SpecElementInsert;
}

class clUI_Dialog_SpecElementInsert : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_Dialog_SpecElementInsert(const QSet<QString> *p_Situations,
                                        const QSet<QString> *p_Events,
                                        QWidget *parent = nullptr);
    ~clUI_Dialog_SpecElementInsert();

    QString get() const { return mResult; }

private slots:
    void on_pushButton_today_clicked();
    void on_pushButton_dates_clicked();
    void situation_selected(const QString &sit);
    void event_selected(const QString &event);

private:
    Ui::clUI_Dialog_SpecElementInsert *ui;
    QString mResult;
};

#endif // DIALOG_SPECELEMENTINSERT_H
