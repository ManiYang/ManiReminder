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
    if(! dir.isReadable())
        exit_on_error(QString("Directory \"%1\" is not readable.").arg(mDataDir));
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

bool clFileReadWrite::save_overdue_tasks(const QList<clUtil_Task> &overdue_tasks,
                                         const QDate &last_traced_day)
{
    const QString fname = mDataDir+"overdue_reminders.txt";
    QFile F(fname);
    if(! F.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "warning",
                             QString("Could not open \"%1\" for writing.").arg(fname));
        return false;
    }
    else
    {
        QTextStream ts(&F);

        ts << last_traced_day.toString("yyyy/M/d") << '\n';

        for(auto it=overdue_tasks.cbegin(); it!=overdue_tasks.cend(); it++)
        {
            ts << (*it).mRemID << ' '
               << (*it).mDeadline.toString("yyyy/M/d") << '\n';
        }
    }

    F.close();
    return true;
}

void clFileReadWrite::read_overdue_tasks(QList<clUtil_Task> &overdue_reminders,
                                         QDate &last_traced_day)
{
    const QString fname = mDataDir+"overdue_reminders.txt";
    QFile F(fname);
    if(! F.open(QIODevice::ReadOnly))
        exit_on_error(QString("Could not open \"%1\" for reading.").arg(fname));

    //
    QString first_line = QString::fromUtf8(F.readLine());
    last_traced_day = QDate::fromString(first_line.simplified(), "yyyy/M/d");
    if(! last_traced_day.isValid())
        exit_on_error(QString("Error in reading \"%1\".\n\n"
                              "Could not parse the first line \"%2\" at a date.")
                      .arg(fname, first_line));

    //
    overdue_reminders.clear();
#define ON_ERROR exit_on_error(QString("Error in reading \"%1\".\n\n" \
                                       "Could not parse the line \"%2\".").arg(fname, line))
    while(! F.atEnd())
    {
        QByteArray line_bytes = F.readLine().simplified();
        if(line_bytes.isEmpty())
            continue;

        QString line = QString::fromUtf8(line_bytes);
        QStringList tokens = line.split(' ');
        if(tokens.size() != 2)
            ON_ERROR;

        bool ok;
        int id = tokens.at(0).toInt(&ok);
        if(!ok)
            ON_ERROR;

        QDate due_date = QDate::fromString(tokens.at(1).simplified(), "yyyy/M/d");
        if(! due_date.isValid())
            ON_ERROR;

        overdue_reminders << clUtil_Task(id, due_date);
    }
#undef ON_ERROR

    //
    F.close();
}
void clFileReadWrite::read_rem_deadlines_states_in_history(const int id,
                                         const QDate &deadline_min, const QDate &deadline_max,
                                         QMap<QDate,clDataElem_TaskState> &deadline_states)
//Read the reminder-deadline state history to get the recorded tasks (and their latest states)
//involving reminder `id` with deadline within [`deadline_min`, `deadline_max`].
{
    Q_ASSERT(deadline_min.isValid() && deadline_max.isValid());
    Q_ASSERT(deadline_min <= deadline_max);

    deadline_states.clear();

    //
    const QString fname = mDataDir + "task_state_history/"
                          + QString("%1.txt").arg(id,4,10,QChar('0'));
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        return;

    //
    QByteArray line_bytes = F.readLine().simplified();
    QString line = QString::fromUtf8(line_bytes);
    QDate deadline;
#define ON_ERROR exit_on_error(QString("Error in reading \"%1\".\n\nCould not parse \"%2\".") \
                               .arg(fname, line))
    while(! line.isEmpty())
    {
        QDate d;
        clDataElem_TaskState state;

        if(line.startsWith('D'))
        {
            deadline = QDate::fromString(line.mid(1), "yyyy/M/d");
            if(! deadline.isValid())
                ON_ERROR;

            if(deadline > deadline_max)
                break;
        }
        else if(deadline >= deadline_min)
        {
            QStringList tokens = line.split(' ');
            if(tokens.size() != 2)
                ON_ERROR;

            d = QDate::fromString(tokens.at(0), "yyyy/M/d");
            if(!d.isValid())
                ON_ERROR;

            bool ch = state.parse_and_set(tokens.at(1));
            if(!ch)
                ON_ERROR;
        }

        // read next line
        line_bytes = F.readLine().simplified();
        line = QString::fromUtf8(line_bytes);

        if(line.startsWith('D') || line.isEmpty())
        {
            //next line starts with 'D' or is empty (last line)
            // => `state` (if set) is the final state

            if(d.isValid()) //(`d` and `state` is set)
                deadline_states.insert(deadline, state);
        }
    }
#undef ON_ERROR

    //
    F.close();
}


static int find_first_greater_or_equal(const QList<QDate> dates, const int start_index,
                                       const QDate &wrt_date)
