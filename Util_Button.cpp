#include "Util_Button.h"
#include "utility_DOM.h"

clUtil_Button::clUtil_Button()
    : mEnding(false)
{
}

clUtil_Button::clUtil_Button(const QString &text, const QString &icon_file, const bool ending,
                             const QString &log_text)
    : mText(text), mIconFile(icon_file), mEnding(ending), mLogText(log_text)
{
}

bool clUtil_Button::parse_and_set(QDomElement &parent)
//under `parent`:
//   <text> text </text>
//   <icon> icon </icon>
//   <ending/>
//   <log-text> log text </log-text>
//Every element is optional.
{
    mText = "";
    mIconFile = "";
    mEnding = false;
    mLogText = "";

    QString S = nsDomUtil::get_text_of_first_child_element(parent, "text").simplified();
    if(! S.isEmpty())
        mText = S;

    S = nsDomUtil::get_text_of_first_child_element(parent, "icon").simplified();
    if(! S.isEmpty())
        mIconFile = S;

    if(! parent.firstChildElement("ending").isNull())
        mEnding = true;

    S = nsDomUtil::get_text_of_first_child_element(parent, "log-text").simplified();
    if(! S.isEmpty())
        mLogText = S;

    //
    return true;
}

QString clUtil_Button::get_icon_file() const
//return empty string if no icon
{
    if(mIconFile.isEmpty())
        return QString();
    else
        return ":/icons/icons/16/"+mIconFile;
}

void clUtil_Button::add_to_XML(QDomDocument &doc, QDomElement &parent) const
//add under `parent`
{
    //under `parent`:
    //   <text> text </text>
    //   <icon> icon </icon>
    //   <ending/>
    //   <log-text> log text </log-text>

    if(! mText.isEmpty())
        nsDomUtil::add_child_element_w_text(doc, parent, "text", mText);
    if(! mIconFile.isEmpty())
        nsDomUtil::add_child_element_w_text(doc, parent, "icon", mIconFile);
    if(mEnding)
        nsDomUtil::add_child_element(doc, parent, "ending");
    if(! mLogText.isEmpty())
        nsDomUtil::add_child_element_w_text(doc, parent, "log-text", mLogText);
}

QString clUtil_Button::print() const
{
    return QString("text=\"%1\", icon=\"%2\", ending=%3, log-text=\"%4\"")
            .arg(mText, mIconFile, (mEnding?"true":"false"), mLogText);
}

bool clUtil_Button::operator == (const clUtil_Button &another) const
{
    return mText == another.mText
            && mIconFile == another.mIconFile
            && mEnding == another.mEnding
            && mLogText == mLogText;
}
