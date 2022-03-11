#ifndef TASK_H
#define TASK_H

#include <QDialog>
#include "detwindow.h"
#include "segwindow.h"
#include "ocrwindow.h"
#include "classwindow.h"
namespace Ui {
class Task;
}

class Task : public QDialog
{
    Q_OBJECT

public:
    explicit Task(QWidget *parent = 0);
    ~Task();

public:
    DetWindow *detWin;
    SegWindow *segWin;
    OCRWindow *ocrWin;

    ClassWindow *classWin;
private slots:
    void detect_task();
    void seg_task();
    void ocr_task();//OCR标注任务
    void class_task();//OCR标注任务
    void goHomeSlot();

signals://信号声明--不需实现，后面通过emit发送就可以了
    void mysignals();
    void boxTask();//目前是检测的标注任务
    void ocrTask();//OCR标记任务
    void classTask();//OCR标记任务
private:
    Ui::Task *ui;
};

#endif // TASK_H
