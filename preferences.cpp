#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
}

Preferences::~Preferences()
{
    delete ui;
}

QString Preferences::getDir()
{
    return ui->lineEdit->text();
}

void Preferences::on_pushButton_clicked()
{
    QFileDialog fd;
    ui->lineEdit->setText(fd.getExistingDirectory(this, "", "C:/Program Files (x86)/Steam/SteamApps/common/dota 2 beta/dota/replays"));
}
