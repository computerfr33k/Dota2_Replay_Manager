#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    dir = settings->value("replayFolder", "C:/Program Files (x86)/Steam/SteamApps/common/dota 2 beta/dota/replays").toString();
    picDir = QDir::currentPath() + "/thumbnails/";
    start();
    addFilesToDb();
}


MainWindow::~MainWindow()
{
    db.close();
    delete settings;
    delete ui;
    delete model;
}

void MainWindow::start()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("matches.db");
    db.open();
    db.exec( "create table if not exists replays (title TEXT, filename TEXT PRIMARY KEY, fileExists BLOB)" );
    model = new QSqlTableModel(this, db);
    ui->tableView->setModel(model);
    model->setTable("replays");
    model->select();
    ui->tableView->hideColumn(2);
}

void MainWindow::checkDb() //check and remove files from db that are no longer locally saved
{
    db.transaction();
    QSqlQuery query;
    QSqlQuery updateRow;
    query.exec("update replays set fileExists = 0");
    updateRow.prepare("update replays set fileExists = 1 where filename = :filename");
    query.prepare("select * from replays where filename = :id");
    int i=0;
    for(QStringList::Iterator it = list.begin(); it != list.end(); it++)
    {
        QString param = QString(list.at(i));
        qDebug() << param;
        query.bindValue(":id", param);
        if(query.exec())
        {
            while(query.next())
            {
                updateRow.bindValue(0, param);
                updateRow.exec();
            }
        }
        else
        {
            qDebug() << "query did not exec;";
        }
        i++;
    }
    query.exec("delete from replays where fileExists = 0");
    db.commit();
    //ui->tableView->sortByColumn(1, Qt::DescendingOrder);
    //model->select();
}

void MainWindow::addFilesToDb()
{
    list.clear();
    QSqlQuery query;
    db.transaction();
    query.prepare("insert into replays (fileExists, filename) VALUES (1, :filename)");
    bool ok = dir.exists();  //check if directory exist
        if ( ok )
        {
            //set fileinfo filter
            QFileInfoList entries = dir.entryInfoList( QDir::NoDotAndDotDot |
                    QDir::Dirs | QDir::Files );
                    //loop over entries filter selected
            foreach ( QFileInfo entryInfo, entries )
            {
                //QString path = entryInfo.absoluteFilePath();
                QString fileName = entryInfo.fileName();

                if ( entryInfo.isDir() )    //check if entryInfo is dir
                {
                }
                else
                {
                    if(fileName.endsWith(".dem"))
                    {
                        list.append(fileName);
                        query.bindValue(":filename", fileName);
                        query.exec();
                    }
                }
            }
        }

        if (ok && !dir.exists(dir.absolutePath()))
            ok = false;

        db.commit();
        checkDb();
        model->select();
        ui->tableView->resizeColumnsToContents();
        //return ok;
}

void MainWindow::on_commandLinkButton_clicked()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::httpFinished()
{
    QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
    qDebug() << json.toJson();
    setMatchInfo(json);
}

