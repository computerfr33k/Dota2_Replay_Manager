#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>

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
    void on_pushButton_start_clicked();
    void on_pushButton_clicked();
    void httpFinished();
    void readyRead();
    void renameReplay();
    
private:
    bool listFileAndDirectory(QDir dir);

    QString dir;
    QStringList replayList;
    QStringListModel *model;
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QJsonDocument json;
    QSettings *settings;
};

#endif // MAINWINDOW_H
