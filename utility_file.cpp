#include <QSaveFile>
#include "utility_file.h"

bool write_to_file(const QString &fname, const QString &data, QString &error_msg)
//output encodeing: UTF8
{
    QSaveFile F(fname);
    if(!F.open(QIODevice::WriteOnly))
    {
        error_msg = QString("Could not open \"%1\" for writing.").arg(fname);
        return false;
    }

    F.write(data.toUtf8());
    bool ch = F.commit();

    if(!ch)
    {
        error_msg = QString("Error while writing to \"%1\".\n\n%2")
                    .arg(fname, F.errorString());
    }

    return ch;
}
