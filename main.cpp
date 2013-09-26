#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("Computerfr33k");
    QApplication::setApplicationName("Dota 2 Replay Manager");
    QApplication::setApplicationVersion("Beta 1");
    MainWindow w;
    w.show();
    
    return a.exec();
}