//Find first element of `dates[]` that is >= `wrt_date`, starting from index `start_index`.
//Return the index. If not found, return -1.
//`start_index` can have any value.
{
    if(start_index >= dates.size())
        return -1;

    int i = start_index;
    if(i < 0)
        i = 0;
    for( ; i<dates.size(); i++)
    {
        if(dates.at(i) >= wrt_date)
            return i;
    }
    return -1;
}

void clFileReadWrite::read_rem_deadline_states(
                              const int id,
                              //
                              const QList<QDate> &query_deadlines, //must be in ascending order
                              QList<clDataElem_TaskState> &state_of_query_deadlines,
                              //
                              const QDate &max_shfited_date,
                              QList<QDate> &deadlines_w_date_shifted,
                              QList<QDate> &shifted_dates,
                              //
                              const QDate &record_date,
                              QList<QDate> &deadlines_w_record_on_date,
                              QList<clDataElem_TaskState> &states_on_date)
// * `state_of_query_deadlines[i]` will be the latest state of the deadline
//   `query_deadlines[i]`.
//
// * `deadlines_w_date_shifted[]` will be the deadlines with latest state = `DateShifted`
//   and shifted-date <= `max_shfited_date`. `shifted_dates[j]` will be the shifted date of
//   `deadlines_w_date_shifted[j]`.
//
// * `deadlines_w_record_on_date[]` will be the deadlines with a state update record on
//   `record_date`. `states[k]` will be the updated state of `deadlines_w_record_on_date[k]`
//   on that date.
{
    state_of_query_deadlines.clear();
    for(int i=0; i<query_deadlines.size(); i++)
        state_of_query_deadlines << clDataElem_TaskState();

    deadlines_w_date_shifted.clear();
    shifted_dates.clear();
    deadlines_w_record_on_date.clear();
    states_on_date.clear();

    //
    const QString fname = mDataDir + "task_state_history/"
                          + QString("%1.txt").arg(id,4,10,QChar('0'));
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        return;

    //
    int query_deadlines_search_from = 0;

    // first line (first deadline)
    QByteArray line_bytes = F.readLine().simplified();
    if(line_bytes.isEmpty())
        return;
    QString line = QString::fromUtf8(line_bytes);
    Q_ASSERT(line.startsWith('D'));

    QDate deadline1 = QDate::fromString(line.mid(1), "yyyy/M/d");
    Q_ASSERT(deadline1.isValid());

    //
    const QString record_date_str = record_date.toString("yyyy/M/d");
    QString date_str1, state_str1;
    while(! line.isEmpty())
    {
        if(line.startsWith('D'))
        {
            deadline1 = QDate::fromString(line.mid(1), "yyyy/M/d");
            Q_ASSERT(deadline1.isValid());
        }
        else
        {
            QStringList tokens = line.split(' ');
            date_str1 = tokens.at(0);
            state_str1 = tokens.at(1);

            //
            if(date_str1 == record_date_str)
            {
                deadlines_w_record_on_date << deadline1;

                clDataElem_TaskState state;
                bool ch = state.parse_and_set(state_str1);
                Q_ASSERT(ch);
                states_on_date << state;
            }
        }

        // read next line
        line_bytes = F.readLine().simplified();
        line = QString::fromUtf8(line_bytes);

        if(line.startsWith('D') || line.isEmpty())
        {
            // (now is at the end of a deadline block, `state_str1` is the latest state
            //  of `deadline1`)

            clDataElem_TaskState state1;
            bool ch = state1.parse_and_set(state_str1);
            Q_ASSERT(ch);

            // search `query_deadlines[]` for `deadline1`
            int pos = find_first_greater_or_equal(query_deadlines,
                                                  query_deadlines_search_from, deadline1);
            if(pos == -1)
                query_deadlines_search_from = query_deadlines.size();
            else
            {
                query_deadlines_search_from = pos;

                //
                if(query_deadlines.at(pos) == deadline1) //(found at `pos`)
                {
                    //set `state_of_query_deadlines[pos]` to `state1`
                    state_of_query_deadlines[pos] = state1;
                }
            }

            //
            if(state1.get_state() == clDataElem_TaskState::DateShifted)
            {                                    //(`deadline1` has latest state `DateShifted`)
                QDate shifted_date = state1.get_shifted_date();
                if(shifted_date <= max_shfited_date)
                {
                    deadlines_w_date_shifted << deadline1;
                    shifted_dates << shifted_date;
                }
            }
        }
    }

    //
    F.close();
}

