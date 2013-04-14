#ifndef EDITTITLE_H
#define EDITTITLE_H

#include <QDialog>

namespace Ui {
class EditTitle;
}

class EditTitle : public QDialog
{
    Q_OBJECT
    
public:
    explicit EditTitle(QWidget *parent = 0);
    QString getTitle();
    void setTitle(QString);
    ~EditTitle();
    
private:
    Ui::EditTitle *ui;
};

#endif // EDITTITLE_H
