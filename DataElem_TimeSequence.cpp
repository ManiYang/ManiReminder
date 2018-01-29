#include <QStringList>
#include <cmath>
#include "DataElem_TimeSequence.h"
#include "utility_math.h"

clDataElem_TimeSequence::clDataElem_TimeSequence()
    : m_Ntimes(0)
{

}

clDataElem_TimeSequence::clDataElem_TimeSequence(const int t_start,
                                                       const int t_step, const int Ntimes)
//`Ntimes` must be > 0 or = -1 (infinite times).
//`t_step` must be > 0.
{
    Q_ASSERT(t_step > 0);
    Q_ASSERT(Ntimes > 0 || Ntimes == -1);
    m_tStart = t_start;
    m_tStep = t_step;
    m_Ntimes = Ntimes;
}

void clDataElem_TimeSequence::set(const int t_start, const int t_step, const int Ntimes)
//`Ntimes` must be > 0 or = -1 (infinite times).
//`t_step` must be > 0.
{
    Q_ASSERT(t_step > 0);
    Q_ASSERT(Ntimes > 0 || Ntimes == -1);
    m_tStart = t_start;
    m_tStep = t_step;
    m_Ntimes = Ntimes;
}

static bool parse_xhxm(const QString &S_, int *minutes)
//"<x>h", "<x>h<x>m", "<x>m"
//<x> is an integer
{
    QString S = S_.simplified();

    if(S.endsWith('h'))
    {
        S = S.left(S.size()-1);
        bool ch;
        int hr = S.toInt(&ch);
        if(!ch)
            return false;
        *minutes = hr*60;
        return true;
    }

    ///
    if(! S.endsWith('m'))
        return false;
    S = S.left(S.size()-1);

    int hr, min;
    QStringList tokens = S.split('h');
    if(tokens.size() == 1)
    {
        hr = 0;

        bool ok;
        min = tokens.at(0).simplified().toInt(&ok);
        if(!ok)
            return false;
    }
    else if(tokens.size() == 2)
    {
        bool ok;
        hr = tokens.at(0).simplified().toInt(&ok);
        if(!ok)
            return false;

        min = tokens.at(1).simplified().toInt(&ok);
        if(!ok)
            return false;
    }
    else
        return false;

    //
    *minutes = hr*60 + min;
    return true;
}

bool clDataElem_TimeSequence::parse_and_set(const QString &S_)
//format 1:
//   [<start>]                   --> single time
//   [<start>, <step>]           --> infinite
//   [<start>, <step>, <times>]  --> general
//   [<start>, <step>, <until>]  --> finite
//
//format 2:
//   <start>                                              --> single time
//   from <start> repeat every <step>                     --> infinite
//   from <start> repeat every <step> for <repeat_times> times  --> general
//   from <start> repeat every <step> until <until>       --> finite
//
//where <start>, <step>, <until> are of the form "<x>h", "<x>h<x>m" or "<x>m", and
//<times> and <repeat_times> are integers.
//<step> must be > 0.
//<times>  --  -1: infinite  0: empty sequence  >=1: finite time(s)
//<repeat_times>  -- -1: infinite  >=0: finite time(s), corresponding to <times>-1
{
    QString S = S_.simplified();

    if(S.startsWith('[') && S.endsWith(']')) //(format 1)
    {
        S = S.mid(1, S.size()-2);
        QStringList tokens = S.split(',');
        if(tokens.size() < 1 || tokens.size() > 3)
            return false;

        //
        if(! parse_xhxm(tokens.at(0), &m_tStart))
            return false;
        m_Ntimes = 1; //single time
        m_tStep = 0;

        //
        if(tokens.size() >= 2)
        {
            if(! parse_xhxm(tokens.at(1), &m_tStep))
                return false;
            m_Ntimes = -1; //infinite
        }

        //
        if(tokens.size() == 3)
        {
            int until;
            bool ch = parse_xhxm(tokens.at(2), &until);
            if(ch)
            {
                if(m_tStep <= 0)
                    return false;
                if((until - m_tStart)%m_tStep == 0)
                    m_Ntimes = (until - m_tStart)/m_tStep;
                else
                    m_Ntimes = (until - m_tStart)/m_tStep + 1;

                if(m_Ntimes < 0)
                    m_Ntimes = 0;
            }
            else
            {
                m_Ntimes = tokens.at(2).simplified().toInt(&ch);
                if(!ch)
                    return false;
            }
        }
    }
    else //(format 2)
    {
        if(! S.startsWith("from ")) //"<start>"
        {
           if(! parse_xhxm(S, &m_tStart))
                return false;
            m_tStep = 0;
            m_Ntimes = 1;
        }
        else // (starts with "from ")
        {
            S = S.mid(5);
            QStringList tokens = S.split(" repeat every ");
            if(tokens.size() != 2)
                return false;

            if(! parse_xhxm(tokens.at(0), &m_tStart))
                return false;

            S = tokens.at(1); //`S`: "<step>"
                              //     "<step> for <repeat_times> time(s)"
                              //     "<step> until <until>"
            if(S.endsWith("time") || S.endsWith("times"))
            {
                tokens = S.split(" for ");
                if(tokens.size() != 2)
                    return false;

                if(! parse_xhxm(tokens.at(0), &m_tStep))
                    return false;

                S = tokens.at(1);
                S = S.left(S.size()-5);
                bool ch;
                int repeat_times = S.simplified().toInt(&ch);
                if(!ch)
                    return false;
                if(repeat_times == -1)
                    m_Ntimes = -1;
                else
                    m_Ntimes = repeat_times + 1;
            }
            else //"<step> (until <until>)"
            {
                tokens = S.split(" until ");
                if(tokens.size() > 2)
                    return false;

                if(! parse_xhxm(tokens.at(0), &m_tStep))
                    return false;
                m_Ntimes = -1;

                if(tokens.size() == 2)
                {
                    int until;
                    if(! parse_xhxm(tokens.at(1), &until))
                        return false;

                    if(m_tStep <= 0)
                        return false;

                    if(until - m_tStart <= 0)
                        m_Ntimes = 0;
                    else
                    {
                        if((until - m_tStart)%m_tStep == 0)
                            m_Ntimes = (until - m_tStart)/m_tStep;
                        else
                            m_Ntimes = (until - m_tStart)/m_tStep + 1;
                    }
                }
            }
        }
    }

    //
    if(m_Ntimes < -1)
        return false;

    if(m_Ntimes > 1)
    {
        if(m_tStep <= 0)
            return false;
    }

    return true;
}

