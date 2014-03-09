#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>

class Thread : public QThread
{
    Q_OBJECT
private:
    void run();

public:

signals:

public slots:

};

#endif // THREAD_H
