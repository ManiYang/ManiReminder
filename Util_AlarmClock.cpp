//version 1.5

#include <QDebug>
#include "Util_AlarmClock.h"

clUtil_AlarmClock::clUtil_AlarmClock(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setSingleShot(true);
}

void clUtil_AlarmClock::set(const QDateTime &t_target)
//Set the alarm clock so that it emits the signal 'ring()' at 't_target', or immediately
//if 't_target' is past.
{
    //qDebug() << "clUtil_AlarmClock::set() : t_target =" << t_target.toString("hh:mm:ss");
    Q_ASSERT(t_target.isValid());

    m_timer->stop(); //previous time-out (with signal `timeout()` not yet emitted) is ignored

    if(t_target <= QDateTime::currentDateTime())
    {
        //qDebug() << "  t_target is past";
        disconnect(m_timer, SIGNAL(timeout()), 0, 0);
        connect(m_timer, SIGNAL(timeout()), this, SIGNAL(ring()));
        m_timer->start(0);
    }
    else
    {
        //qDebug() << "  t_target is not past, set timer";
        m_t_target = t_target;
        disconnect(m_timer, SIGNAL(timeout()), 0, 0);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(set_timer()));
        set_timer();
    }
}

void clUtil_AlarmClock::cancel()
{
    m_timer->stop();
}

void clUtil_AlarmClock::set_timer()
{
    qint64 dt_ms = QDateTime::currentDateTime().msecsTo(m_t_target);

    if(dt_ms <= 30) //(within 0.03 sec to m_t_target)
    {
        // ring immediately
        emit ring();
    }
    else if(dt_ms <= 10000) //(0.03 ~ 10 sec to m_t_target)
    {
        // ring after 'dt_ms'
        disconnect(m_timer, SIGNAL(timeout()), 0, 0);
        connect(m_timer, SIGNAL(timeout()), this, SIGNAL(ring()));
        m_timer->start(dt_ms);
    }
    else if(dt_ms <= 80000) //(10 ~ 80 sec to m_t_target)
    {
        //set timer at 8 sec before t_target
        m_timer->start(dt_ms-8000);
    }
    else //(>80 sec to m_t_target)
    {
        //set timer at 60 sec from now
        m_timer->start(60000);
    }
}
