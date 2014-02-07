#include "mainwindow.h"
#include "thread.h"
#include "http.h"
#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("Computerfr33k");
    QApplication::setApplicationName("Dota 2 Replay Manager");
    QApplication::setApplicationVersion("Beta 2");
    MainWindow w;
    w.show();
    
    return a.exec();
}
