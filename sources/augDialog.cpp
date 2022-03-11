#include "augDialog.h"
//#include "ui_mydialog.h"
#include <QHBoxLayout>
#include<QDebug>
AugDialog::AugDialog(QWidget *parent)
    : QDialog(parent)
{
    //增强数据的微调框和滑动条
    nums_label=new QLabel(this);//增强的图片基数
    nums_label->setText("nums:");
    nums_pSpinBox=new QSpinBox(this);
    nums_pSpinBox->setMinimum(0);
    nums_pSpinBox->setMaximum(10);
    nums_pSpinBox->setSingleStep(5);
    nums_slider=new QSlider(Qt::Horizontal);
    nums_slider->setMinimum(0);
    nums_slider->setMaximum(10);
    nums_slider->setSingleStep(5);
    connect(nums_slider, SIGNAL(valueChanged(int)), this, SLOT(setSliderValue(int)));
    connect(nums_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSpinBox(int)));
    //旋转角度的微调框和滑动条
    ang_label=new QLabel(this);
    ang_label->setText("angle: ");//旋转的角度
    ang_pSpinBox=new QSpinBox(this);
    ang_pSpinBox->setMinimum(0);
    ang_pSpinBox->setMaximum(30);
    ang_pSpinBox->setSingleStep(5);
    ang_slider=new QSlider(Qt::Horizontal);
    ang_slider->setMinimum(0);
    ang_slider->setMaximum(30);
    ang_slider->setSingleStep(5);
    connect(ang_slider, SIGNAL(valueChanged(int)), this, SLOT(setAngSliderValue(int)));
    connect(ang_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setAngSpinBox(int)));
    //是否选择90度和180度翻转
    rotate_label=new QLabel(this);//选择翻转角度
    rotate_label->setText("degree option: ");
    verti_flip = new QCheckBox(this);
    verti_flip->setText(QString::fromLocal8Bit("90 degree"));
    connect(verti_flip, SIGNAL(stateChanged(int)), this, SLOT(onVerStateChanged(int)));
    rote_flip = new QCheckBox(this);
    rote_flip->setText(QString::fromLocal8Bit("180 degree"));
    connect(rote_flip, SIGNAL(stateChanged(int)), this, SLOT(onRoteStateChanged(int)));
    mir_flip = new QCheckBox(this);
    mir_flip->setText(QString::fromLocal8Bit("270 degree"));
    connect(mir_flip, SIGNAL(stateChanged(int)), this, SLOT(onMirStateChanged(int)));
    //调整gv差值
    gv_label=new QLabel(this);//选择翻转角度
    gv_label->setText("gv option:");
    _editGvDir=new QLineEdit(this);
    connect(_editGvDir, SIGNAL(returnPressed()), this, SLOT(getGVDir()));
//    gv_qCB = new QCheckBox(this);
//    gv_qCB->setText(QString::fromLocal8Bit("gv value"));
//    connect(gv_qCB, SIGNAL(stateChanged(int)), this, SLOT(onGVStateChanged(int)));
    //确定和取消按钮
    confirm=new QPushButton(this);
    confirm->setText("OK");
    cancle=new QPushButton(this);
    cancle->setText("Cancle");
    connect(confirm,SIGNAL(clicked()),this,SLOT(on_confirm_clicked()));
    connect(cancle,SIGNAL(clicked()),this,SLOT(on_cancle_clicked()));
    QGridLayout *layout = new QGridLayout();//网格布局
    layout->addWidget(nums_label,0,0,1,1);
    layout->addWidget(nums_pSpinBox,0,1,1,1);
    layout->addWidget(nums_slider,0,2,1,1);
    layout->addWidget(ang_label,0,3,1,1);
    layout->addWidget(ang_pSpinBox,0,4,1,1);
    layout->addWidget(ang_slider,0,5,1,1);
    layout->addWidget(rotate_label,1,0,1,1);
    layout->addWidget(verti_flip,1,1,1,1);
    layout->addWidget(rote_flip,1,2,1,1);
    layout->addWidget(mir_flip,1,3,1,1);
    layout->addWidget(gv_label,2,0,1,1);
    layout->addWidget(_editGvDir,2,1,2,1);
    layout->addWidget(nullptr,3,0,2,1);
    layout->addWidget(confirm,3,1,2,1);
    layout->setSpacing(25);
    layout->addWidget(cancle,3,2,2,1);
    this->setLayout(layout);
//    setFixedSize(this->width(), this->height());
    this->setWindowTitle("Select AugMent Parameter");
}

int AugDialog::getNumsValue()
{
    return num_value;
}
//调整增强的总数目
void AugDialog::setSliderValue(int value)
{
    num_value = nums_slider->value();
    nums_pSpinBox->setValue(num_value);
    qDebug()<<"the num of aug is ....."<<num_value;
}
void AugDialog::setSpinBox(int value)
{
    num_value=nums_pSpinBox->value();
    nums_slider->setValue(num_value);
    qDebug()<<"the num of aug is....."<<num_value;
}

//调整增强的总数目
void AugDialog::setAngSliderValue(int value)
{
    degree_value = ang_slider->value();
    ang_pSpinBox->setValue(degree_value);
    qDebug()<<"the num of aug is....."<<degree_value;
}
void AugDialog::setAngSpinBox(int value)
{
    degree_value=ang_pSpinBox->value();
    ang_slider->setValue(degree_value);
    qDebug()<<"the num of aug is....."<<degree_value;
}

//void AugDialog::onGVStateChanged(int state)
//{
//    if (state == Qt::Checked) // "选中状态"
//    {
//        is_gv=true;
//    }else // 未选中 - Qt::Unchecked
//    {
//        is_gv=false;
//    }
//}

void AugDialog::onVerStateChanged(int state)
{
    if (state == Qt::Checked) // "选中状态"
    {
        is_verti=true;
    }else // 未选中 - Qt::Unchecked
    {
        is_verti=false;
    }

}

void AugDialog::onRoteStateChanged(int state)
{
    if (state == Qt::Checked) // "选中状态"
    {
        is_rote=true;
    }else // 未选中 - Qt::Unchecked
    {
        is_rote=false;
    }
}

void AugDialog::onMirStateChanged(int state)
{
    if (state == Qt::Checked) // "选中状态"
    {
        is_mir=true;
    }else // 未选中 - Qt::Unchecked
    {
        is_mir=false;
    }
}

void AugDialog::getGVDir()
{
    gv_dir = _editGvDir->text();
    qDebug()<<"bug........"<<gv_dir;
}

void AugDialog::on_confirm_clicked()
{
    this->close();
    gv_dir = _editGvDir->text();
    qDebug()<<"the num is ......."<<num_value<<"the ang is ......"<<degree_value<<"is is_verti......"<<is_verti<<"is_rote ......."<<is_rote<<"is_gv ......."<<gv_dir;
    emit augConfirm(num_value,degree_value,is_verti,is_rote,is_mir,gv_dir);

}

void AugDialog::on_cancle_clicked()
{
    qDebug()<<"cancle.........";
    this->close();//关闭窗口
}
AugDialog::~AugDialog()
{
    qDebug()<<"run...................";
    this->setAttribute(Qt::WA_DeleteOnClose, true);

}

