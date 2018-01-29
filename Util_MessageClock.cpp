#include <QDebug>
#include "Util_MessageClock.h"

clUtil_MessageClock::clUtil_MessageClock(QObject *parent)
    : QObject(parent)
{
    connect(&m_alarm_clock, SIGNAL(ring()), this, SLOT(On_alarm_clock_rings()));
}

void clUtil_MessageClock::On_alarm_clock_rings()
{
    //qDebug() << "clUtil_MessageClock::On_alarm_clock_rings()";

    // get first (time, message) pair
    Q_ASSERT(! m_messages.isEmpty());

    QMap<QDateTime, QString>::iterator head = m_messages.begin();
    QDateTime t1 = head.key();
    QString message1 = head.value();
    //qDebug() << "  first message: (" << t1.toString("hh:mm:ss") << ", " << message1 << ")";

    // remove first (time, message) pair
    m_messages.erase(head);
    //qDebug() << "  remaining scheduled messages:"; print_messages();

    // set alarm clock at the new first date-time
    if(! m_messages.isEmpty())
    {
        //qDebug() << "  set alarm clock at" << m_messages.firstKey().toString("hh:mm:ss");
        m_alarm_clock.set(m_messages.firstKey());
    }

    //
    emit notify(t1, message1);
}

void clUtil_MessageClock::cancel(const QString &message)
//cancal every scheduled notification with message 'message'
{
    // remove all scheduled (time, message) pairs with message equal to 'message'
    QMap<QDateTime, QString>::iterator it;
    for(it=m_messages.begin(); it!=m_messages.end(); )
    {
        if(it.value() == message)
            it = m_messages.erase(it);
        else
            it++;
    }

    // set alarm clock at the new first date-time
    if(! m_messages.isEmpty())
        m_alarm_clock.set(m_messages.firstKey());
}

void clUtil_MessageClock::cancel_all()
//cancel all scheduled nofications
{
    m_alarm_clock.cancel();
    m_messages.clear();
}

void clUtil_MessageClock::schedule(const QDateTime &t, const QString &message)
//Will emit 'notify(t, message)' at time 't' or immediately if 't' is past.
{
    m_messages.insert(t, message);
    m_alarm_clock.set(m_messages.firstKey());
}

void clUtil_MessageClock::print_messages() const
//[debug]
{
    if(m_messages.isEmpty())
        qDebug() << "    none";
    else
    {
        for(auto it=m_messages.constBegin(); it!=m_messages.constEnd(); it++)
            qDebug() << "    " << it.key().toString("hh:mm:ss") << ", msg =" << it.value();
    }
}
