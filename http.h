/*
 * Network Class For D2RM.
 *
 */

#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include <QtNetwork>

class Http : public QObject
{
    Q_OBJECT
public:
    explicit Http(QObject *parent = 0);
    ~Http();
    QByteArray get();
    void close();

signals:

public slots:

private slots:
    void downloadReadyRead(QNetworkReply *);

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;

};

#endif // HTTP_H
