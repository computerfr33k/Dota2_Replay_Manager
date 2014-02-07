#include "http.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include "thread.h"

Http::Http(QObject *parent) : QObject(parent), downloadCount(0), totalCount(0)
{
    manager = new QNetworkAccessManager(this);
}
Http::~Http()
{
    delete manager;
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

void Http::setRawHeader(QByteArray headers, QByteArray headerValue)
{
    rawHeaders = headers;
    rawHeaderValue = headerValue;
}

QString Http::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    return basename;
}

bool Http::isFinished()
{
    return downloadQueue.isEmpty();
}

void Http::startNextDownload()
{
    if(downloadQueue.isEmpty())
    {
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();

    QString filename = saveFileName(url);
    //if filename is equal to the php script filename, then we know it is the match info in json. So add json as the file extension
    if(filename.compare("json-mashape.php") == 0)
        filename = QUrlQuery(url).queryItemValue("match_id") + QString(".json");
    else if(filename.compare("_sb.png") == 0)
        startNextDownload();

    output.setFileName("downloads/" + filename);
    if(QFileInfo(output.fileName()).lastModified().addDays(14) > QDateTime::currentDateTime()) //file is not older than 2 weeks, do not bother updating it.
    {
        startNextDownload();
        return;
    }

    if(!output.open(QIODevice::WriteOnly))
    {
        fprintf(stderr, "Problem Opening save file %s for download %s: %s\n", qPrintable(filename), url.toEncoded().constData(), qPrintable(output.errorString()));

        startNextDownload();
        return;
    }

    QNetworkRequest request(url);

    //only apply api key header to requests that are going to the API
    if(url.host().compare("computerfr33k-dota-2-replay-manager.p.mashape.com") == 0)
        request.setRawHeader(rawHeaders, rawHeaderValue);

    currentDownload = manager->get(request);
    connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

    //printf("Downloading %s...\n", url.toEncoded().constData());
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
    }
    else
    {
        ++downloadCount;
    }

    currentDownload->close();
    currentDownload->deleteLater();
    currentDownload = NULL;
    startNextDownload();
}

void Http::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}
