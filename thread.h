#ifndef THREAD_H
#define THREAD_H

#include <QObject>

class Thread : public QObject
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = 0);

signals:

public slots:

};

#endif // THREAD_H
