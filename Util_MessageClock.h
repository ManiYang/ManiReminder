#ifndef CLUTIL_MESSAGECLOCK_H
#define CLUTIL_MESSAGECLOCK_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QMultiMap>
#include "Util_AlarmClock.h"

class clUtil_MessageClock : public QObject
{
    Q_OBJECT

public:
    explicit clUtil_MessageClock(QObject *parent = 0);

    void schedule(const QDateTime &t, const QString &message);
         // Will emit 'notify(t, message)' at time 't' or immediately if 't' is past.

    void cancel(const QString &message);
                //cancal every scheduled notification with message 'message'
    void cancel_all(); //cancel all scheduled nofications

    //
    QMultiMap<QDateTime, QString> get_all_scheduled_messages() const { return m_messages; }

signals:
    void notify(QDateTime t, QString message);

private slots:
    void On_alarm_clock_rings();

private:
    clUtil_AlarmClock m_alarm_clock;
    QMultiMap<QDateTime, QString> m_messages;

    void print_messages() const; //[debug]
};

#endif // CLUTIL_MESSAGECLOCK_H
