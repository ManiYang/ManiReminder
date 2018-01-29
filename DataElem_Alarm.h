#ifndef DATA_ELEMENT_ALARM_H
#define DATA_ELEMENT_ALARM_H

#include <QString>
#include <QDomElement>

class clDataElem_ReminderAlarm
{
public:
    clDataElem_ReminderAlarm();
    
    void set(const int pause_after_sec, const int pause_interval_sec, const bool play_sound);
    void clear();

    bool parse_and_set(const QString &setting, QString &error_msg);
         //format:
         //   [with sound;] pause after <dt> for <dt>
         //where <dt> can be "<n>m", "<n>s", or "<n>m<n>s"

    //
    bool is_empty() const { return mPauseAfter_sec == 0; }

    bool play_sound() const;
    int get_pause_after_sec() const;
    int get_pause_interval_sec() const;

    QString print() const;

private:
    int mPauseAfter_sec; //0: *this is empty
    int mPauseInterval_sec; //"snooze" time, must be >= 1
    bool mPlaySound;

      // Blink the reminder title and play the sound (if `mPlaySound` is true) starting at
      // the moment the reminder is shown on the board.
      // The alarm (sound and blinking) stops when user clicks on a button or when the reminder
      // is closed.

      //what if the reminder has no button...?

      // The alarm pauses for `mPauseInterval_sec` minutes when the user clicked "delay"
      // ("snooze"), or when `mPauseAfter_sec` sec has elapsed since last start or resume of
      // the alarm.
};

#endif // DATA_ELEMENT_ALARM_H
