#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QStringList>
#include <QSqlQuery>
#include <iterator>
#include <QPixmap>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkDiskCache>
#include <QSqlRecord>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QSslError>
#include <QDebug>

#include "edittitle.h"
#include "preferences.h"

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
    void setMatchInfo(QJsonDocument);
    void downloadMatch(QString);
    void setPicksBans();                //to display picks and bans for CM games
    
private slots:
    QPixmap getImage(QString type, QString name);
    void on_watchReplay_clicked();
    void httpFinished();
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

private:
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
    QProgressDialog *progressDialog;
    bool block;                         //if true, block all network requests
};

#endif // MAINWINDOW_H
