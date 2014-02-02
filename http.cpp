#include "http.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <stdio.h>

Http::Http(QObject *parent) : QObject(parent), downloadCount(0), totalCount(0)
{
    manager = new QNetworkAccessManager(this);
}
Http::~Http()
{
    delete manager;
    delete currentDownload;
}

void Http::append(const QUrl &url)
{
    if(downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    downloadQueue.enqueue(url);
    ++totalCount;
}

void Http::append(const QStringList &urlList)
{
    foreach(QString url, urlList)
        append(QUrl::fromEncoded(url.toLocal8Bit()));

    if(downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void Http::setRawHeader(QByteArray &headers, QByteArray &headerValue)
{
    rawHeaders = headers;
    rawHeaderValue = headerValue;
}

QString Http::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if(QFile::exists(basename))
    {
        //already exists, don't overwrite
        int i=0;
        basename += '.';
        while(QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }
    return basename;
}

void Http::startNextDownload()
{
    if(downloadQueue.isEmpty())
    {
        printf("%d/%d files downloaded successfully\n", downloadCount, totalCount);
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();

    QString filename = saveFileName(url);
    qDebug() << filename.compare("json-mashape.php");
    if(filename.compare("json-mashape.php") == 0)
        filename = QUrlQuery(url).queryItemValue("match_id") + QString(".json");

    qDebug() << filename;
    output.setFileName(filename);
    if(!output.open(QIODevice::WriteOnly))
    {
        fprintf(stderr, "Problem Opening save file %s for download %s: %s\n", qPrintable(filename), url.toEncoded().constData(), qPrintable(output.errorString()));

        startNextDownload();
        return;
    }

    QNetworkRequest request(url);
    request.setRawHeader(rawHeaders, rawHeaderValue);
    currentDownload = manager->get(request);
    connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTime.start();
}

void Http::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
}

void Http::downloadFinished()
{
    output.close();

    if(currentDownload->error())
    {
        progressDialog.close();
    }
    else
    {
        ++downloadCount;
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void Http::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}
