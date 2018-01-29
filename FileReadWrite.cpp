#include <QFile>
#include <QSaveFile>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QDomDocument>
#include "FileReadWrite.h"
#include "utility_file.h"
#include "utility_DOM.h"

static void exit_on_error(const QString &error_msg)
{
    QMessageBox::critical(nullptr, "error", error_msg);
    exit(EXIT_FAILURE);
}

static QString get_data_dir()
{
    const QString fname = ":/files/data_path.txt";

    // open file
    QFile F(fname);
    bool ch = F.open(QIODevice::ReadOnly);
    if(!ch)
        exit_on_error(QString("Could not open resource file \"%1\".").arg(fname));

    // read file
    QString path;
    while(! F.atEnd())
    {
        QByteArray line_bytes = F.readLine();
        QString line = QString::fromUtf8(line_bytes).simplified();

        if(line.isEmpty()) //(is empty line)
            continue;

        if(line.startsWith('#')) //(is a comment line)
            continue;

        //
        if(! path.isEmpty())
            exit_on_error(QString("File \"%1\" contains more than "
                                  "one (non-empty, non-comment) line.").arg(fname));

        path = line;
    }
    F.close();

    //
    if(path.isNull())
        exit_on_error(QString("Could not find any data in resource file \"%1\".").arg(fname));

    if(! path.endsWith('/'))
        path += '/';

    return path;
}

clFileReadWrite::clFileReadWrite()
{
    // get data dir --> 'mDataDir'
    mDataDir = get_data_dir();

    // check existence
    QDir dir(mDataDir);
    if(! dir.exists())
        exit_on_error(QString("Directory \"%1\" does not exist.").arg(mDataDir));
}

QList<clReminder *> clFileReadWrite::read_all_reminders()
//The returned `clReminder` objects are created and are not delete here.
{
    QDir Dir(mDataDir + "reminders/");
    if(! Dir.exists())
        exit_on_error(QString("Directory \"%1\" does not exist.").arg(mDataDir));

    // get .xml files in the directory `Dir`
    QFileInfoList Files = Dir.entryInfoList(QStringList() << "*.xml");

    // read the .xml files
    QList<clReminder *> Reminders;
    foreach(QFileInfo file, Files)
    {
        const QString fname = file.absoluteFilePath();

        int id;
        {
            QString s = file.fileName();
            s.chop(4);
            bool ok;
            id = s.toInt(&ok);
            if(! ok)
                exit_on_error(QString("Unrecognized file-name format: \"%1\" .").arg(fname));
        }

        // open file
        QFile F(fname);
        if(! F.open(QIODevice::ReadOnly))
            exit_on_error(QString("Could not open file \"%1\".").arg(fname));

        // parse as XML --> `doc`
        QDomDocument doc;
        {
            QString err_msg;
            int err_line;
            bool ch = doc.setContent(&F, &err_msg, &err_line);
            if(!ch)
                exit_on_error(QString("Error in parsing \"%1\" as XML."
                                      "\n\nline: %2"
                                      "\n\n%3").arg(fname).arg(err_line).arg(err_msg));
        }
        F.close();

        // parse as reminder --> `R`
        QDomElement root = doc.documentElement();
        if(root.tagName() != "reminder")
            exit_on_error(QString("The root of file \"%1\" is not \"<reminder>\".").arg(fname));

        clReminder *R = new clReminder(id);
        bool ch = R->parse_and_set(root);
        if(! ch)
            exit_on_error(QString("Failed to parse reminder setting.\n\n"
                                  "File: \"%1\"\n\n"
                                  "%2").arg(fname).arg(R->parse_error_message));

        //
        Reminders << R;
    }

    //
    return Reminders;
}

QSet<QString> clFileReadWrite::read_all_situations()
{
    QString fname = mDataDir+"situations.txt";
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        exit_on_error(QString("Could not open \"%1\" for reading.").arg(fname));
    QByteArray bytes = F.readAll();
    F.close();

    QString S = QString::fromUtf8(bytes);
    QStringList tokens = S.split('\n', QString::SkipEmptyParts);
    return tokens.toSet();
}

QSet<QString> clFileReadWrite::read_all_events()
{
    QString fname = mDataDir+"events.txt";
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        exit_on_error(QString("Could not open \"%1\" for reading.").arg(fname));
    QByteArray bytes = F.readAll();
    F.close();

    QString S = QString::fromUtf8(bytes);
    QStringList tokens = S.split('\n', QString::SkipEmptyParts);
    return tokens.toSet();
}

