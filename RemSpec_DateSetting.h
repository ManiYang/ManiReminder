#ifndef REM_SPEC_DATE_SETTING_H
#define REM_SPEC_DATE_SETTING_H

#include <QObject>
#include <QDomElement>
#include <QDomDocument>
#include "DataElem_DatePattern.h"
typedef clDataElem_DatePattern clDatePattern;

class clRemSpec_DateSetting : public QObject
{
    Q_OBJECT
public:
    explicit clRemSpec_DateSetting(QObject *parent = nullptr);

    void clear();

    bool parse_and_set(QDomElement &spec_setting, QString &parse_error_msg);
         //Return false iff there's parsing error, in which case the error message will
         //be copied to `parse_error_msg`.

    //
    bool is_due_date(const QDate &date) const;
    bool is_precaution_date(const QDate &date, int *Ndays_to_due = 0) const;
    bool includes(const QDate &date, int *Ndays_to_due = 0) const;
                  //Return true if `date` is a due date or a precaution date.
                  //`*Ndays_to_due` will be 0 if `date` is a due date.

    QList<QDate> get_due_dates_within(const QDate &d0, const QDate &d1) const;
    int get_precaution_day_counts() const; //`*this` cannot be empty

    //
    bool is_empty() const { return mDatePatterns.isEmpty(); }

    QString print() const;
    void add_to_XML(QDomDocument &doc, QDomElement &parent) const;

    bool operator == (const clRemSpec_DateSetting &another) const;
    void operator = (const clRemSpec_DateSetting &another);

private:
    QList<clDatePattern> mDatePatterns; //contains the deadline/due dates
    int mPrecautionDayCount; //must be >= 0 (meaningless if `mDatePatterns[]` is empty)
};

#endif // REM_SPEC_DATE_SETTING_H
