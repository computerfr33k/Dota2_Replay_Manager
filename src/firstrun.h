#ifndef FIRSTRUN_H
#define FIRSTRUN_H

#include <QDialog>

namespace Ui {
class FirstRun;
}

class FirstRun : public QDialog
{
    Q_OBJECT

public:
    explicit FirstRun(QWidget *parent = 0);
    ~FirstRun();

private:
    Ui::FirstRun *ui;
};

#endif // FIRSTRUN_H
