#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtGui>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QStringList>
#include <QSqlQuery>
#include <iterator>
#include <QPixmap>
#include <QMessageBox>
#include <QSqlRecord>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QSslError>
#include <QDebug>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#include "edittitle.h"
#include "preferences.h"
#include "http.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void start();
    void checkDb();
    void addFilesToDb();

    void downloadMatch(QString);
    void setPicksBans();                //to display picks and bans for CM games

private slots:
    QPixmap getImage(QString type, QString name);
    void on_watchReplay_clicked();
    //void httpFinished();
    void on_viewMatchButton_clicked();
    void on_editTitle_clicked();
    void on_actionPreferences_triggered();
    void on_actionClear_Cache_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionWebsite_triggered();
    void on_tableView_clicked(const QModelIndex &index);
    void on_refreshButton_clicked();
    void on_deleteReplayButton_clicked();
    void on_actionCheck_For_Updates_triggered();
    void networkError();
    void sslError();
    void setMatchInfo();

private:
    //array of labels for UI
    //use array because it will be less code and allow us to iterate through them with for loops.
    QLabel *radiantBansUI[5];
    QLabel *radiantPicksUI[5];
    QLabel *direPicksUI[5];
    QLabel *direBansUI[5];
    QLabel *radiantHeroPicUI[5];
    QLabel *playerNameUI[2][5];
    QLabel *playerLevelUI[2][5];
    QLabel *playerHeroNameUI[2][5];
    QLabel *playerheroPicUI[2][5];
    QLabel *playerKillsUI[2][5];
    QLabel *playerDeathsUI[2][5];
    QLabel *playerAssistsUI[2][5];
    QLabel *playerItemsUI[2][5][6];     // 3-D Array for items [team][player][item_slot]
    QLabel *playerGoldUI[2][5];
    QLabel *playerLastHitsUI[2][5];
    QLabel *playerDeniesUI[2][5];
    QLabel *playerGPMUI[2][5];
    QLabel *playerXPMUI[2][5];
    //

    QSettings *settings;
    QDir dir;                           //replay Dir
    QDir userDir;                       //AppData Location for storing program settings
    QFont font;
    Ui::MainWindow *ui;
    QSqlDatabase db;                    //for the database of files and names
    QSqlTableModel *model;
    QSqlQueryModel queryModel;          //for querying the sqlite3 db
    QStringList list;                   //list of replay files
    QNetworkAccessManager *manager;     //manager for network connections
    QNetworkReply *reply;               //http reply
    QString picDir;                     //dir where images are located. (./thumbnails)
    QString apiKey;
    QPixmap image;
    QPixmap *emptyPicture;              //QPixmap object that is empty, useful so we only need one object for empty images instead of multiple.
    QProgressDialog *progressDialog;
    bool block;                         //if true, block all network requests
    Http http;
};

#endif // MAINWINDOW_H
