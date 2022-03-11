#include "task.h"
#include "ui_task.h"
#include <QDebug>
Task::Task(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Task)
{
    ui->setupUi(this);
    ui->detBt->setStyleSheet("QPushButton{border-radius:5px;background:rgb(150, 190, 60);color:blue;font-size:15px;}");
    ui->segBt->setStyleSheet("QPushButton{border-radius:5px;background:rgb(150, 190, 60);color:red;font-size:15px;");
    ui->detOCR->setStyleSheet("QPushButton{border-radius:5px;background:rgb(150, 190, 60);color:red;font-size:15px;");
    ui->classBSt->setStyleSheet("QPushButton{border-radius:5px;background:rgb(150, 190, 60);color:red;font-size:15px;");

    detWin = nullptr;
    segWin = nullptr;
    ocrWin=nullptr;
    classWin=nullptr;
    //按钮信号与本类的槽函数关联
    connect(ui->detBt,SIGNAL(clicked(bool)),this,SLOT(detect_task()));
    connect(ui->segBt,SIGNAL(clicked(bool)),this,SLOT(seg_task()));
    connect(ui->detOCR,SIGNAL(clicked(bool)),this,SLOT(ocr_task()));
    connect(ui->classBt,SIGNAL(clicked(bool)),this,SLOT(class_task()));


}


void Task::detect_task()
{
    detWin = new DetWindow();
    connect(detWin,SIGNAL(goBack()),this,SLOT(goHomeSlot()));//这里接受到信号
    this->hide();
    detWin->show();
    emit boxTask();

}

void Task::goHomeSlot()
{
    this->show();
    if(detWin) delete detWin;
    if(segWin) delete segWin;
    if(ocrWin) delete ocrWin;
    if(classWin) delete classWin;
    detWin = nullptr;
    segWin = nullptr;
    ocrWin=nullptr;
    classWin=nullptr;
}
void Task::seg_task()
{
    segWin = new SegWindow();
    connect(segWin,SIGNAL(goBack()),this,SLOT(goHomeSlot()));//这里接受到信号
    this->hide();
    segWin->show();
}

void Task::ocr_task()
{
    ocrWin = new OCRWindow();
    connect(ocrWin,SIGNAL(goBack()),this,SLOT(goHomeSlot()));//这里接受到信号
    this->hide();
    ocrWin->show();
}

void Task::class_task()
{
    classWin=new ClassWindow();
    connect(classWin,SIGNAL(goBack()),this,SLOT(goHomeSlot()));//这里接受到信号
    this->hide();
    classWin->show();
}

Task::~Task()
{
    delete ui;
}

