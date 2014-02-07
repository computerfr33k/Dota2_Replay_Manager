#include "thread.h"

#include <QTimer>

void Thread::run()
{
    qDebug()<<"From work thread: "<< currentThreadId();
    QTimer timer;
    timer.start(1000);
    exec();
}
