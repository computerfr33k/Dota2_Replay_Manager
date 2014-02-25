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

    //get player names
    QVector<QVector<QString> > getPlayerNames();
    QVector<QVector<QString> > getPlayerLevel();
    QVector<QVector<QJsonObject> > getPlayerHeroName();
    QVector<QVector<QString> > getPlayerKills();
    QVector<QVector<QString> > getPlayerDeaths();
    QVector<QVector<QString> > getPlayerAssists();
    QVector<QVector<QVector<QString> > > getPlayerItems();
    QVector<QVector<QString> > getPlayerGold();
    QVector<QVector<QString> > getPlayerLH();
    QVector<QVector<QString> > getPlayerDN();
    QVector<QVector<QString> > getPlayerGPM();
    QVector<QVector<QString> > getPlayerXPM();

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

    QVector<QVector<QString> > playerNames;
    QVector<QVector<QString> > playerLevel;
    QVector<QVector<QJsonObject> > playerHeroName;          //use JsonObject because then we can get name and localized name
    QVector<QVector<QString> > playerKills;
    QVector<QVector<QString> > playerDeaths;
    QVector<QVector<QString> > playerAssists;
    QVector<QVector<QVector<QString> > > playerItems;       //3-D Array; [team][player_slot][item_id]
    QVector<QVector<QString> > playerGold;
    QVector<QVector<QString> > playerLH;
    QVector<QVector<QString> > playerDN;
    QVector<QVector<QString> > playerGPM;
    QVector<QVector<QString> > playerXPM;

    //internal funcion(s)
    void downloadImages();
};

#endif // MATCHINFO_H
