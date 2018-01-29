#ifndef UI_DEBUG_SCHEDULED_ACTIONS_H
#define UI_DEBUG_SCHEDULED_ACTIONS_H

#include <QDialog>
#include <QDateTime>
#include <QStringList>

namespace Ui {
class clUI_Debug_ScheduledActions;
}

class clUI_Debug_ScheduledActions : public QDialog
{
    Q_OBJECT

public:
    explicit clUI_Debug_ScheduledActions(QWidget *parent = 0);
    ~clUI_Debug_ScheduledActions();

    void set_data(const QDateTime &t, const QString &reminder_title,
                  const QMultiMap<QDateTime,QString> &scheduled_actions);

private:
    Ui::clUI_Debug_ScheduledActions *ui;
};

#endif // UI_DEBUG_SCHEDULED_ACTIONS_H
