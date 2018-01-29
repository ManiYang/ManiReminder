#include <QDebug>
#include <QStringList>
#include "DataElem_Alarm.h"
#include "utility_time.h"

clDataElem_ReminderAlarm::clDataElem_ReminderAlarm()
{
    mPauseAfter_sec = 0;
}

void clDataElem_ReminderAlarm::clear()
{
    mPauseAfter_sec = 0;
    mPauseInterval_sec = 1;
    mPlaySound = false;
}

void clDataElem_ReminderAlarm::set(const int pause_after_sec, const int pause_interval_sec,
                              const bool play_sound)
{
    clear();

    Q_ASSERT(pause_after_sec >= 1);
    Q_ASSERT(pause_interval_sec >= 1);

    mPauseAfter_sec = pause_after_sec;
    mPauseInterval_sec = pause_interval_sec;
    mPlaySound = play_sound;
}

static int parse_as_seconds(const QString &S_)
//e.g.: "10s", "2m", "1m30s"
//Retruns -1 if failed.
{
    QString sec_str, min_str;
    QStringList tokens = S_.simplified().split('m');
    if(tokens.size() == 1)
    {
        if(tokens[0].endsWith('s'))
            sec_str = tokens[0].left(tokens[0].size()-1);
        else
            min_str = tokens[0];
    }
    else if(tokens.size() == 2)
    {
        min_str = tokens[0];
        sec_str = tokens[1].left(tokens[1].size()-1);
    }
    else
        return -1;

    //
    int result = 0;
    bool ok;
    if(! min_str.isEmpty())
    {
        int m = min_str.toInt(&ok);
        if(!ok)
            return -1;
        result += m*60;
    }
    if(! sec_str.isEmpty())
    {
        int s = sec_str.toInt(&ok);
        if(!ok)
            return -1;
        result += s;
    }

    return result;
}

QString clDataElem_ReminderAlarm::print() const
//[with sound;] pause after <dt> for <dt>
{
    QString S;
    if(is_empty())
        return S;

    if(mPlaySound)
        S += "with sound; ";

    S += QString("pause after %1 for %2")
            .arg(print_xmxs(mPauseAfter_sec))
            .arg(print_xmxs(mPauseInterval_sec));

    return S;
}

bool clDataElem_ReminderAlarm::parse_and_set(const QString &setting, QString &error_msg)
//format:
//   [with sound;] pause after <dt> for <dt>
//where <dt> can be "<n>m", "<n>s", or "<n>m<n>s"
{
    QString S = setting.simplified();

    clear();
    if(S.isEmpty())
        return true;

    //
    QStringList tokens = S.split(';');
    if(tokens.size() > 2)
    {
        error_msg = "Unrecognized setting.";
        return false;
    }

    //
    QString pause_setting;
    if(tokens.size() == 2)
    {
        if(tokens.at(0).simplified() != "with sound")
        {
            error_msg = QString("Unrecognized setting: \"%1\"").arg(tokens.at(0));
            return false;
        }
        mPlaySound = true;
        pause_setting = tokens.at(1);
    }
    else
        pause_setting = tokens.at(0);

    // parse `pause_setting` (e.g. "pause after 10s for 1m")
    QStringList words = pause_setting.split(' ', QString::SkipEmptyParts);
    if(words.size() != 5)
    {
        error_msg = QString("Could not parse \"%1\" as pause setting.").arg(pause_setting);
        return false;
    }
    if(words[0]!="pause" || words[1]!="after" || words[3]!="for")
    {
        error_msg = QString("Could not parse \"%1\" as pause setting.").arg(pause_setting);
        return false;
    }

    int dt = parse_as_seconds(words[2]); //pause-after
    if(dt == -1)
    {
        error_msg = QString("Could not parse \"%1\" as seconds.").arg(words[2]);
        return false;
    }
    if(dt == 0)
    {
        error_msg = "The setting for pause-after must be >= 1s";
        return false;
    }
    mPauseAfter_sec = dt;

    dt = parse_as_seconds(words[4]); //pause interval
    if(dt == -1)
    {
        error_msg = QString("Could not parse \"%1\" as seconds.").arg(words[4]);
        return false;
    }
    if(dt == 0)
    {
        error_msg = "Pause interval must be >= 1s";
        return false;
    }
    mPauseInterval_sec = dt;

    //
    return true;
}

bool clDataElem_ReminderAlarm::play_sound() const
{
    Q_ASSERT(! is_empty());
    return mPlaySound;
}

int clDataElem_ReminderAlarm::get_pause_after_sec() const
{
    Q_ASSERT(! is_empty());
    return mPauseAfter_sec;
}

int clDataElem_ReminderAlarm::get_pause_interval_sec() const
{
    Q_ASSERT(! is_empty());
    return mPauseInterval_sec;
}
