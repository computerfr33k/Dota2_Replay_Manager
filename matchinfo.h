#ifndef MATCHINFO_H
#define MATCHINFO_H

#include <QApplication>
#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include "http.h"

class matchInfo : public QObject
{
    Q_OBJECT
public:
    explicit matchInfo(QObject *parent = 0);
    void parse(const QString& filename);

    //get basic match info
    QString getMatchID();
    QString getGameMode();
    QString getStartTime();
    QString getLobbyType();
    QString getDuration();
    QString getFirstBloodTime();
    QString getMatchWinner();

    //get picks & bans
    QVector<QVector<QString> > getPicks();
    QVector<QVector<QString> > getBans();

signals:

public slots:

private:
    QJsonDocument json;
    //url used for the base of image downloads
    QString baseUrl;

    //basic match Info variables
    QString matchID;
    QString gameMode;
    QString startTime;
    QString lobbyType;
    QString duration;
    QString firstBloodTime;

    //Picks & Bans
    //2-D array, 1st for team; 0 = radiant, 1 = dire; 2nd for which pick/ban
    QVector<QVector<QString> > picks;
    QVector<QVector<QString> > bans;

    //internal funcion(s)
    void downloadImages();
};

#endif // MATCHINFO_H
