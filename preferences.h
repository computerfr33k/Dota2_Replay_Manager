#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT
    
public:
    explicit Preferences(QWidget *parent = 0);
    QString getDir();
    void setDir(QString);
    QFont getFont();
    void setFont(QFont);
    QString getApiKey();
    void setApiKey(QString&);
    ~Preferences();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::Preferences *ui;
};

#endif // PREFERENCES_H