QMultiMap<QDateTime,clGEvent> clFileReadWrite::read_gevent_history()
{
    QString fname = mDataDir+"gevent_history.txt";
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        exit_on_error(QString("Could not open \"%1\" for reading.").arg(fname));

    //
    QByteArray bytes = F.readAll();
    F.close();

    QString S = QString::fromUtf8(bytes);
    QStringList Lines = S.split('\n', QString::SkipEmptyParts);

    QMultiMap<QDateTime,clGEvent> history;
    bool ok = true;
    foreach(QString line, Lines)
    {
        QStringList tokens = line.split("; ");
        if(tokens.size() != 2)
        {
            ok = false;
            break;
        }

        QDateTime t = QDateTime::fromString(tokens[0], "yyyy/M/d.hh:mm:ss");
        if(t.isNull())
        {
            ok = false;
            break;
        }

        clGEvent gevent;
        bool ch = gevent.parse_and_set(tokens[1]);
        if(!ch)
        {
            ok = false;
            break;
        }

        //
        history.insert(t, gevent);
    }

    if(!ok)
        exit_on_error(QString("Unrecognized contents of \"%1\" .").arg(fname));

    //
    return history;
}

QMap<int,clDataElem_ScheduleStatus> clFileReadWrite::read_day_planning_status(
                                                                            const QDate &date)
//Return empty map if not found or there's error.
{
    // open file
    QString fname = mDataDir+"day_plan/"+date.toString("yyyyMMdd.xml");
    QFile F(fname);
    if(! F.open(QIODevice::ReadOnly))
        return QMap<int,clDataElem_ScheduleStatus>();

    // read as XML
    QDomDocument doc;
    QString error_msg;
    int error_line;
    bool ch = doc.setContent(&F, &error_msg, &error_line);
    if(!ch)
    {
        QString msg = QString("Failed to read \"%1\" as XML.\n\nLine %2: %3")
                     .arg(fname).arg(error_line).arg(error_msg);
        QMessageBox::warning(0, "warning", msg);
        return QMap<int,clDataElem_ScheduleStatus>();
    }

    // parse
    // <root>
    //   <schedule-status reminder="rem_id"> scheduling status </schedule-status>
    //   <schedule-status reminder="rem_id"> scheduling status </schedule-status>
    //   ...
    // </root>
    QMap<int,clDataElem_ScheduleStatus> status;

    QDomElement root = doc.firstChildElement();
    for(QDomElement e = root.firstChildElement("schedule-status");
        ! e.isNull();
        e = e.nextSiblingElement("schedule-status"))
    {
        // get reminder id --> `id`
        if(! e.hasAttribute("reminder"))
        {
            QString msg = QString("Failed to parse \"%1\".\n\n"
                                  "Attribute \"reminder\" not found in <schedule-status>.")
                         .arg(fname);
            QMessageBox::warning(0, "warning", msg);
            return QMap<int,clDataElem_ScheduleStatus>();
        }

        QString attr_value = e.attribute("reminder");
        int id = attr_value.toInt(&ch);
        if(!ch)
        {
            QString msg = QString("Failed to parse \"%1\".\n\n"
                                  "Unrecognized value \"%2\" of attribute \"reminder\" "
                                  "in <schedule-status>.")
                         .arg(fname, attr_value);
            QMessageBox::warning(0, "warning", msg);
            return QMap<int,clDataElem_ScheduleStatus>();
        }

        // get scheduling status --> `SS`
        QString text;
        ch = nsDomUtil::is_an_element_containing_text(e, nullptr, &text);
        Q_ASSERT(ch);

        clDataElem_ScheduleStatus SS;
        ch = SS.parse_and_set(text);
        if(!ch)
        {
            QString msg = QString("Failed to parse \"%1\".\n\n"
                                  "Could not parse \"%2\" as scheduling status.")
                         .arg(fname, text);
            QMessageBox::warning(0, "warning", msg);
            return QMap<int,clDataElem_ScheduleStatus>();
        }

        //
        status.insert(id, SS);
    }

    //
    F.close();
    return status;
}

bool clFileReadWrite::save_day_planning_status(
                                         const QDate &date,
                                         const QMap<int,clDataElem_ScheduleStatus> &status)
