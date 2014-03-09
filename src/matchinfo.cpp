#include "matchinfo.h"

matchInfo::matchInfo(QObject *parent) :
    QObject(parent)
{
    //size our array accordingly
    bans.resize(2);
    picks.resize(2);
    playerNames.resize(2);
    playerLevel.resize(2);
    playerHeroName.resize(2);
    playerKills.resize(2);
    playerDeaths.resize(2);
    playerAssists.resize(2);
    playerItems.resize(2);
    playerGold.resize(2);
    playerLH.resize(2);
    playerDN.resize(2);
    playerGPM.resize(2);
    playerXPM.resize(2);

    for(int i=0; i<2; i++)
    {
        bans[i].resize(5);
        picks[i].resize(5);
        playerNames[i].resize(5);
        playerLevel[i].resize(5);
        playerHeroName[i].resize(5);
        playerKills[i].resize(5);
        playerDeaths[i].resize(5);
        playerAssists[i].resize(5);
        playerItems[i].resize(5);
        for(int j=0; j<5; j++)
            playerItems[i][j].resize(6);

        playerGold[i].resize(5);
        playerLH[i].resize(5);
        playerDN[i].resize(5);
        playerGPM[i].resize(5);
        playerXPM[i].resize(5);
    }

    baseUrl = "http://media.steampowered.com/apps/dota2/images/";
}

void matchInfo::parse(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        //qDebug() << "Could Not Open file: " << filename;
        return;
    }

    json = QJsonDocument::fromJson(file.readAll());

    //parse basic match info
    matchID = json.object().value("match_id").toString();
    gameMode = json.object().value("game_mode").toString();
    startTime = json.object().value("start_time").toString();
    lobbyType = json.object().value("lobby_type").toString();
    duration = json.object().value("duration").toString();
    firstBloodTime = json.object().value("first_blood_time").toString();

    //parse picks & bans if CM
    if(gameMode.compare("Captains Mode") == 0)
    {
        for(int i=0; i < 2; i++)
        {
            for(int j=0; j < 5; j++)
            {
                if(i == 0)
                {
                    bans[i][j] = json.object().value("picks_bans").toObject().value("radiant").toObject().value("bans").toArray().at(j).toObject().value("name").toString();
                    picks[i][j] = json.object().value("picks_bans").toObject().value("radiant").toObject().value("picks").toArray().at(j).toObject().value("name").toString();
                }
                else
                {
                    bans[i][j] = json.object().value("picks_bans").toObject().value("dire").toObject().value("bans").toArray().at(j).toObject().value("name").toString();
                    picks[i][j] = json.object().value("picks_bans").toObject().value("dire").toObject().value("picks").toArray().at(j).toObject().value("name").toString();
                }
            }
        }
    }

    //used for holding either 'dire' or 'radiant' depending on which part of the for loop we are in.
    QString team;
    for(int i=0; i<2; i++)
        for(int j=0; j<5; j++)
        {
            team = (i == 0) ? "radiant" : "dire";       //if else statement, just in a single line.
            playerNames[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("account_name").toString();
            playerLevel[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("level").toString();
            playerHeroName[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("hero").toObject();
            playerKills[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("kills").toString();
            playerDeaths[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("deaths").toString();
            playerAssists[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("assists").toString();
            playerGold[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("gold_spent").toString();
            playerLH[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("last_hits").toString();
            playerDN[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("denies").toString();
            playerGPM[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("gold_per_min").toString();
            playerXPM[i][j] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("xp_per_min").toString();

            //parse items into the array (QVector)
            for(int k=0; k<6; k++)
            {
                playerItems[i][j][k] = json.object().value("slots").toObject().value(team).toArray().at(j).toObject().value("item_" + QString::number(k) ).toString();
            }
        }

    //Now Download Images since we are done parsing
    downloadImages();
}

QString matchInfo::getMatchID()
{
    return matchID;
}

QString matchInfo::getGameMode()
{
    return gameMode;
}

QString matchInfo::getStartTime()
{
    return startTime;
}

QString matchInfo::getLobbyType()
{
    return lobbyType;
}

QString matchInfo::getDuration()
{
    return duration;
}

QString matchInfo::getFirstBloodTime()
{
    return firstBloodTime;
}

QString matchInfo::getMatchWinner()
{
    if(json.object().value("radiant_win").toString().compare("1") == 0)
        return "<font color=\"green\">Radiant Victory</font>";
    else
        return "<font color=\"red\">Dire Victory</font>";
}

QVector<QVector<QString> > matchInfo::getPicks()
{
    return picks;
}

QVector<QVector<QString> > matchInfo::getBans()
{
    return bans;
}

QVector<QVector<QString> > matchInfo::getPlayerNames()
{
    return playerNames;
}

QVector<QVector<QString> > matchInfo::getPlayerLevel()
{
    return playerLevel;
}

QVector<QVector<QJsonObject> > matchInfo::getPlayerHeroName()
{
    return playerHeroName;
}

QVector<QVector<QString> > matchInfo::getPlayerKills()
{
    return playerKills;
}

QVector<QVector<QString> > matchInfo::getPlayerDeaths()
{
    return playerDeaths;
}

QVector<QVector<QString> > matchInfo::getPlayerAssists()
{
    return playerAssists;
}

QVector<QVector<QVector<QString> > > matchInfo::getPlayerItems()
{
    return playerItems;
}

QVector<QVector<QString> > matchInfo::getPlayerGold()
{
    return playerGold;
}

QVector<QVector<QString> > matchInfo::getPlayerLH()
{
    return playerLH;
}

QVector<QVector<QString> > matchInfo::getPlayerDN()
{
    return playerDN;
}

QVector<QVector<QString> > matchInfo::getPlayerGPM()
{
    return playerGPM;
}

QVector<QVector<QString> > matchInfo::getPlayerXPM()
{
    return playerXPM;
}

void matchInfo::downloadImages()
{
    Http http;
    QEventLoop loop;

    //get picks for picks & bans
    if(gameMode.compare("Captains Mode") == 0)
    {
        for(int i=0; i<2; i++)
            for(int j=0; j<5; j++)
            {
                //qDebug() << "Downloading... " + baseUrl + "heroes/" + getBans()[i][j] + "_sb.png";
                http.append(QUrl(baseUrl + "heroes/" + getBans()[i][j] + "_sb.png" ));
                http.append(QUrl(baseUrl + "heroes/" + getPicks()[i][j] + "_sb.png" ));
            }
    }
    else if(gameMode.compare("Captains Draft") == 0)
    {
        for(int i=0; i<2; i++)
            for(int j=0; j<2; j++)
                http.append(QUrl(baseUrl + "heroes/" + getPicks()[i][j] + "_sb.png" ));
    }

    for(int i=0; i<2; i++)
        for(int j=0; j<5; j++)
        {
            //download hero pic(s)
            http.append(QUrl(baseUrl + "heroes/" + getPlayerHeroName()[i][j].value("name").toString() + "_sb.png"));
            //download item(s)
            for(int k=0; k<6; k++)
            {
                http.append(QUrl(baseUrl + "items/" + getPlayerItems()[i][j][k] + "_lg.png"));
            }
        }

    //Need an event loop so that networking can do its thing and download the files.
    //once all images are downloaded, the loop exits.
    connect(&http, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
