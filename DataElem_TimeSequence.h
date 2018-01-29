#ifndef DATA_ELEMENT_TIME_SEQUENCE_H
#define DATA_ELEMENT_TIME_SEQUENCE_H

#include <QString>
#include <QDateTime>
#include <QList>

/* A (relative) time sequence that can be specified by start time (in minutes), repeating
 * time interval (in minutes), and number of time instants, e.g., (25m, 30m, 10).
 * The number of time instants can be infinite. */

class clDataElem_TimeSequence
{
public:
    clDataElem_TimeSequence();
    clDataElem_TimeSequence(const int t_start, const int t_step, const int Ntimes);
                               //`Ntimes` must be > 0 or = -1 (infinite times).
                               //`t_step` must be > 0.

    void set(const int t_start, const int t_step, const int Ntimes);
             //`Ntimes` must be > 0 or = -1 (infinite times). `t_step` must be > 0.

    bool parse_and_set(const QString &S);
        //format:
        //   [<start>]                   --> single time
        //   [<start>, <step>]           --> infinite
        //   [<start>, <step>, <times>]  --> general
        //   [<start>, <step>, <until>]  --> finite
        //   <start>                                              --> single time
        //   from <start> repeat every <step>                     --> infinite
        //   from <start> repeat every <step> for <repeat_times> times  --> general
        //   from <start> repeat every <step> until <until>       --> finite
        //where <start>, <step>, <until> are of the form "<x>h", "<x>h<x>m" or "<x>m", and
        //<times> and <repeat_times> are integers.
        //<step> must be > 0.
        //<times>  --  -1: infinite  0: empty sequence  >=1: finite time(s)
        //<repeat_times>  -- -1: infinite  >=0: finite time(s), corresponding to <times>-1

    void clear() { m_Ntimes = 0; }

    //
    bool is_empty() const { return m_Ntimes == 0; }
    bool is_infinite() const { return m_Ntimes == -1; }
    int get_t_start() const; //get start time in minutes
    int get_Ntimes() const { return m_Ntimes; }
    QString print() const;

    QList<QDateTime> get_times(const QDateTime &base_time) const;
                     //Return the times represented by `*this`, which must be finite.
    QList<QDateTime> get_times_within_time_range(const QDateTime &base_time,
                                                 const QDateTime &t0, const QDateTime &t1) const;
                     //Return the times represented by `*this` within (`t0`, `t1`], regarding 0m
                     //as the time `base_time`.

    QDateTime get_latest_time(const QDateTime &base_time) const;
                     //`*this` must be finite and non-empty

    //
    bool operator == (const clDataElem_TimeSequence &another) const;

private:
    int m_Ntimes; //including `m_tStart`. Must be >= -1.
                  //0 => `*this` is empty; -1 => infinite
    int m_tStart; //(minute)
    int m_tStep; //(minute) must be > 0. Meaningless if `m_Ntimes` = 0 or 1.
    // The sequence is
    //    {t_i = `m_tStart` + i*`m_tStep` | i = 0, ..., `m_Ntimes`-1}.
};

#endif // DATA_ELEMENT_TIME_SEQUENCE_H
