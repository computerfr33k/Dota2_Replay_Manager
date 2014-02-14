#include "matchinfo.h"

matchInfo::matchInfo(QObject *parent) :
    QObject(parent)
{
    //size our array accordingly
    bans.resize(2);
    bans[0].resize(5);
    bans[1].resize(5);
    picks.resize(2);
    picks[0].resize(5);
    picks[1].resize(5);

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

void matchInfo::downloadImages()
{
    Http http;
    QEventLoop loop;

    //get picks for picks & bans
    for(int i=0; i<2; i++)
        for(int j=0; j<5; j++)
        {
            //qDebug() << "Downloading... " + baseUrl + "heroes/" + getBans()[i][j] + "_sb.png";
            http.append(QUrl(baseUrl + "heroes/" + getBans()[i][j] + "_sb.png" ));
            http.append(QUrl(baseUrl + "heroes/" + getPicks()[i][j] + "_sb.png" ));
        }

    //Need an event loop so that networking can do its thing and download the files.
    //once all images are downloaded, the loop exits.
    connect(&http, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
