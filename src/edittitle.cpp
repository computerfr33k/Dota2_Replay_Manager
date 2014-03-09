#include "edittitle.h"
#include "ui_edittitle.h"

EditTitle::EditTitle(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTitle)
{
    ui->setupUi(this);
}

EditTitle::~EditTitle()
{
    delete ui;
}

void EditTitle::setTitle(QString tmp)
{
    ui->lineEdit->setText(tmp);
}

QString EditTitle::getTitle()
{
    return ui->lineEdit->text();
}
