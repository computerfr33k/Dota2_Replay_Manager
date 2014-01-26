#include "http.h"

Http::Http(QObject *parent) :
    QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}
Http::~Http()
{
    if(manager != NULL)
    {
        delete manager;
        manager = NULL;
    }
}

QByteArray Http::get()
{
    return QByteArray();
}

void Http::close()
{
}

void Http::downloadReadyRead(QNetworkReply *reply)
{
    reply->readAll();
}
