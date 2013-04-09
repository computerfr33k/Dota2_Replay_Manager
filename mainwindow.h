#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void on_pushButton_clicked();
    void httpFinished();
    void readyRead();
    void renameReplay();
    void on_insertRow_Button_clicked();
    void on_removeRow_Button_clicked();
    void on_refreshReplayListButton_clicked();
    
private:
    bool listFileAndDirectory(QDir dir);
    void on_pushButton_start_clicked();

    QString dir;
    QStringList replayList;
    QStringListModel *model;
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QJsonDocument json;
    QSettings *settings;
    QSqlDatabase db;
    QSqlTableModel *model_2;
};

#endif // MAINWINDOW_H