void MainWindow::setMatchInfo(QJsonDocument json)
{
    //main match info
    ui->matchID->setText(json.object().value("match_id").toString());
    ui->gameMode->setText(json.object().value("game_mode").toString());
    ui->startTime->setText(json.object().value("start_time").toString());
    ui->lobbyType->setText(json.object().value("lobby_type").toString());
    ui->duration->setText(json.object().value("duration").toString());
    ui->fbTime->setText(json.object().value("first_blood_time").toString());

    //radiant
    // radiant Player Names
    QJsonArray radiantSlots = json.object().value("slots").toObject().value("radiant").toArray();
    ui->radiantPlayer_1->setText(radiantSlots.at(0).toObject().value("account_name").toString());
    ui->radiantPlayer_2->setText(radiantSlots.at(1).toObject().value("account_name").toString());
    ui->radiantPlayer_3->setText(radiantSlots.at(2).toObject().value("account_name").toString());
    ui->radiantPlayer_4->setText(radiantSlots.at(3).toObject().value("account_name").toString());
    ui->radiantPlayer_5->setText(radiantSlots.at(4).toObject().value("account_name").toString());

    //radiant levels
    ui->radiantLevel_1->setText(radiantSlots.at(0).toObject().value("level").toString());
    ui->radiantLevel_2->setText(radiantSlots.at(1).toObject().value("level").toString());
    ui->radiantLevel_3->setText(radiantSlots.at(2).toObject().value("level").toString());
    ui->radiantLevel_4->setText(radiantSlots.at(3).toObject().value("level").toString());
    ui->radiantLevel_5->setText(radiantSlots.at(4).toObject().value("level").toString());

    //radiant Hero Pix
    ui->radiantHeroPic_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantSlots.at(0).toObject().value("hero").toObject().value("name").toString() + ".jpg"));
    ui->radiantHeroPic_2->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantSlots.at(1).toObject().value("hero").toObject().value("name").toString() + ".jpg"));
    ui->radiantHeroPic_3->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantSlots.at(2).toObject().value("hero").toObject().value("name").toString() + ".jpg"));
    ui->radiantHeroPic_4->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantSlots.at(3).toObject().value("hero").toObject().value("name").toString() + ".jpg"));
    ui->radiantHeroPic_5->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantSlots.at(4).toObject().value("hero").toObject().value("name").toString() + ".jpg"));

    // radiant Hero Names
    ui->radiantHero_1->setText(radiantSlots.at(0).toObject().value("hero").toObject().value("localized_name").toString());
    ui->radiantHero_2->setText(radiantSlots.at(1).toObject().value("hero").toObject().value("localized_name").toString());
    ui->radiantHero_3->setText(radiantSlots.at(2).toObject().value("hero").toObject().value("localized_name").toString());
    ui->radiantHero_4->setText(radiantSlots.at(3).toObject().value("hero").toObject().value("localized_name").toString());
    ui->radiantHero_5->setText(radiantSlots.at(4).toObject().value("hero").toObject().value("localized_name").toString());

    //radiant Kills
    ui->radiantKills_1->setText(radiantSlots.at(0).toObject().value("kills").toString());
    ui->radiantKills_2->setText(radiantSlots.at(1).toObject().value("kills").toString());
    ui->radiantKills_3->setText(radiantSlots.at(2).toObject().value("kills").toString());
    ui->radiantKills_4->setText(radiantSlots.at(3).toObject().value("kills").toString());
    ui->radiantKills_5->setText(radiantSlots.at(4).toObject().value("kills").toString());

    //radiant Deaths
    ui->radiantDeaths_1->setText(radiantSlots.at(0).toObject().value("deaths").toString());
    ui->radiantDeaths_2->setText(radiantSlots.at(1).toObject().value("deaths").toString());
    ui->radiantDeaths_3->setText(radiantSlots.at(2).toObject().value("deaths").toString());
    ui->radiantDeaths_4->setText(radiantSlots.at(3).toObject().value("deaths").toString());
    ui->radiantDeaths_5->setText(radiantSlots.at(4).toObject().value("deaths").toString());

    //radiant Assists
    ui->radiantAssists_1->setText(radiantSlots.at(0).toObject().value("assists").toString());
    ui->radiantAssists_2->setText(radiantSlots.at(1).toObject().value("assists").toString());
    ui->radiantAssists_3->setText(radiantSlots.at(2).toObject().value("assists").toString());
    ui->radiantAssists_4->setText(radiantSlots.at(3).toObject().value("assists").toString());
    ui->radiantAssists_5->setText(radiantSlots.at(4).toObject().value("assists").toString());

    //radiant Items
    //player 1
    ui->radiantItems_1_1->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_0").toString() + ".jpg"));
    ui->radiantItems_1_2->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_1").toString() + ".jpg"));
    ui->radiantItems_1_3->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_2").toString() + ".jpg"));
    ui->radiantItems_1_4->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_3").toString() + ".jpg"));
    ui->radiantItems_1_5->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_4").toString() + ".jpg"));
    ui->radiantItems_1_6->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(0).toObject().value("item_5").toString() + ".jpg"));
    //player 2
    ui->radiantItems_2_1->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_0").toString() + ".jpg"));
    ui->radiantItems_2_2->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_1").toString() + ".jpg"));
    ui->radiantItems_2_3->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_2").toString() + ".jpg"));
    ui->radiantItems_2_4->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_3").toString() + ".jpg"));
    ui->radiantItems_2_5->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_4").toString() + ".jpg"));
    ui->radiantItems_2_6->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(1).toObject().value("item_5").toString() + ".jpg"));
    //player 3
    ui->radiantItems_3_1->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_0").toString() + ".jpg"));
    ui->radiantItems_3_2->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_1").toString() + ".jpg"));
    ui->radiantItems_3_3->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_2").toString() + ".jpg"));
    ui->radiantItems_3_4->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_3").toString() + ".jpg"));
    ui->radiantItems_3_5->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_4").toString() + ".jpg"));
    ui->radiantItems_3_6->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(2).toObject().value("item_5").toString() + ".jpg"));
    //player 4
    ui->radiantItems_4_1->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_0").toString() + ".jpg"));
    ui->radiantItems_4_2->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_1").toString() + ".jpg"));
    ui->radiantItems_4_3->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_2").toString() + ".jpg"));
    ui->radiantItems_4_4->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_3").toString() + ".jpg"));
    ui->radiantItems_4_5->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_4").toString() + ".jpg"));
    ui->radiantItems_4_6->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(3).toObject().value("item_5").toString() + ".jpg"));
    //player 5
    ui->radiantItems_5_1->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_0").toString() + ".jpg"));
    ui->radiantItems_5_2->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_1").toString() + ".jpg"));
    ui->radiantItems_5_3->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_2").toString() + ".jpg"));
    ui->radiantItems_5_4->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_3").toString() + ".jpg"));
    ui->radiantItems_5_5->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_4").toString() + ".jpg"));
    ui->radiantItems_5_6->setPixmap(QPixmap(picDir + "items/JPEG/" + radiantSlots.at(4).toObject().value("item_5").toString() + ".jpg"));

    //radiant Gold
    /*
    int radiantGold[5];
    for(int i=0; i < 5; i++)
    {
        radiantGold[i] = radiantSlots.at(i).toObject().value("gold_spent").toString().toInt();
        radiantGold[i] += radiantSlots.at(i).toObject().value("gold").toString().toInt();
    }
    */
    ui->radiantGold_1->setText(radiantSlots.at(0).toObject().value("gold_spent").toString());
    ui->radiantGold_2->setText(radiantSlots.at(1).toObject().value("gold_spent").toString());
    ui->radiantGold_3->setText(radiantSlots.at(2).toObject().value("gold_spent").toString());
    ui->radiantGold_4->setText(radiantSlots.at(3).toObject().value("gold_spent").toString());
    ui->radiantGold_5->setText(radiantSlots.at(4).toObject().value("gold_spent").toString());

    //radiant Last Hits
    ui->radiantLH_1->setText(radiantSlots.at(0).toObject().value("last_hits").toString());
    ui->radiantLH_2->setText(radiantSlots.at(1).toObject().value("last_hits").toString());
    ui->radiantLH_3->setText(radiantSlots.at(2).toObject().value("last_hits").toString());
    ui->radiantLH_4->setText(radiantSlots.at(3).toObject().value("last_hits").toString());
    ui->radiantLH_5->setText(radiantSlots.at(4).toObject().value("last_hits").toString());

    //radiant Denies
    ui->radiantDN_1->setText(radiantSlots.at(0).toObject().value("denies").toString());
    ui->radiantDN_2->setText(radiantSlots.at(1).toObject().value("denies").toString());
    ui->radiantDN_3->setText(radiantSlots.at(2).toObject().value("denies").toString());
    ui->radiantDN_4->setText(radiantSlots.at(3).toObject().value("denies").toString());
    ui->radiantDN_5->setText(radiantSlots.at(4).toObject().value("denies").toString());

    //radiant Gold/Min
    ui->radiantGPM_1->setText(radiantSlots.at(0).toObject().value("gold_per_min").toString());
    ui->radiantGPM_2->setText(radiantSlots.at(1).toObject().value("gold_per_min").toString());
    ui->radiantGPM_3->setText(radiantSlots.at(2).toObject().value("gold_per_min").toString());
    ui->radiantGPM_4->setText(radiantSlots.at(3).toObject().value("gold_per_min").toString());
    ui->radiantGPM_5->setText(radiantSlots.at(4).toObject().value("gold_per_min").toString());

    //radiant XP/Min
    ui->radiantXPM_1->setText(radiantSlots.at(0).toObject().value("xp_per_min").toString());
    ui->radiantXPM_2->setText(radiantSlots.at(1).toObject().value("xp_per_min").toString());
    ui->radiantXPM_3->setText(radiantSlots.at(2).toObject().value("xp_per_min").toString());
    ui->radiantXPM_4->setText(radiantSlots.at(3).toObject().value("xp_per_min").toString());
    ui->radiantXPM_5->setText(radiantSlots.at(4).toObject().value("xp_per_min").toString());
    //end radiant

    //dire
    QJsonArray direSlots = json.object().value("slots").toObject().value("dire").toArray();
    //dire player names
    ui->direPlayer_1->setText(direSlots.at(0).toObject().value("account_name").toString());
    ui->direPlayer_2->setText(direSlots.at(1).toObject().value("account_name").toString());
    ui->direPlayer_3->setText(direSlots.at(2).toObject().value("account_name").toString());
    ui->direPlayer_4->setText(direSlots.at(3).toObject().value("account_name").toString());
    ui->direPlayer_5->setText(direSlots.at(4).toObject().value("account_name").toString());

    //dire Levels
    ui->direLevel_1->setText(direSlots.at(0).toObject().value("level").toString());
    ui->direLevel_2->setText(direSlots.at(1).toObject().value("level").toString());
    ui->direLevel_3->setText(direSlots.at(2).toObject().value("level").toString());
    ui->direLevel_4->setText(direSlots.at(3).toObject().value("level").toString());
    ui->direLevel_5->setText(direSlots.at(4).toObject().value("level").toString());

    //dire Hero Pix
    ui->direHeroPic_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direSlots.at(0).toObject().value("hero").toObject().value("name").toString()));
    ui->direHeroPic_2->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direSlots.at(1).toObject().value("hero").toObject().value("name").toString()));
    ui->direHeroPic_3->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direSlots.at(2).toObject().value("hero").toObject().value("name").toString()));
    ui->direHeroPic_4->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direSlots.at(3).toObject().value("hero").toObject().value("name").toString()));
    ui->direHeroPic_5->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direSlots.at(4).toObject().value("hero").toObject().value("name").toString()));

    //dire Hero Names
    ui->direHero_1->setText(direSlots.at(0).toObject().value("hero").toObject().value("localized_name").toString());
    ui->direHero_2->setText(direSlots.at(1).toObject().value("hero").toObject().value("localized_name").toString());
    ui->direHero_3->setText(direSlots.at(2).toObject().value("hero").toObject().value("localized_name").toString());
    ui->direHero_4->setText(direSlots.at(3).toObject().value("hero").toObject().value("localized_name").toString());
    ui->direHero_5->setText(direSlots.at(4).toObject().value("hero").toObject().value("localized_name").toString());

    //dire Kills
    ui->direKills_1->setText(direSlots.at(0).toObject().value("kills").toString());
    ui->direKills_2->setText(direSlots.at(1).toObject().value("kills").toString());
    ui->direKills_3->setText(direSlots.at(2).toObject().value("kills").toString());
    ui->direKills_4->setText(direSlots.at(3).toObject().value("kills").toString());
    ui->direKills_5->setText(direSlots.at(4).toObject().value("kills").toString());

    //dire Deaths
    ui->direDeaths_1->setText(direSlots.at(0).toObject().value("deaths").toString());
    ui->direDeaths_2->setText(direSlots.at(1).toObject().value("deaths").toString());
    ui->direDeaths_3->setText(direSlots.at(2).toObject().value("deaths").toString());
    ui->direDeaths_4->setText(direSlots.at(3).toObject().value("deaths").toString());
    ui->direDeaths_5->setText(direSlots.at(4).toObject().value("deaths").toString());

    //dire Assists
    ui->direAssists_1->setText(direSlots.at(0).toObject().value("assists").toString());
    ui->direAssists_2->setText(direSlots.at(1).toObject().value("assists").toString());
    ui->direAssists_3->setText(direSlots.at(2).toObject().value("assists").toString());
    ui->direAssists_4->setText(direSlots.at(3).toObject().value("assists").toString());
    ui->direAssists_5->setText(direSlots.at(4).toObject().value("assists").toString());

    //dire Items
    //player 1
    ui->direItems_1_1->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_0").toString() + ".jpg"));
    ui->direItems_1_2->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_1").toString() + ".jpg"));
    ui->direItems_1_3->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_2").toString() + ".jpg"));
    ui->direItems_1_4->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_3").toString() + ".jpg"));
    ui->direItems_1_5->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_4").toString() + ".jpg"));
    ui->direItems_1_6->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(0).toObject().value("item_5").toString() + ".jpg"));
    //player 2
    ui->direItems_2_1->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_0").toString() + ".jpg"));
    ui->direItems_2_2->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_1").toString() + ".jpg"));
    ui->direItems_2_3->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_2").toString() + ".jpg"));
    ui->direItems_2_4->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_3").toString() + ".jpg"));
    ui->direItems_2_5->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_4").toString() + ".jpg"));
    ui->direItems_2_6->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(1).toObject().value("item_5").toString() + ".jpg"));
    //player 3
    ui->direItems_3_1->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_0").toString() + ".jpg"));
    ui->direItems_3_2->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_1").toString() + ".jpg"));
    ui->direItems_3_3->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_2").toString() + ".jpg"));
    ui->direItems_3_4->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_3").toString() + ".jpg"));
    ui->direItems_3_5->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_4").toString() + ".jpg"));
    ui->direItems_3_6->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(2).toObject().value("item_5").toString() + ".jpg"));
    //player 4
    ui->direItems_4_1->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_0").toString() + ".jpg"));
    ui->direItems_4_2->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_1").toString() + ".jpg"));
    ui->direItems_4_3->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_2").toString() + ".jpg"));
    ui->direItems_4_4->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_3").toString() + ".jpg"));
    ui->direItems_4_5->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_4").toString() + ".jpg"));
    ui->direItems_4_6->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(3).toObject().value("item_5").toString() + ".jpg"));
    //player 5
    ui->direItems_5_1->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_0").toString() + ".jpg"));
    ui->direItems_5_2->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_1").toString() + ".jpg"));
    ui->direItems_5_3->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_2").toString() + ".jpg"));
    ui->direItems_5_4->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_3").toString() + ".jpg"));
    ui->direItems_5_5->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_4").toString() + ".jpg"));
    ui->direItems_5_6->setPixmap(QPixmap(picDir + "items/JPEG/" + direSlots.at(4).toObject().value("item_5").toString() + ".jpg"));

    //dire Gold
    ui->direGold_1->setText(direSlots.at(0).toObject().value("gold_spent").toString());
    ui->direGold_2->setText(direSlots.at(1).toObject().value("gold_spent").toString());
    ui->direGold_3->setText(direSlots.at(2).toObject().value("gold_spent").toString());
    ui->direGold_4->setText(direSlots.at(3).toObject().value("gold_spent").toString());
    ui->direGold_5->setText(direSlots.at(4).toObject().value("gold_spent").toString());

    //dire Last Hits
    ui->direLH_1->setText(direSlots.at(0).toObject().value("last_hits").toString());
    ui->direLH_2->setText(direSlots.at(1).toObject().value("last_hits").toString());
    ui->direLH_3->setText(direSlots.at(2).toObject().value("last_hits").toString());
    ui->direLH_4->setText(direSlots.at(3).toObject().value("last_hits").toString());
    ui->direLH_5->setText(direSlots.at(4).toObject().value("last_hits").toString());

    //dire Denies
    ui->direDN_1->setText(direSlots.at(0).toObject().value("denies").toString());
    ui->direDN_2->setText(direSlots.at(1).toObject().value("denies").toString());
    ui->direDN_3->setText(direSlots.at(2).toObject().value("denies").toString());
    ui->direDN_4->setText(direSlots.at(3).toObject().value("denies").toString());
    ui->direDN_5->setText(direSlots.at(4).toObject().value("denies").toString());

    //dire GPM
    ui->direGPM_1->setText(direSlots.at(0).toObject().value("gold_per_min").toString());
    ui->direGPM_2->setText(direSlots.at(1).toObject().value("gold_per_min").toString());
    ui->direGPM_3->setText(direSlots.at(2).toObject().value("gold_per_min").toString());
    ui->direGPM_4->setText(direSlots.at(3).toObject().value("gold_per_min").toString());
    ui->direGPM_5->setText(direSlots.at(4).toObject().value("gold_per_min").toString());

    //dire XPM
    ui->direXPM_1->setText(direSlots.at(0).toObject().value("xp_per_min").toString());
    ui->direXPM_2->setText(direSlots.at(1).toObject().value("xp_per_min").toString());
    ui->direXPM_3->setText(direSlots.at(2).toObject().value("xp_per_min").toString());
    ui->direXPM_4->setText(direSlots.at(3).toObject().value("xp_per_min").toString());
    ui->direXPM_5->setText(direSlots.at(4).toObject().value("xp_per_min").toString());

    //end dire

    //switch to match details tab after writing all the info
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_viewMatchButton_clicked()
{
    queryModel.setQuery("SELECT * FROM replays");
    QString matchID = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString().remove(".dem");
    manager = new QNetworkAccessManager(this);
    reply = manager->get(QNetworkRequest(QUrl("http://dota2.computerfr33k.com/json.php?match_id=" + matchID)));
    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void MainWindow::on_editTitle_clicked()
{
    queryModel.setQuery("SELECT * FROM replays");
    EditTitle title;
    title.setTitle(queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("title").toString());
    if(title.exec())
    {
        QSqlQuery query("update replays set title = :title WHERE filename = :filename");
        query.bindValue(0, title.getTitle());
        query.bindValue(1, queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString());
        query.exec();
        model->select();
        ui->tableView->resizeColumnsToContents();
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    Preferences pref;
    if(pref.exec())
    {
        dir = pref.getDir();
        settings->setValue("replayFolder", pref.getDir());
        settings->sync();
        QMessageBox::information(this, "Info", "Please Restart The Program To Reload The New Folder.\nThis is jsut a limitation until I finish this part of the program");
    }
}
