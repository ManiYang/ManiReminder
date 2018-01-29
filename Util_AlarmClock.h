//version 1.5

#ifndef CLALARMCLOCK_H
#define CLALARMCLOCK_H

#include <QDateTime>
#include <QTimer>

class clUtil_AlarmClock : public QObject
{
    Q_OBJECT

public:
    explicit clUtil_AlarmClock(QObject *parent = 0);

    void set(const QDateTime &t_target);
         //Set the alarm clock so that it emits the signal 'ring()' at 't_target'.
         //If 't_target' is past, 'ring()' is emitted immediately.

    void cancel();

signals:
    void ring();

private slots:
    void set_timer();

private:
    QDateTime m_t_target;
    QTimer *m_timer;

    // Timer times out and set_timer() is called every 1 minute until 'm_t_target'
    // is imminent.
};

#endif // CLALARMCLOCK_H