//Unscheduled items will be ignored.
//Remove the file if `status` is empty or every `status[]` is "unscheduled".
{
    const QString fname = mDataDir+"day_plan/"+date.toString("yyyyMMdd.xml");

    //
    bool remove_file = false;
    if(status.isEmpty())
        remove_file = true;
    else
    {
        remove_file = true;
        QList<clDataElem_ScheduleStatus> SSs = status.values();
        for(auto it=SSs.begin(); it!=SSs.end(); it++)
        {
            if((*it).get_status() != clDataElem_ScheduleStatus::Unscheduled)
            {
                remove_file = false;
                break;
            }
        }
    }

    if(remove_file)
    {
        QFileInfo FI(fname);
        if(FI.exists())
        {
            QFile F(fname);
            bool ch = F.remove();
            if(!ch)
            {
                QString msg = QString("Could not delete file \"%1\".").arg(fname);
                QMessageBox::warning(0, "warning", msg);
                return false;
            }
        }

        return true;
    }

    // build document
    QDomDocument doc;
    QDomElement root = doc.createElement("day-planning-status");
    doc.appendChild(root);

    for(auto it=status.constBegin(); it!=status.constEnd(); it++)
    {
        const int &id = it.key();
        const clDataElem_ScheduleStatus &SS = it.value();

        if(SS.get_status() == clDataElem_ScheduleStatus::Unscheduled)
            continue;

        //
        QDomElement e = doc.createElement("schedule-status");
        e.setAttribute("reminder", id);

        QDomText text_node = doc.createTextNode(SS.print());
        e.appendChild(text_node);

        //
        root.appendChild(e);
    }

    // open file
    QFile F(fname);
    if(! F.open(QIODevice::WriteOnly))
    {
        QString msg = QString("Could not open \"%1\" for writing.").arg(fname);
        QMessageBox::warning(0, "warning", msg);
        return false;
    }

    // write to file
    QTextStream TS(&F);
    TS << doc.toString(2);

    //
    if(F.error() != QFile::NoError)
    {
        F.close();
        return false;
    }

    F.close();
    return true;
}

bool clFileReadWrite::save_reminder(const clReminder &reminder, QString &error_msg)
{
    // create XML document --> `doc`
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml",
                                                    "version=\"1.0\" encoding=\"UTF-8\""));
    QDomElement root = doc.createElement("reminder");
    doc.appendChild(root);

    reminder.add_to_XML(doc, root);

    //
    QString data = doc.toString(2);

    QString fname = mDataDir+"reminders/%1.xml";
    fname = fname.arg(reminder.get_id(), 4, 10, QChar('0'));

    bool ch = write_to_file(fname, data, error_msg);
    return ch;
}

bool clFileReadWrite::delete_reminder(const int id)
//return true iff done
{
    QString dir_name = mDataDir+"reminders/";
    QString fname = QString("%1.xml").arg(id, 4, 10, QChar('0'));

    QFileInfo F(dir_name+fname);
    Q_ASSERT(F.exists());

    QDir D = F.absoluteDir();
    bool ch = D.remove(fname);
    if(!ch)
    {
        QString msg = "Could not delete file \"%1\" .";
        msg = msg.arg(dir_name+fname);
        QMessageBox::warning(Q_NULLPTR, "error", msg);
    }

    return ch;
}

bool clFileReadWrite::save_situations(const QSet<QString> &situations)
{
    QString Str = QStringList(situations.toList()).join('\n') + '\n';

    QString fname = mDataDir+"situations.txt";
    QString error_msg;
    bool ch = write_to_file(fname, Str, error_msg);

    if(!ch)
        QMessageBox::warning(nullptr, "error", error_msg);
    return ch;
}

bool clFileReadWrite::save_events(const QSet<QString> &events)
{
    QString Str = QStringList(events.toList()).join('\n') + '\n';

    QString fname = mDataDir+"events.txt";
    QString error_msg;
    bool ch = write_to_file(fname, Str, error_msg);

    if(!ch)
        QMessageBox::warning(nullptr, "error", error_msg);
    return ch;
}

bool clFileReadWrite::save_gevent_history(const QMultiMap<QDateTime, clGEvent> &history)
{
    QString S;
    for(auto it=history.constBegin(); it!=history.constEnd(); it++)
    {
        S += it.key().toString("yyyy/M/d.hh:mm:ss");
        S += "; ";
        S += it.value().print();
        S += '\n';
    }

    QString fname = mDataDir+"gevent_history.txt";
    QString error_msg;
    bool ch = write_to_file(fname, S, error_msg);

    if(!ch)
        QMessageBox::warning(nullptr, "error", error_msg);
    return ch;
}
