#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //create blank image for empty item slots
    image = QPixmap(QSize(32,24));
    image.fill(Qt::black);

    initializeUIPointers();
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
    delete model;
    delete ui;
}

void MainWindow::start()
{
    userDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0);
    downloadsDir = userDir.path() + "/downloads";

    //create the folder if it doesn't already exist
    if(!QDir(userDir).exists())
    {
        userDir.mkpath(userDir.absolutePath());
    }

    //create folder for storing downloads if it doesn't exist
    if(!downloadsDir.exists())
    {
        downloadsDir.mkpath(downloadsDir.path());
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
        //qDebug() << param;
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
    //Disable buttons since nothing will be selected
    ui->watchReplay->setEnabled(false);
    ui->editTitle->setEnabled(false);
    ui->viewMatchButton->setEnabled(false);
    ui->deleteReplayButton->setEnabled(false);

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
    //return empty QPixmap because the item slot was empty, so no need to try and fetch something that will fail and waste time for a failed request.
    if(name.compare("empty") == 0)
        return image;

    QPixmap pic;
    QString size;
    int width;

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
    pic.load("downloads/" + name + "_" + size + ".png");

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

void MainWindow::setMatchInfo()
{
    //we need to make sure we don't use this slot anymore since this is in the slot.
    //if we kept this slot enabled we would keep calling this everytime a download is finished and continous loop.
    http.disconnect();

    ui->tabWidget->setCurrentIndex(0);

    QString matchID = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString().remove(".dem");

    //test matchInfo parse class
    matchInfo MatchParser;
    MatchParser.parse(downloadsDir.path() + "/" + matchID + ".json");

    //display basic match info
    ui->winner->setText( MatchParser.getMatchWinner() );
    ui->matchID->setText( MatchParser.getMatchID() );
    ui->gameMode->setText( MatchParser.getGameMode() );
    ui->startTime->setText( MatchParser.getStartTime() );
    ui->lobbyType->setText( MatchParser.getLobbyType() );
    ui->duration->setText( MatchParser.getDuration() );
    ui->fbTime->setText( MatchParser.getFirstBloodTime() );

    //if CM, then display picks & bans
    if(MatchParser.getGameMode().compare("Captains Mode") == 0)
    {
        for(int i=0; i < 5; i++)
        {
            radiantBansUI[i]->setText("<img src=\"" + downloadsDir.path() + "/" + MatchParser.getBans()[0][i] + "_sb.png\" width=\"45\" />");
            radiantPicksUI[i]->setText("<img src=\"" + downloadsDir.path() + "/" + MatchParser.getPicks()[0][i] + "_sb.png\" width=\"45\" />");
        }

        for(int i=0; i<5; i++)
        {
            //Pixmap display higher quality, but using html <img> is easier but does not display as good of quality
            //QPixmap pic;
            //pic.load("downloads/" + MatchParser.getBans()[1][i] + "_sb.png");

            direBansUI[i]->setText("<img src=\"" + downloadsDir.path() + "/" + MatchParser.getBans()[1][i] + "_sb.png\" width=\"45\" />");
            direPicksUI[i]->setText("<img src=\"" + downloadsDir.path() + "/" + MatchParser.getPicks()[1][i] + "_sb.png\" width=\"45\" />");
        }
    }
    else //match was not CM, clear images
    {
        for(int i=0; i<5; i++)
        {
            radiantBansUI[i]->clear();
            radiantPicksUI[i]->clear();
            direBansUI[i]->clear();
            direPicksUI[i]->clear();
        }
    }

    for(int i=0; i<2; i++)
        for(int j=0; j<5; j++)
        {
            //qDebug() << "<img src=\"file:///" + downloadsDir.path() + "/" + MatchParser.getPlayerHeroName()[i][j].value("name").toString() + "_sb.png\" width=\"45\" />";
            playerNameUI[i][j]->setText( MatchParser.getPlayerNames()[i][j] );
            playerLevelUI[i][j]->setText( MatchParser.getPlayerLevel()[i][j] );
            playerHeroPicUI[i][j]->setText( "<img src=\"" + downloadsDir.path() + "/" + MatchParser.getPlayerHeroName()[i][j].value("name").toString() + "_sb.png\" width=\"45\" />" );
            playerHeroNameUI[i][j]->setText( MatchParser.getPlayerHeroName()[i][j].value("localized_name").toString() );
            playerKillsUI[i][j]->setText( MatchParser.getPlayerKills()[i][j] );
            playerDeathsUI[i][j]->setText( MatchParser.getPlayerDeaths()[i][j] );
            playerAssistsUI[i][j]->setText( MatchParser.getPlayerAssists()[i][j] );
            playerGoldUI[i][j]->setText( MatchParser.getPlayerGold()[i][j] );
            playerLastHitsUI[i][j]->setText( MatchParser.getPlayerLH()[i][j] );
            playerDeniesUI[i][j]->setText( MatchParser.getPlayerDN()[i][j] );
            playerGPMUI[i][j]->setText( MatchParser.getPlayerGPM()[i][j] );
            playerXPMUI[i][j]->setText( MatchParser.getPlayerXPM()[i][j] );

            //display items
            for(int k=0; k<6; k++)
            {
                //only display image if it is not empty
                if(MatchParser.getPlayerItems()[i][j][k] != "empty")
                    playerItemsUI[i][j][k]->setText( "<img src=\"" + downloadsDir.path() + "/" + MatchParser.getPlayerItems()[i][j][k] + "_lg.png\" width=\"32\"/>" );
            }
        }

    ui->statusBar->showMessage("Loading Complete!", 30000);     //display message in status bar for 30 sec.
}

void MainWindow::on_viewMatchButton_clicked()
{
    //check if apiKey is not set
    if(apiKey.isEmpty())
    {
        QMessageBox::information(this, "Api Key", "Make sure you set your api key in the preferences");
        return;
    }
    ui->statusBar->showMessage("Loading...");

    queryModel.setQuery("SELECT * FROM replays");
    QString matchID = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString().remove(".dem");
    downloadMatch(matchID);
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
    }
}

