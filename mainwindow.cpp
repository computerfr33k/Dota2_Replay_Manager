#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    start();
    addFilesToDb();
}


MainWindow::~MainWindow()
{
    settings->setValue("windowGeometry", saveGeometry());
    settings->setValue("windowState", saveState());
    settings->sync();
    db.exec(QString("VACUUM"));
    db.close();
    delete settings;
    delete ui;
    delete model;
}

void MainWindow::start()
{
    userDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0);

    //create the folder if it doesn't already exist
    if(!QDir(userDir).exists())
    {
        userDir.mkpath(userDir.absolutePath());
    }
    settings = new QSettings(userDir.absolutePath() + "/settings.ini", QSettings::IniFormat);
    apiKey = settings->value("apiKey").toString();

    dir = settings->value("replayFolder", "C:/Program Files (x86)/Steam/SteamApps/common/dota 2 beta/dota/replays").toString();
    restoreGeometry(settings->value("windowGeometry", "").toByteArray()); //restore previous session's dimensions of the program
    restoreState(settings->value("windowState", "").toByteArray()); //restore the previous session's state of the program

    //set buttons to disabled until user selects a valid row
    ui->watchReplay->setEnabled(false);
    ui->editTitle->setEnabled(false);
    ui->viewMatchButton->setEnabled(false);
    ui->deleteReplayButton->setEnabled(false);

    //font settings
    font.setPointSize(settings->value("fontSize", "10").toInt());
    font.setFamily(settings->value("fontFamily", "Times New Roman").toString());

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(userDir.absolutePath() + "/matches.db");
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
}

/*
 * Valid types are 'heroes' or 'items'
 * name is the name of the hero/item your are fetching
 */
QPixmap MainWindow::getImage(QString type, QString name)
{
    //user canceled the network request, so block
    if(block)
        return QPixmap();

    //return empty QPixmap because the item slot was empty, so no need to try and fetch something that will fail and waste time for a failed request.
    if(name.compare("empty") == 0)
        return QPixmap();

    QPixmap pic;
    QString size;
    int width;

    manager = new QNetworkAccessManager(this);
    QNetworkDiskCache *cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(userDir.absolutePath() + "/cache");
    manager->setCache(cache);

    if(type.compare("heroes") == 0)
    {
        size = "sb";
        width = 45;
    }
    else
    {
        size = "lg";
        width = 32;
    }

    QNetworkRequest req(QUrl("http://media.steampowered.com/apps/dota2/images/" + type + "/" + name + "_" + size + ".png"));
    req.setRawHeader("User-Agent","d2rm-app");
    reply = manager->get(req);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    pic.loadFromData(reply->readAll());

    qDebug() << "image from cache: " << reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();

    manager->deleteLater();
    reply->deleteLater();

    return pic.scaledToWidth(width);
}

void MainWindow::on_watchReplay_clicked()
{
    queryModel.setQuery("SELECT * FROM replays");
    QDialog dialog(this);
    QVBoxLayout *layout = new QVBoxLayout;
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    textEdit->setHtml(QString("Type This Into Dota 2 Console: <p> <pre>playdemo replays/%1</pre><p><em>make sure the replay is in your default dota 2 replay directory</em>").arg(queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString()));
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    QPushButton *acceptButton = new QPushButton(tr("Ok"));
    buttonBox->addButton(acceptButton, QDialogButtonBox::AcceptRole);
    layout->addWidget(buttonBox);

    dialog.setLayout(layout);
    dialog.connect(buttonBox, SIGNAL(accepted()), SLOT(close()));
    dialog.exec();
    acceptButton->deleteLater();
    buttonBox->deleteLater();
    textEdit->deleteLater();
    layout->deleteLater();
}

void MainWindow::httpFinished()
{
        QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

        if(json.object().value("match_id").isNull())
        {
            QMessageBox::warning(this, "ERROR!", "Could not find match.");
        }
        else
        {
            if(json.object().value("success").toString().compare("1") == 0) //if success is 1, strcmp returns 0; if success true
                setMatchInfo(json);
            else
               QMessageBox::warning(this, "Warning", json.object().value("message").toString());
        }

    manager->deleteLater();
    reply->deleteLater();
}

