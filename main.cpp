#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationVersion("Alpha 3");
    MainWindow w;
    w.show();
    
    return a.exec();
}
