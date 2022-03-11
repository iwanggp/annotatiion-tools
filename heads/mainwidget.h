#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include<QtWidgets/QWidget>
#include<QtWidgets/QPushButton>
#include<QtCore/QDebug>
#include "subwidget.h"//子窗口头文件
#include "detwindow.h"
class MainWidget:public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent=0);
    ~MainWidget();


public:
    DetWindow subWin;
    QPushButton * mainButton;
private slots:
    void tomainSlot();
    void tosubSlot();
};


#endif // MAINWIDGET_H
