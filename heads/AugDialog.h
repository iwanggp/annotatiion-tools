#ifndef MYDIALOG_H
#define MYDIALOG_H
#include<QDialog>
#include<QSpinBox>
#include<QSlider>
#include<QLabel>
#include<QCheckBox>
#include<QPushButton>
#include <QLineEdit>
QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class AugDialog : public QDialog
{
    Q_OBJECT
signals:
    void augConfirm(int num_value,int degree,bool is_verti,bool is_rote,bool is_mir,QString gv_dir);
public slots:
    void setSliderValue(int value);
    void setAngSliderValue(int value);
    void setSpinBox(int value);
    void setAngSpinBox(int value);
//    void onGVStateChanged(int state);
    void onVerStateChanged(int state);
    void onRoteStateChanged(int state);
    void onMirStateChanged(int state);
    void getGVDir();
    void on_confirm_clicked();
    void on_cancle_clicked();
public:
    explicit AugDialog(QWidget *parent = nullptr);
    int dataAugment();//数据增强函数
    int getNumsValue();
    ~AugDialog();
private:
    QLabel *nums_label,*ang_label,*rotate_label,*gv_label;
    QSpinBox *nums_pSpinBox,*ang_pSpinBox;
    QSlider *nums_slider,*ang_slider;
    QCheckBox *verti_flip,*rote_flip,*mir_flip,*gv_qCB;
    QPushButton *confirm,*cancle;
    int num_value=0,degree_value=0;
    bool is_verti=false,is_rote=false,is_mir=false,is_gv=false;
    QLineEdit *_editGvDir;
    QString gv_dir;//gv路径
};
#endif // MYDIALOG_H
