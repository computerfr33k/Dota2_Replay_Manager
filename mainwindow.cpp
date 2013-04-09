#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new QStringListModel(replayList);
    ui->listView->setModel(model);
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    if(settings->value("firstRun", true).toBool())
    {
        QMessageBox::information(this, tr("Dota 2 Replay Manager First Run"), tr("Please Select Your Dota 2 Replay Folder\nHint: Where Dota 2 downloads your replays to"));
        on_pushButton_start_clicked();
        settings->setValue("firstRun", false);
    }
    settings->sync();
    dir = settings->value("replayFolder", "C:/Program Files (x86)/Steam/steamApps/common/dota 2 beta/dota/replays").toString();
    connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(renameReplay()));
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("replays.db");
    db.open();
    model_2 = new QSqlTableModel(this, db);
    renameReplay();
}

void MainWindow::on_pushButton_start_clicked()
{
    QFileDialog fd;
    dir = fd.getExistingDirectory(this, tr("Select Dota 2 Replays Folder"), "C:/Program Files (x86)/Steam/steamApps/common/dota 2 beta/dota/replays");
    settings->setValue("replayFolder", dir);
    settings->sync();
    replayList.clear();
    listFileAndDirectory(QDir(dir));
}

void MainWindow::on_pushButton_clicked()
{
    QString fileName = model->itemData(ui->listView->currentIndex()).value(0).toString();
    if(fileName.contains(".dem"))
        fileName.remove(".dem");
    else
    {
        settings->beginGroup("name");
        fileName = settings->value(fileName).toString();
        settings->endGroup();
    }

    manager = new QNetworkAccessManager(this);
    reply = manager->get(QNetworkRequest(QUrl("http://dota2.computerfr33k.com/json.php?match_id=" + fileName)));
    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void MainWindow::readyRead()
{
    QFile file("match.json");
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    file.write(reply->readAll());
    file.close();
}

void MainWindow::httpFinished()
{
    QByteArray tmp = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(tmp);
    qDebug() << json.toJson();
    if(json.object().value("success").toString().compare("0") == 0)
    {
        QMessageBox::warning(this, "Warning", "Could not fetch info about this match\nCould Be A Private Match");
        return;
    }

    //general match info
    ui->lineEdit->setText(json.object().value("match_id").toString());
    ui->lineEdit_2->setText(json.object().value("game_mode").toString());
    ui->lineEdit_3->setText(json.object().value("start_time").toString());
    ui->lineEdit_4->setText(json.object().value("lobby_type").toString());
    ui->lineEdit_5->setText(json.object().value("duration").toString());
    ui->lineEdit_6->setText(json.object().value("first_blood_time").toString());

    ui->radiantPlayer_1->setText(json.object().value("slots").toObject().value("radiant").toArray().at(0).toObject().value("account_name").toString());
    ui->radiantPlayer_2->setText(json.object().value("slots").toObject().value("radiant").toArray().at(1).toObject().value("account_name").toString());
    ui->radiantPlayer_3->setText(json.object().value("slots").toObject().value("radiant").toArray().at(2).toObject().value("account_name").toString());
    ui->radiantPlayer_4->setText(json.object().value("slots").toObject().value("radiant").toArray().at(3).toObject().value("account_name").toString());
    ui->radiantPlayer_5->setText(json.object().value("slots").toObject().value("radiant").toArray().at(4).toObject().value("account_name").toString());

    //player hero
    ui->lineEdit_15->setText(json.object().value("slots").toObject().value("radiant").toArray().at(0).toObject().value("hero").toObject().value("localized_name").toString());
    ui->lineEdit_11->setText(json.object().value("slots").toObject().value("radiant").toArray().at(1).toObject().value("hero").toObject().value("localized_name").toString());
    ui->lineEdit_12->setText(json.object().value("slots").toObject().value("radiant").toArray().at(2).toObject().value("hero").toObject().value("localized_name").toString());
    ui->lineEdit_13->setText(json.object().value("slots").toObject().value("radiant").toArray().at(3).toObject().value("hero").toObject().value("localized_name").toString());
    ui->lineEdit_16->setText(json.object().value("slots").toObject().value("radiant").toArray().at(4).toObject().value("hero").toObject().value("localized_name").toString());
    //level
    ui->spinBox->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(0).toObject().value("level").toString().toInt());
    ui->spinBox_2->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(1).toObject().value("level").toString().toInt());
    ui->spinBox_3->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(2).toObject().value("level").toString().toInt());
    ui->spinBox_4->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(3).toObject().value("level").toString().toInt());
    ui->spinBox_5->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(4).toObject().value("level").toString().toInt());
    //kills
    ui->spinBox_6->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(0).toObject().value("kills").toString().toInt());
    ui->spinBox_7->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(1).toObject().value("kills").toString().toInt());
    ui->spinBox_8->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(2).toObject().value("kills").toString().toInt());
    ui->spinBox_9->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(3).toObject().value("kills").toString().toInt());
    ui->spinBox_10->setValue(json.object().value("slots").toObject().value("radiant").toArray().at(4).toObject().value("kills").toString().toInt());
}

void MainWindow::renameReplay()
{
    QString name = model->itemData(ui->listView->currentIndex()).value(0).toString();
    qDebug() << name;
    model_2->setTable("replay");
    model_2->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setHeaderData(1, Qt::Horizontal, tr("Name"), Qt::NoTextInteraction);
    model_2->select();
    //model_2->removeColumn(0); // don't show the ID
    ui->replayNames_2->setModel(model_2);
    ui->replayNames_2->hideColumn(0);
}

void MainWindow::on_insertRow_Button_clicked()
{
    QSqlQuery query("insert into replay (name, filename) values (:name, :filename)");
    query.bindValue(0, "");
    query.bindValue(1, model->itemData(ui->listView->currentIndex()).value(0).toString());
    query.exec();
    model_2->select();
}

void MainWindow::on_removeRow_Button_clicked()
{
    model_2->removeRow(ui->replayNames_2->currentIndex().row());
    model_2->select();
}

void MainWindow::on_refreshReplayListButton_clicked()
{
    replayList.clear();
    listFileAndDirectory(dir);
}

bool MainWindow::listFileAndDirectory(QDir dir)
{
    bool ok = dir.exists();  //check if directory exist
        if ( ok )
        {
            //set fileinfo filter
            QFileInfoList entries = dir.entryInfoList( QDir::NoDotAndDotDot |
                    QDir::Dirs | QDir::Files );
                    //loop over entries filter selected
            foreach ( QFileInfo entryInfo, entries )
            {
                QString path = entryInfo.absoluteFilePath();
                QString fileName = entryInfo.fileName();

                if ( entryInfo.isDir() )    //check if entryInfo is dir
                {
                }
                else
                {
                    replayList.append(fileName);
                    model->setStringList(replayList);
                }
            }
        }

        if (ok && !dir.exists(dir.absolutePath()))
            ok = false;

        return ok;
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
    delete model;
    delete settings;
}