void MainWindow::setMatchInfo(QJsonDocument json)
{
    //either network was canceled, or network error occured. Don't waste time iterating.
    if(block)
    {
        return;
    }

    //set winner
    if(json.object().value("radiant_win").toString().compare("1") == 0)
        ui->winner->setText("<font color=\"green\">Radiant Victory</font>");
    else
        ui->winner->setText("<font color=\"red\">Dire Victory</font>");

    //main match info
    ui->matchID->setText(QString("<a href=\"http://dotabuff.com/matches/%1\">%1</a>").arg(json.object().value("match_id").toString()));

    ui->gameMode->setText(json.object().value("game_mode").toString());
    ui->startTime->setText(json.object().value("start_time").toString());
    ui->lobbyType->setText(json.object().value("lobby_type").toString());
    ui->duration->setText(json.object().value("duration").toString());
    ui->fbTime->setText(json.object().value("first_blood_time").toString());

    //picks and bans
    if(json.object().value("game_mode").toString().compare("Captains Mode") == 0)
    {
        QJsonArray radiantBans = json.object().value("picks_bans").toObject().value("radiant").toObject().value("bans").toArray();
        //ui->radiantBan_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantBans.at(0).toObject().value("name").toString() + ".jpg"));
        ui->radiantBan_1->setPixmap(getImage(QString("heroes"), radiantBans.at(0).toObject().value("name").toString()));
        ui->radiantBan_2->setPixmap(getImage(QString("heroes"), radiantBans.at(1).toObject().value("name").toString()));
        ui->radiantBan_3->setPixmap(getImage(QString("heroes"), radiantBans.at(2).toObject().value("name").toString()));
        ui->radiantBan_4->setPixmap(getImage(QString("heroes"), radiantBans.at(3).toObject().value("name").toString()));
        ui->radiantBan_5->setPixmap(getImage(QString("heroes"), radiantBans.at(4).toObject().value("name").toString()));

        QJsonArray radiantPicks = json.object().value("picks_bans").toObject().value("radiant").toObject().value("picks").toArray();
        //ui->radiantPick_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + radiantPicks.at(0).toObject().value("name").toString() + ".jpg"));
        ui->radiantPick_1->setPixmap(getImage("heroes", radiantPicks.at(0).toObject().value("name").toString() ));
        ui->radiantPick_2->setPixmap(getImage("heroes", radiantPicks.at(1).toObject().value("name").toString() ));
        ui->radiantPick_3->setPixmap(getImage("heroes", radiantPicks.at(2).toObject().value("name").toString() ));
        ui->radiantPick_4->setPixmap(getImage("heroes", radiantPicks.at(3).toObject().value("name").toString() ));
        ui->radiantPick_5->setPixmap(getImage("heroes", radiantPicks.at(4).toObject().value("name").toString() ));

        QJsonArray direBans = json.object().value("picks_bans").toObject().value("dire").toObject().value("bans").toArray();
        //ui->direBan_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direBans.at(0).toObject().value("name").toString() + ".jpg"));
        ui->direBan_1->setPixmap(getImage( "heroes", direBans.at(0).toObject().value("name").toString() ));
        ui->direBan_2->setPixmap(getImage( "heroes", direBans.at(1).toObject().value("name").toString() ));
        ui->direBan_3->setPixmap(getImage( "heroes", direBans.at(2).toObject().value("name").toString() ));
        ui->direBan_4->setPixmap(getImage( "heroes", direBans.at(3).toObject().value("name").toString() ));
        ui->direBan_5->setPixmap(getImage( "heroes", direBans.at(4).toObject().value("name").toString() ));

        QJsonArray direPicks = json.object().value("picks_bans").toObject().value("dire").toObject().value("picks").toArray();
        //ui->direPick_1->setPixmap(QPixmap(picDir + "heroes/JPEG/" + direPicks.at(0).toObject().value("name").toString() + ".jpg"));
        ui->direPick_1->setPixmap(getImage( "heroes", direPicks.at(0).toObject().value("name").toString() ));
        ui->direPick_2->setPixmap(getImage( "heroes", direPicks.at(1).toObject().value("name").toString() ));
        ui->direPick_3->setPixmap(getImage( "heroes", direPicks.at(2).toObject().value("name").toString() ));
        ui->direPick_4->setPixmap(getImage( "heroes", direPicks.at(3).toObject().value("name").toString() ));
        ui->direPick_5->setPixmap(getImage( "heroes", direPicks.at(4).toObject().value("name").toString() ));
    }
    else
    {
        //clear picks and bans because the selected match was not in CM
        ui->radiantBan_1->clear();
        ui->radiantBan_2->clear();
        ui->radiantBan_3->clear();
        ui->radiantBan_4->clear();
        ui->radiantBan_5->clear();

        ui->radiantPick_1->clear();
        ui->radiantPick_2->clear();
        ui->radiantPick_3->clear();
        ui->radiantPick_4->clear();
        ui->radiantPick_5->clear();

        ui->direPick_1->clear();
        ui->direPick_2->clear();
        ui->direPick_3->clear();
        ui->direPick_4->clear();
        ui->direPick_5->clear();

        ui->direBan_1->clear();
        ui->direBan_2->clear();
        ui->direBan_3->clear();
        ui->direBan_4->clear();
        ui->direBan_5->clear();
    }

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
    ui->radiantHeroPic_1->setPixmap(getImage( "heroes", radiantSlots.at(0).toObject().value("hero").toObject().value("name").toString() ));
    ui->radiantHeroPic_2->setPixmap(getImage( "heroes", radiantSlots.at(1).toObject().value("hero").toObject().value("name").toString() ));
    ui->radiantHeroPic_3->setPixmap(getImage( "heroes", radiantSlots.at(2).toObject().value("hero").toObject().value("name").toString() ));
    ui->radiantHeroPic_4->setPixmap(getImage( "heroes", radiantSlots.at(3).toObject().value("hero").toObject().value("name").toString() ));
    ui->radiantHeroPic_5->setPixmap(getImage( "heroes", radiantSlots.at(4).toObject().value("hero").toObject().value("name").toString() ));

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
    ui->radiantItems_1_1->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_0").toString() ));
    ui->radiantItems_1_2->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_1").toString() ));
    ui->radiantItems_1_3->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_2").toString() ));
    ui->radiantItems_1_4->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_3").toString() ));
    ui->radiantItems_1_5->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_4").toString() ));
    ui->radiantItems_1_6->setPixmap(getImage( "items", radiantSlots.at(0).toObject().value("item_5").toString() ));

    //player 2
    ui->radiantItems_2_1->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_0").toString() ));
    ui->radiantItems_2_2->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_1").toString() ));
    ui->radiantItems_2_3->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_2").toString() ));
    ui->radiantItems_2_4->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_3").toString() ));
    ui->radiantItems_2_5->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_4").toString() ));
    ui->radiantItems_2_6->setPixmap(getImage( "items", radiantSlots.at(1).toObject().value("item_5").toString() ));

    //player 3
    ui->radiantItems_3_1->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_0").toString() ));
    ui->radiantItems_3_2->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_1").toString() ));
    ui->radiantItems_3_3->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_2").toString() ));
    ui->radiantItems_3_4->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_3").toString() ));
    ui->radiantItems_3_5->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_4").toString() ));
    ui->radiantItems_3_6->setPixmap(getImage( "items", radiantSlots.at(2).toObject().value("item_5").toString() ));

    //player 4
    ui->radiantItems_4_1->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_0").toString() ));
    ui->radiantItems_4_2->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_1").toString() ));
    ui->radiantItems_4_3->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_2").toString() ));
    ui->radiantItems_4_4->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_3").toString() ));
    ui->radiantItems_4_5->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_4").toString() ));
    ui->radiantItems_4_6->setPixmap(getImage( "items", radiantSlots.at(3).toObject().value("item_5").toString() ));

    //player 5
    ui->radiantItems_5_1->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_0").toString() ));
    ui->radiantItems_5_2->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_1").toString() ));
    ui->radiantItems_5_3->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_2").toString() ));
    ui->radiantItems_5_4->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_3").toString() ));
    ui->radiantItems_5_5->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_4").toString() ));
    ui->radiantItems_5_6->setPixmap(getImage( "items", radiantSlots.at(4).toObject().value("item_5").toString() ));

    //radiant gold spent
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
    ui->direHeroPic_1->setPixmap(getImage( "heroes", direSlots.at(0).toObject().value("hero").toObject().value("name").toString() ));
    ui->direHeroPic_2->setPixmap(getImage( "heroes", direSlots.at(1).toObject().value("hero").toObject().value("name").toString() ));
    ui->direHeroPic_3->setPixmap(getImage( "heroes", direSlots.at(2).toObject().value("hero").toObject().value("name").toString() ));
    ui->direHeroPic_4->setPixmap(getImage( "heroes", direSlots.at(3).toObject().value("hero").toObject().value("name").toString() ));
    ui->direHeroPic_5->setPixmap(getImage( "heroes", direSlots.at(4).toObject().value("hero").toObject().value("name").toString() ));

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
    ui->direItems_1_1->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_0").toString() ));
    ui->direItems_1_2->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_1").toString() ));
    ui->direItems_1_3->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_2").toString() ));
    ui->direItems_1_4->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_3").toString() ));
    ui->direItems_1_5->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_4").toString() ));
    ui->direItems_1_6->setPixmap(getImage( "items", direSlots.at(0).toObject().value("item_5").toString() ));

    //player 2
    ui->direItems_2_1->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_0").toString() ));
    ui->direItems_2_2->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_1").toString() ));
    ui->direItems_2_3->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_2").toString() ));
    ui->direItems_2_4->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_3").toString() ));
    ui->direItems_2_5->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_4").toString() ));
    ui->direItems_2_6->setPixmap(getImage( "items", direSlots.at(1).toObject().value("item_5").toString() ));

    //player 3
    ui->direItems_3_1->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_0").toString() ));
    ui->direItems_3_2->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_1").toString() ));
    ui->direItems_3_3->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_2").toString() ));
    ui->direItems_3_4->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_3").toString() ));
    ui->direItems_3_5->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_4").toString() ));
    ui->direItems_3_6->setPixmap(getImage( "items", direSlots.at(2).toObject().value("item_5").toString() ));

    //player 4
    ui->direItems_4_1->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_0").toString() ));
    ui->direItems_4_2->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_1").toString() ));
    ui->direItems_4_3->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_2").toString() ));
    ui->direItems_4_4->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_3").toString() ));
    ui->direItems_4_5->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_4").toString() ));
    ui->direItems_4_6->setPixmap(getImage( "items", direSlots.at(3).toObject().value("item_5").toString() ));

    //player 5
    ui->direItems_5_1->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_0").toString() ));
    ui->direItems_5_2->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_1").toString() ));
    ui->direItems_5_3->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_2").toString() ));
    ui->direItems_5_4->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_3").toString() ));
    ui->direItems_5_5->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_4").toString() ));
    ui->direItems_5_6->setPixmap(getImage( "items", direSlots.at(4).toObject().value("item_5").toString() ));

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
    progressDialog->close();
    progressDialog->deleteLater();
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_viewMatchButton_clicked()
{
    //reset blocking, so the new requests work
    block = false;

    //check if apiKey is not set
    if(apiKey.isEmpty())
    {
        QMessageBox::information(this, "Api Key", "Make sure you set your api key in the preferences");
        return;
    }
    progressDialog = new QProgressDialog(this);
    progressDialog->setRange(0,0);
    progressDialog->setLabelText("Loading...");

    queryModel.setQuery("SELECT * FROM replays");
    QString matchID = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString().remove(".dem");
    downloadMatch(matchID);
    progressDialog->exec();

    //use QNetworkDiskCache instead of my method for ease
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
    pref.setDir(settings->value("replayFolder").toString());
    pref.setApiKey(apiKey);
    if(pref.exec())
    {
        dir = pref.getDir();
        apiKey = pref.getApiKey();
        settings->setValue("replayFolder", pref.getDir());
        settings->setValue("apiKey", apiKey);
        settings->sync();
        addFilesToDb();
        //QMessageBox::information(this, "Info", "Please Restart The Program To Reload The New Folder.\nThis is jsut a limitation until I finish this part of the program");
    }
}

void MainWindow::on_actionClear_Cache_triggered()
{
    QNetworkDiskCache *cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(userDir.absolutePath() + "/cache");
    cache->clear();

    delete cache;
}

void MainWindow::on_deleteReplayButton_clicked()
{
    queryModel.setQuery("SELECT * FROM replays");
    QString filename = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString();
    QFile::remove(dir.absolutePath() + "/" + filename);
}

void MainWindow::downloadMatch(QString id)
{
    QNetworkDiskCache *cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(userDir.absolutePath() + "/cache");
    manager = new QNetworkAccessManager(this);
    manager->setCache(cache);

    QNetworkRequest req(QUrl("https://computerfr33k-dota-2-replay-manager.p.mashape.com/json-mashape.php?match_id=" + id));
    req.setRawHeader(QByteArray("X-Mashape-Authorization"), apiKey.toLatin1());
    req.setRawHeader("User-Agent","d2rm-app");
    reply = manager->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError()));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError> errors)));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About"), tr("Version: %1 \nCreated by: Computerfr33k").arg(QApplication::applicationVersion()));
}

void MainWindow::on_actionWebsite_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.dota2replay-manager.com"));
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    if(index.row() < 0)
    {
        ui->watchReplay->setEnabled(false);
        ui->editTitle->setEnabled(false);
        ui->viewMatchButton->setEnabled(false);
        ui->deleteReplayButton->setEnabled(false);
    }
    else
    {
        ui->watchReplay->setEnabled(true);
        ui->editTitle->setEnabled(true);
        ui->viewMatchButton->setEnabled(true);
        ui->deleteReplayButton->setEnabled(true);
    }
}

void MainWindow::on_refreshButton_clicked()
{
    addFilesToDb();
}

void MainWindow::networkError()
{
    block = true;
    qDebug() << reply->errorString();
    reply->deleteLater();
    progressDialog->close();
    progressDialog->deleteLater();
    manager->deleteLater();
}

void MainWindow::sslError(QList<QSslError> errors)
{
    block = true;
    qDebug() << errors;
    reply->deleteLater();
    progressDialog->close();
    progressDialog->deleteLater();
    manager->deleteLater();
}