static QString print_xhxm(const int minutes)
{
    if(minutes < 0) //just in case
    {
        return QString("%1m").arg(minutes);
    }

    int hr = minutes / 60;
    int min = minutes % 60;
    if(hr == 0)
        return QString("%1m").arg(min);
    else
        return QString("%1h%2m").arg(hr).arg(min);
}

QString clDataElem_TimeSequence::print() const
//format:
//   [<start>]                   --> single time
//   [<start>, <step>]           --> infinite
//   [<start>, <step>, <times>]  --> finite multiple times
{
    if(m_Ntimes == 0)
        return "[]";

    QString str_start = print_xhxm(m_tStart);
    if(m_Ntimes == 1)
        return QString("[%1]").arg(str_start);

    QString str_step = print_xhxm(m_tStep);
    if(m_Ntimes == -1)
        return QString("[%1, %2]").arg(str_start, str_step);

    QString str_times = QString::number(m_Ntimes);
    return QString("[%1, %2, %3]").arg(str_start, str_step, str_times);
}

int clDataElem_TimeSequence::get_t_start() const
{
    Q_ASSERT(! is_empty());
    return m_tStart;
}

QList<QDateTime> clDataElem_TimeSequence::get_times(const QDateTime &base_time) const
//Return the times represented by `*this`, which must be finite.
{
    Q_ASSERT(m_Ntimes != -1);

    QList<QDateTime> ts;

    if(m_Ntimes == 0)
        return ts;

    for(int i=0; i<m_Ntimes; i++)
    {
        int minutes = m_tStart + m_tStep*i;
        ts << base_time.addSecs(minutes*60);
    }
    return ts;
}

QList<QDateTime> clDataElem_TimeSequence::get_times_within_time_range(
                    const QDateTime &base_time, const QDateTime &t0, const QDateTime &t1) const
//Return the times represented by `*this` within (`t0`, `t1`], regarding 0m as the time
//`base_time`.
{
    if(is_empty())
        return QList<QDateTime>();

    Q_ASSERT(t0.isValid() && t1.isValid());
    Q_ASSERT(t0 < t1);

    const double tt0 = base_time.secsTo(t0)/60.0; //(minute)
    const double tt1 = base_time.secsTo(t1)/60.0; //(minute)

    //
    if(m_Ntimes == 1)
    {
        QList<QDateTime> ts;
        if(tt0 < m_tStart && m_tStart <= tt1)
            ts << base_time.addSecs(m_tStart*60);
        return ts;
    }

    //(`m_Ntimes` = -1 or >= 2)
    const double x0 = (tt0 - m_tStart)/m_tStep,
                 x1 = (tt1 - m_tStart)/m_tStep;
    const int i0 = int(floor(x0)) + 1,
              i1 = int(floor(x1));

    QList<QDateTime> ts;
    const int i_min = max_of_two(i0, 0);
    const int i_max = (m_Ntimes == -1) ? i1 : min_of_two(i1, m_Ntimes-1);
    for(int i=i_min; i<=i_max; i++)
    {
        int minutes = m_tStart + m_tStep*i;
        ts << base_time.addSecs(minutes*60);
    }

    return ts;
}

QDateTime clDataElem_TimeSequence::get_latest_time(const QDateTime &base_time) const
//`*this` must be finite
{
    Q_ASSERT(! is_empty());
    Q_ASSERT(! is_infinite());

    qint64 Dt = m_tStart + m_tStep*(m_Ntimes - 1);
    return base_time.addSecs(Dt*60);
}

bool clDataElem_TimeSequence::operator ==(const clDataElem_TimeSequence &another) const
{
    if(m_Ntimes != another.m_Ntimes)
        return false;
    if(m_Ntimes == 0)
        return true;
    if(m_tStart != another.m_tStart)
        return false;
    if(m_Ntimes == 1)
        return true;
    return m_tStep == another.m_tStep;
}
