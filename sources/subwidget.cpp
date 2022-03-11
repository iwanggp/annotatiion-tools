#include "subwidget.h"
SubWidget::SubWidget(QWidget *parent):QWidget(parent)
{
    this->setWindowTitle("son");
    subButton=new QPushButton(this);
    subButton->setParent(this);
    subButton->setText(QString::fromLocal8Bit("切换到主窗口"));
    subButton->move(QPoint(50,50));
    subButton->setParent(this);

    connect(subButton,SIGNAL(clicked()),this,SLOT(changeSlot()));
}


void SubWidget::changeSlot()
{
    emit mySignal();
}
