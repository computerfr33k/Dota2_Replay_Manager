#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QtGui>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = 0);
    void setUrl(QUrl);
    void start();

private:
    QUrl url;
    QNetworkAccessManager *manager;
    QNetworkReply reply;
    
signals:
    
public slots:
    
};

#endif // UPDATER_H
