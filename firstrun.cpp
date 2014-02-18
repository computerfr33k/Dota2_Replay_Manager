#include "firstrun.h"
#include "ui_firstrun.h"

FirstRun::FirstRun(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstRun)
{
    ui->setupUi(this);
}

FirstRun::~FirstRun()
{
    delete ui;
}
