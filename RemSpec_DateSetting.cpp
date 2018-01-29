#include "RemSpec_DateSetting.h"
#include "utility_DOM.h"

clRemSpec_DateSetting::clRemSpec_DateSetting(QObject *parent) : QObject(parent)
{

}

void clRemSpec_DateSetting::clear()
{
    mDatePatterns.clear();
}

static int parse_precaution_setting(const QString &S)
//"-<N>d", N>=0
//return -1 if there's error
{
    if(!S.startsWith('-') || !S.endsWith('d'))
        return -1;
    QString str = S.mid(1);
    str.chop(1);

    bool ok;
    int N = str.toInt(&ok);
    if(!ok)
        return -1;

    if(N < 0)
        return -1;
    return N;
}

bool clRemSpec_DateSetting::parse_and_set(QDomElement &spec_setting, QString &parse_error_msg)
//Return false iff there's parsing error, in which case the error message will
//be copied to `parse_error_msg`.
{
    clear();

    //Find under `spec_setting`:
    //   <date-setting>
    //     <due-dates> date-pattern 1 </due-dates>
    //     <due-dates> date-pattern 2 </due-dates>
    //     ...
    //     [<precaution> -<N>d </precaution>]
    //   </date-setting>
    //where N >= 0.
    //If <precaution> is omitted, "-0d" will be assumed.

    int c = nsDomUtil::get_count_of_child_elements(spec_setting, "date-setting");
    if(c == 0)
        return true;

    if(c != 1)
    {
        parse_error_msg = QString("More than one <date-setting> found.");
        return false;
    }

    QDomElement elem_DS = spec_setting.firstChildElement("date-setting");

    //
    QStringList str_due_dates = nsDomUtil::get_texts_of_child_elements(elem_DS, "due-dates");
    if(str_due_dates.isEmpty())
    {
        parse_error_msg = QString("Tag <due-dates> not found in date-setting.");
        return false;
    }

    foreach(QString S, str_due_dates)
    {
        clDatePattern DP;
        bool ch = DP.parse_and_set(S);
        if(!ch)
        {
            parse_error_msg = QString("Could not parse \"%1\" as date pattern.").arg(S);
            return false;
        }

        mDatePatterns << DP;
    }

    //
    c = nsDomUtil::get_count_of_child_elements(elem_DS, "precaution");
    if(c == 0)
        mPrecautionDayCount = 0;
    else
    {
        if(c != 1)
        {
            parse_error_msg = QString("More than one <precaution> found in date-setting.");
            return false;
        }

        QString S = nsDomUtil::get_text_of_first_child_element(elem_DS, "precaution");
        int N = parse_precaution_setting(S.simplified());
        if(N == -1)
        {
            parse_error_msg
                  = QString("Could not parse \"%1\" as precaution setting (in date-setting).")
                    .arg(S);
            return false;
        }

        mPrecautionDayCount = N;
    }

    return true;
}

QString clRemSpec_DateSetting::print() const
{
    if(is_empty())
        return "";

    QString S = "date-setting: ";
    for(int i=0; i<mDatePatterns.size(); i++)
        S += "\n |  "+ mDatePatterns.at(i).print();
    if(mPrecautionDayCount > 0)
        S += QString("\n |  -%1d").arg(mPrecautionDayCount);
    return S;
}

void clRemSpec_DateSetting::add_to_XML(QDomDocument &doc, QDomElement &parent) const
{
    QDomElement elem_DS = nsDomUtil::add_child_element(doc, parent, "date-setting");

    for(int i=0; i<mDatePatterns.size(); i++)
    {
        QString S = mDatePatterns.at(i).print();
        nsDomUtil::add_child_element_w_text(doc, elem_DS, "due-dates", S);
    }

    if(mPrecautionDayCount > 0)
        nsDomUtil::add_child_element_w_text(doc, elem_DS, "precaution",
                                            QString("-%1d").arg(mPrecautionDayCount));
}

bool clRemSpec_DateSetting::operator == (const clRemSpec_DateSetting &another) const
{
    if(mDatePatterns != another.mDatePatterns)
        return false;

    if(mDatePatterns.isEmpty())
        return true;

    return mPrecautionDayCount == another.mPrecautionDayCount;
}

void clRemSpec_DateSetting::operator = (const clRemSpec_DateSetting &another)
{
    mDatePatterns = another.mDatePatterns;
    mPrecautionDayCount = another.mPrecautionDayCount;
}

bool clRemSpec_DateSetting::is_due_date(const QDate &date) const
{
    for(auto it=mDatePatterns.constBegin(); it!=mDatePatterns.constEnd(); it++)
    {
        if((*it).includes(date))
            return true;
    }
    return false;
}

bool clRemSpec_DateSetting::is_precaution_date(const QDate &date, int *Ndays_to_due) const
{
    if(is_empty())
        return false;
    if(mPrecautionDayCount == 0)
        return false;

    int diff = 1;
    QDate d = date.addDays(1);
    for( ; d<=date.addDays(mPrecautionDayCount); d=d.addDays(1))
    {
        if(is_due_date(d))
        {
            if(Ndays_to_due != 0)
                *Ndays_to_due = diff;
            return true;
        }
        diff++;
    }

    return false;
}

bool clRemSpec_DateSetting::includes(const QDate &date, int *Ndays_to_due) const
//Return true if `date` is a due date or a precaution date.
//`*Ndays_to_due` will be 0 if `date` is a due date.
{
    if(is_due_date(date))
    {
        if(Ndays_to_due != 0)
            *Ndays_to_due = 0;
        return true;
    }

    return is_precaution_date(date, Ndays_to_due);
}
