#ifndef UTIL_BUTTON_H
#define UTIL_BUTTON_H
#include <QString>
#include <QDomDocument>
#include <QDomElement>

class clUtil_Button
{
public:
    clUtil_Button();
    clUtil_Button(const QString &text, const QString &icon_file, const bool ending,
                  const QString &log_text);

    bool parse_and_set(QDomElement &parent);
                       //under `parent`:
                       //   <text> text </text>
                       //   <icon> icon </icon>
                       //   <ending/>
                       //   <log-text> log text </log-text>
                       //Every element is optional.

    //
    QString get_text() const { return mText; }
    QString get_icon_file() const; //return empty string if no icon
    bool is_ending() const { return mEnding; }
    QString get_log_text() const { return mLogText; }

    QString print() const;
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const; //add under `parent`

    bool operator == (const clUtil_Button &another) const;

private:
    QString mText;
    QString mIconFile; //empty: no icon
    bool mEnding;
    QString mLogText; //empty: no log text
};

#endif // UTIL_BUTTON_H
