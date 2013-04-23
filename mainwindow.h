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
#include <QSqlRecord>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>

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
    
private slots:
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

private:
    QSettings *settings;
    QDir dir;
    QDir userDir;
    QFont font;
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *model;
    QSqlQueryModel queryModel;
    QStringList list;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QString picDir;
};

#endif // MAINWINDOW_H
