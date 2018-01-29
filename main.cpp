#include <QApplication>
#include "UI_MainWindow.h"
#include "Control.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //
    //clUI_MainWindow MainWindow;
    clControl Control;


    //
    Control.start_up();
    //MainWindow.show();
    return a.exec();
}
