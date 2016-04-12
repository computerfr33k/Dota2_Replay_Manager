#include "mainwindow.h"
#include "thread.h"
#include "http.h"
#include "firstrun.h"
#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("Computerfr33k");
    QApplication::setApplicationName("Dota 2 Replay Manager");
    QApplication::setApplicationVersion("Beta 3");

    MainWindow w;
    w.show();

    //use settings to determine if this is the first Run.
    /*
    QSettings settings(QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0) + "/settings.ini", QSettings::IniFormat);
    FirstRun firstRun;
    //if this is the first time the user uses this program, then display the Tutorial.
    if(settings.value("firstRun", true).toBool())
    {
        firstRun.show();
        settings.setValue("firstRun", false);
        settings.sync();
    }
    */

    return a.exec();
}
