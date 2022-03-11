#ifndef SEGWINDOW_H
#define SEGWINDOW_H
#include<QWidget>

class Two;

namespace Ui {
class One;
}

class One:public QWidget
{
    Q_OBJECT


public:
    explicit One(QWidget *parent=0);
    ~One();

signals:

    void display(int number);
private:
    Ui::One *ui;
    Two *two;
};
#endif // SEGWINDOW_H