void clFileReadWrite::add_task_state_update(const clUtil_Task &task,
                                            const clDataElem_TaskState &new_state,
                                            const QDate &update_date)
{
    const int id = task.mRemID;
    const QDate deadline = task.mDeadline;

    // read whole file --> lines[]
    QList<QByteArray> lines;

    const QString fname = mDataDir + "task_state_history/"
                          + QString("%1.txt").arg(id,4,10,QChar('0'));
    QFile F(fname);
    bool ch = F.open(QIODevice::ReadOnly);
    if(ch)
    {
        {
            QByteArray bytes = F.readAll();
            lines = bytes.split('\n');
        }
        F.close();
    }

    // write file
    ch = F.open(QIODevice::WriteOnly);
    Q_ASSERT(ch);

    const QString update_date_str = update_date.toString("yyyy/M/d");

    //
    bool found = false; //`deadline` is found?
    bool added = false; //new state written?
    QDate deadline1;
    for(int i=0; i<lines.size(); i++)
    {
        //
        QString line = QString::fromUtf8(lines.at(i));
        if(line.isEmpty())
            continue;

        if(line.startsWith('D'))
        {
            if(found)
            {
                //insert  (date, state)
                QString str = update_date.toString("  yyyy/M/d ");
                str += new_state.print()+'\n';
                F.write(str.toUtf8());

                //
                found = false;
                added = true;
            }

            // update `deadline1`
            deadline1 = QDate::fromString(line.mid(1), "yyyy/M/d");
            if(deadline1 == deadline)
                found = true;
            else if(deadline1 > deadline)
            {
                if(!added)
                {
                    //insert deadline, (date, state)
                    QString str = 'D'+deadline.toString("yyyy/M/d")+'\n';
                    str += update_date.toString("  yyyy/M/d ");
                    str += new_state.print()+'\n';
                    F.write(str.toUtf8());

                    //
                    added = true;
                }
            }

            //
            F.write(lines.at(i)+'\n');
        }
        else
        {
            if(! found)
                F.write(lines.at(i)+'\n');
            else
            {
                if(! line.simplified().startsWith(update_date_str))
                    F.write(lines.at(i)+'\n');
            }
        }
    }

    if(!added)
    {
        //insert deadline, (date, state)
        QString str;
        if(!found)
            str = 'D'+deadline.toString("yyyy/M/d")+'\n';
        str += update_date.toString("  yyyy/M/d ");
        str += new_state.print()+'\n';
        F.write(str.toUtf8());
    }

    //
    Q_ASSERT(F.error() == QFile::NoError);
    F.close();
}

void clFileReadWrite::read_task_day_scheduling(
                                 const QDate &date,
                                 QMap<clTask, QList<clTaskDayScheduleSession> > &TaskSessions)
{
    Q_ASSERT(date.isValid());

    TaskSessions.clear();

    // open file
    const QString fname = mDataDir + QString("task_day_schedule/%1.txt")
                                     .arg(date.toString("yyyyMMdd"));
    QFile F(fname);
    if(!F.open(QIODevice::ReadOnly))
        return;

    // read file
    clTask task;
    QList<clTaskDayScheduleSession> sessions;
    while(true)
    {
        QString line = QString::fromUtf8( F.readLine().simplified() );
        if(line.isEmpty()) //should be the last line of file
        {
            // (now might be at the end of a task block)
            if(task.mRemID >= 0 && !sessions.isEmpty())
                TaskSessions.insert(task, sessions); //add result

            break;
        }

        //
        if(line.startsWith('T'))
        {
            /// (now is at the end of a task block)
            if(task.mRemID >= 0 && !sessions.isEmpty())
            {
                TaskSessions.insert(task, sessions); //add result
                sessions.clear();
            }

            /// start of next task block
            QStringList tokens = line.split(' ');
            Q_ASSERT(tokens.size() == 3);

            bool ok;
            int id = tokens.at(1).toInt(&ok);
            Q_ASSERT(ok);

            QDate deadline = QDate::fromString(tokens.at(2), "yyyy/M/d");
            Q_ASSERT(deadline.isValid());

            //
            task.set(id, deadline);
        }
        else if(line.startsWith('S'))
        {
            clTaskDayScheduleSession session;
            bool ch = session.parse_and_set(line.mid(2));
            Q_ASSERT(ch);

            //
            sessions << session;
        }
    }

    //
    F.close();
}

void clFileReadWrite::save_task_day_scheduling(
                           const QDate &date,
                           const QMap<clTask, QList<clTaskDayScheduleSession> > &TaskSessions)
{
    Q_ASSERT(date.isValid());

    // open file
    const QString fname = mDataDir + QString("task_day_schedule/%1.txt")
                                     .arg(date.toString("yyyyMMdd"));
    QFile F(fname);
    bool ch = F.open(QIODevice::WriteOnly);
    Q_ASSERT(ch);

    // write to file
    bool empty = true;
    for(auto it=TaskSessions.cbegin(); it!=TaskSessions.cend(); it++)
    {
        const clTask &task = it.key();
        const QList<clTaskDayScheduleSession> &sessions = it.value();

        if(sessions.isEmpty())
            continue;
        empty = false;

        //
        QString line = QString("T %1 %2\n").arg(task.mRemID)
                                           .arg(task.mDeadline.toString("yyyy/M/d"));
        F.write(line.toUtf8());

        //
        for(int i=0; i<sessions.size(); i++)
        {
            line = QString("S   %1\n").arg(sessions.at(i).print());
            F.write(line.toUtf8());
        }
    }

    //
    Q_ASSERT(F.error() == QFile::NoError);
    F.close();

    //
    if(empty)
        F.remove();
}
