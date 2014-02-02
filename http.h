#ifndef HTTP_H
#define HTTP_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QTime>
#include <QUrl>
#include <QtNetwork>
#include <QProgressDialog>
#include <QThread>

class Http : public QObject
{
    Q_OBJECT
public:
    Http(QObject *parent = 0);
    ~Http();

    void append(const QUrl &url);
    void append(const QStringList &urlList);
    void setRawHeader(QByteArray &header, QByteArray &headerValue);
    QString saveFileName(const QUrl &url);

signals:
    void finished();

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    QNetworkAccessManager *manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply *currentDownload;
    QByteArray rawHeaders, rawHeaderValue;
    QFile output;
    QTime downloadTime;
    QProgressDialog progressDialog;

    int downloadCount;
    int totalCount;
};

#endif