void MainWindow::on_actionClear_Cache_triggered()
{
    QDir cache(downloadsDir.path());
    QMessageBox::information(this, tr("Clear Cache"), cache.removeRecursively() ? tr("cache cleared successfully") : tr("cache was not cleared successfully") );
    cache.mkdir(downloadsDir.path());
}

void MainWindow::on_deleteReplayButton_clicked()
{
    queryModel.setQuery("SELECT * FROM replays");
    QString filename = queryModel.record(ui->tableView->selectionModel()->currentIndex().row()).value("filename").toString();
    QFile::remove(dir.absolutePath() + "/" + filename);
    addFilesToDb();
    ui->deleteReplayButton->setEnabled(false);
}

void MainWindow::downloadMatch(QString id)
{
    http.append("https://computerfr33k-dota-2-replay-manager.p.mashape.com/json-mashape.php?match_id=" + id);
    http.setRawHeader(QByteArray("X-Mashape-Authorization"), apiKey.toLatin1());
    connect(&http, SIGNAL(finished()), SLOT(setMatchInfo()));
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

void MainWindow::on_actionCheck_For_Updates_triggered()
{
#ifdef Q_OS_WIN32
    //open the updater with admin priv, so if there is an update we can download and install it.
    QDesktopServices::openUrl(QUrl("file:///" + QDir::currentPath() + "/autoupdate/autoupdate-windows.exe", QUrl::TolerantMode));
#endif
#ifdef Q_OS_LINUX
    QProcess::startDetached("autoupdate/autoupdate-linux.run");
#endif
}

void MainWindow::networkError()
{
}

void MainWindow::sslError()
{
}

void MainWindow::on_actionTutorial_triggered()
{
    FirstRun *fr = new FirstRun;
    fr->show();
    //delete memory once the dialog is closed
    connect(fr, SIGNAL(rejected()), fr, SLOT(deleteLater()));
}

void MainWindow::initializeUIPointers()
{
    //create our QLabel array so we can for loop our match info
    // 2-D Array of QLabels; first index: 0 = radiant, 1 = Dire
    radiantBansUI[0] = ui->radiantBan_1;
    radiantBansUI[1] = ui->radiantBan_2;
    radiantBansUI[2] = ui->radiantBan_3;
    radiantBansUI[3] = ui->radiantBan_4;
    radiantBansUI[4] = ui->radiantBan_5;

    radiantPicksUI[0] = ui->radiantPick_1;
    radiantPicksUI[1] = ui->radiantPick_2;
    radiantPicksUI[2] = ui->radiantPick_3;
    radiantPicksUI[3] = ui->radiantPick_4;
    radiantPicksUI[4] = ui->radiantPick_5;

    radiantHeroPicUI[0] = ui->radiantHeroPic_1;
    radiantHeroPicUI[1] = ui->radiantHeroPic_2;
    radiantHeroPicUI[2] = ui->radiantHeroPic_3;
    radiantHeroPicUI[3] = ui->radiantHeroPic_4;
    radiantHeroPicUI[4] = ui->radiantHeroPic_5;

    direBansUI[0] = ui->direBan_1;
    direBansUI[1] = ui->direBan_2;
    direBansUI[2] = ui->direBan_3;
    direBansUI[3] = ui->direBan_4;
    direBansUI[4] = ui->direBan_5;

    direPicksUI[0] = ui->direPick_1;
    direPicksUI[1] = ui->direPick_2;
    direPicksUI[2] = ui->direPick_3;
    direPicksUI[3] = ui->direPick_4;
    direPicksUI[4] = ui->direPick_5;

    //radiant Player Names
    playerNameUI[0][0] = ui->radiantPlayer_1;
    playerNameUI[0][1] = ui->radiantPlayer_2;
    playerNameUI[0][2] = ui->radiantPlayer_3;
    playerNameUI[0][3] = ui->radiantPlayer_4;
    playerNameUI[0][4] = ui->radiantPlayer_5;
    //Dire Player Names
    playerNameUI[1][0] = ui->direPlayer_1;
    playerNameUI[1][1] = ui->direPlayer_2;
    playerNameUI[1][2] = ui->direPlayer_3;
    playerNameUI[1][3] = ui->direPlayer_4;
    playerNameUI[1][4] = ui->direPlayer_5;

    //radiant levels
    playerLevelUI[0][0] = ui->radiantLevel_1;
    playerLevelUI[0][1] = ui->radiantLevel_2;
    playerLevelUI[0][2] = ui->radiantLevel_3;
    playerLevelUI[0][3] = ui->radiantLevel_4;
    playerLevelUI[0][4] = ui->radiantLevel_5;
    //dire Levels
    playerLevelUI[1][0] = ui->direLevel_1;
    playerLevelUI[1][1] = ui->direLevel_2;
    playerLevelUI[1][2] = ui->direLevel_3;
    playerLevelUI[1][3] = ui->direLevel_4;
    playerLevelUI[1][4] = ui->direLevel_5;

    //radiant Hero Pic
    playerHeroPicUI[0][0] = ui->radiantHeroPic_1;
    playerHeroPicUI[0][1] = ui->radiantHeroPic_2;
    playerHeroPicUI[0][2] = ui->radiantHeroPic_3;
    playerHeroPicUI[0][3] = ui->radiantHeroPic_4;
    playerHeroPicUI[0][4] = ui->radiantHeroPic_5;
    //dire hero pic
    playerHeroPicUI[1][0] = ui->direHeroPic_1;
    playerHeroPicUI[1][1] = ui->direHeroPic_2;
    playerHeroPicUI[1][2] = ui->direHeroPic_3;
    playerHeroPicUI[1][3] = ui->direHeroPic_4;
    playerHeroPicUI[1][4] = ui->direHeroPic_5;

    //radiant hero name
    playerHeroNameUI[0][0] = ui->radiantHero_1;
    playerHeroNameUI[0][1] = ui->radiantHero_2;
    playerHeroNameUI[0][2] = ui->radiantHero_3;
    playerHeroNameUI[0][3] = ui->radiantHero_4;
    playerHeroNameUI[0][4] = ui->radiantHero_5;
    //dire hero name
    playerHeroNameUI[1][0] = ui->direHero_1;
    playerHeroNameUI[1][1] = ui->direHero_2;
    playerHeroNameUI[1][2] = ui->direHero_3;
    playerHeroNameUI[1][3] = ui->direHero_4;
    playerHeroNameUI[1][4] = ui->direHero_5;

    //radiant Kills
    playerKillsUI[0][0] = ui->radiantKills_1;
    playerKillsUI[0][1] = ui->radiantKills_2;
    playerKillsUI[0][2] = ui->radiantKills_3;
    playerKillsUI[0][3] = ui->radiantKills_4;
    playerKillsUI[0][4] = ui->radiantKills_5;
    //dire kills
    playerKillsUI[1][0] = ui->direKills_1;
    playerKillsUI[1][1] = ui->direKills_2;
    playerKillsUI[1][2] = ui->direKills_3;
    playerKillsUI[1][3] = ui->direKills_4;
    playerKillsUI[1][4] = ui->direKills_5;

    //radiant Deaths
    playerDeathsUI[0][0] = ui->radiantDeaths_1;
    playerDeathsUI[0][1] = ui->radiantDeaths_2;
    playerDeathsUI[0][2] = ui->radiantDeaths_3;
    playerDeathsUI[0][3] = ui->radiantDeaths_4;
    playerDeathsUI[0][4] = ui->radiantDeaths_5;
    //dire Deaths
    playerDeathsUI[1][0] = ui->direDeaths_1;
    playerDeathsUI[1][1] = ui->direDeaths_2;
    playerDeathsUI[1][2] = ui->direDeaths_3;
    playerDeathsUI[1][3] = ui->direDeaths_4;
    playerDeathsUI[1][4] = ui->direDeaths_5;

    //radiant Assists
    playerAssistsUI[0][0] = ui->radiantAssists_1;
    playerAssistsUI[0][1] = ui->radiantAssists_2;
    playerAssistsUI[0][2] = ui->radiantAssists_3;
    playerAssistsUI[0][3] = ui->radiantAssists_4;
    playerAssistsUI[0][4] = ui->radiantAssists_5;
    //dire Assists
    playerAssistsUI[1][0] = ui->direAssists_1;
    playerAssistsUI[1][1] = ui->direAssists_2;
    playerAssistsUI[1][2] = ui->direAssists_3;
    playerAssistsUI[1][3] = ui->direAssists_4;
    playerAssistsUI[1][4] = ui->direAssists_5;

    //radiant items 0-5
    //player 1
    playerItemsUI[0][0][0] = ui->radiantItems_1_1;
    playerItemsUI[0][0][1] = ui->radiantItems_1_2;
    playerItemsUI[0][0][2] = ui->radiantItems_1_3;
    playerItemsUI[0][0][3] = ui->radiantItems_1_4;
    playerItemsUI[0][0][4] = ui->radiantItems_1_5;
    playerItemsUI[0][0][5] = ui->radiantItems_1_6;
    //player 2
    playerItemsUI[0][1][0] = ui->radiantItems_2_1;
    playerItemsUI[0][1][1] = ui->radiantItems_2_2;
    playerItemsUI[0][1][2] = ui->radiantItems_2_3;
    playerItemsUI[0][1][3] = ui->radiantItems_2_4;
    playerItemsUI[0][1][4] = ui->radiantItems_2_5;
    playerItemsUI[0][1][5] = ui->radiantItems_2_6;
    //player 3
    playerItemsUI[0][2][0] = ui->radiantItems_3_1;
    playerItemsUI[0][2][1] = ui->radiantItems_3_2;
    playerItemsUI[0][2][2] = ui->radiantItems_3_3;
    playerItemsUI[0][2][3] = ui->radiantItems_3_4;
    playerItemsUI[0][2][4] = ui->radiantItems_3_5;
    playerItemsUI[0][2][5] = ui->radiantItems_3_6;
    //player 4
    playerItemsUI[0][3][0] = ui->radiantItems_4_1;
    playerItemsUI[0][3][1] = ui->radiantItems_4_2;
    playerItemsUI[0][3][2] = ui->radiantItems_4_3;
    playerItemsUI[0][3][3] = ui->radiantItems_4_4;
    playerItemsUI[0][3][4] = ui->radiantItems_4_5;
    playerItemsUI[0][3][5] = ui->radiantItems_4_6;
    //player 5
    playerItemsUI[0][4][0] = ui->radiantItems_5_1;
    playerItemsUI[0][4][1] = ui->radiantItems_5_2;
    playerItemsUI[0][4][2] = ui->radiantItems_5_3;
    playerItemsUI[0][4][3] = ui->radiantItems_5_4;
    playerItemsUI[0][4][4] = ui->radiantItems_5_5;
    playerItemsUI[0][4][5] = ui->radiantItems_5_6;
    //dire items
    //player 1
    playerItemsUI[1][0][0] = ui->direItems_1_1;
    playerItemsUI[1][0][1] = ui->direItems_1_2;
    playerItemsUI[1][0][2] = ui->direItems_1_3;
    playerItemsUI[1][0][3] = ui->direItems_1_4;
    playerItemsUI[1][0][4] = ui->direItems_1_5;
    playerItemsUI[1][0][5] = ui->direItems_1_6;
    //player 2
    playerItemsUI[1][1][0] = ui->direItems_2_1;
    playerItemsUI[1][1][1] = ui->direItems_2_2;
    playerItemsUI[1][1][2] = ui->direItems_2_3;
    playerItemsUI[1][1][3] = ui->direItems_2_4;
    playerItemsUI[1][1][4] = ui->direItems_2_5;
    playerItemsUI[1][1][5] = ui->direItems_2_6;
    //player 3
    playerItemsUI[1][2][0] = ui->direItems_3_1;
    playerItemsUI[1][2][1] = ui->direItems_3_2;
    playerItemsUI[1][2][2] = ui->direItems_3_3;
    playerItemsUI[1][2][3] = ui->direItems_3_4;
    playerItemsUI[1][2][4] = ui->direItems_3_5;
    playerItemsUI[1][2][5] = ui->direItems_3_6;
    //player 4
    playerItemsUI[1][3][0] = ui->direItems_4_1;
    playerItemsUI[1][3][1] = ui->direItems_4_2;
    playerItemsUI[1][3][2] = ui->direItems_4_3;
    playerItemsUI[1][3][3] = ui->direItems_4_4;
    playerItemsUI[1][3][4] = ui->direItems_4_5;
    playerItemsUI[1][3][5] = ui->direItems_4_6;
    //player 5
    playerItemsUI[1][4][0] = ui->direItems_5_1;
    playerItemsUI[1][4][1] = ui->direItems_5_2;
    playerItemsUI[1][4][2] = ui->direItems_5_3;
    playerItemsUI[1][4][3] = ui->direItems_5_4;
    playerItemsUI[1][4][4] = ui->direItems_5_5;
    playerItemsUI[1][4][5] = ui->direItems_5_6;

    //radiant Gold
    playerGoldUI[0][0] = ui->radiantGold_1;
    playerGoldUI[0][1] = ui->radiantGold_2;
    playerGoldUI[0][2] = ui->radiantGold_3;
    playerGoldUI[0][3] = ui->radiantGold_4;
    playerGoldUI[0][4] = ui->radiantGold_5;
    //dire Gold
    playerGoldUI[1][0] = ui->direGold_1;
    playerGoldUI[1][1] = ui->direGold_2;
    playerGoldUI[1][2] = ui->direGold_3;
    playerGoldUI[1][3] = ui->direGold_4;
    playerGoldUI[1][4] = ui->direGold_5;

    //radiant Last Hits
    playerLastHitsUI[0][0] = ui->radiantLH_1;
    playerLastHitsUI[0][1] = ui->radiantLH_2;
    playerLastHitsUI[0][2] = ui->radiantLH_3;
    playerLastHitsUI[0][3] = ui->radiantLH_4;
    playerLastHitsUI[0][4] = ui->radiantLH_5;
    //dire Last Hits
    playerLastHitsUI[1][0] = ui->direLH_1;
    playerLastHitsUI[1][1] = ui->direLH_2;
    playerLastHitsUI[1][2] = ui->direLH_3;
    playerLastHitsUI[1][3] = ui->direLH_4;
    playerLastHitsUI[1][4] = ui->direLH_5;

    //radiant Denies
    playerDeniesUI[0][0] = ui->radiantDN_1;
    playerDeniesUI[0][1] = ui->radiantDN_2;
    playerDeniesUI[0][2] = ui->radiantDN_3;
    playerDeniesUI[0][3] = ui->radiantDN_4;
    playerDeniesUI[0][4] = ui->radiantDN_5;
    //dire Denies
    playerDeniesUI[1][0] = ui->direDN_1;
    playerDeniesUI[1][1] = ui->direDN_2;
    playerDeniesUI[1][2] = ui->direDN_3;
    playerDeniesUI[1][3] = ui->direDN_4;
    playerDeniesUI[1][4] = ui->direDN_5;

    //radiant Gold/Min
    playerGPMUI[0][0] = ui->radiantGPM_1;
    playerGPMUI[0][1] = ui->radiantGPM_2;
    playerGPMUI[0][2] = ui->radiantGPM_3;
    playerGPMUI[0][3] = ui->radiantGPM_4;
    playerGPMUI[0][4] = ui->radiantGPM_5;
    //dire Gold/Min
    playerGPMUI[1][0] = ui->direGPM_1;
    playerGPMUI[1][1] = ui->direGPM_2;
    playerGPMUI[1][2] = ui->direGPM_3;
    playerGPMUI[1][3] = ui->direGPM_4;
    playerGPMUI[1][4] = ui->direGPM_5;

    //radiant XPM
    playerXPMUI[0][0] = ui->radiantXPM_1;
    playerXPMUI[0][1] = ui->radiantXPM_2;
    playerXPMUI[0][2] = ui->radiantXPM_3;
    playerXPMUI[0][3] = ui->radiantXPM_4;
    playerXPMUI[0][4] = ui->radiantXPM_5;
    //dire XPM
    playerXPMUI[1][0] = ui->direXPM_1;
    playerXPMUI[1][1] = ui->direXPM_2;
    playerXPMUI[1][2] = ui->direXPM_3;
    playerXPMUI[1][3] = ui->direXPM_4;
    playerXPMUI[1][4] = ui->direXPM_5;
}
