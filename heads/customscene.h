#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QAction>
#include <QGraphicsView>
#include <QKeyEvent>
#include "boxitem.h"
#include "pointitem.h"
#include <QFileInfo>
#include <QFile>
#include <QImageReader>
#include <QUndoStack>
#include "commands.h"
#include "FreeImage.h"
#include "boxitemmimedata.h"
#include "augDialog.h"
#include <QClipboard>
#include<QMessageBox>
#include<opencv2/opencv.hpp>
class CustomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    CustomScene(QObject* parent = 0);
    ~CustomScene()
    {
        clearAll();
    }

    void loadImage(QString path);
    void saveToFile(const QString& path);
    void clearAll();

    void setTypeNameList (const QStringList &list)
    {
        _typeNameList = list;
    }
    void setTypeName (const QString &name)
    {
        _typeName = name;
    }
    QUndoStack *undoStack() const
    {
        return _undoStack;
    }
    void selectBoxItems(QList<BoxItem *> *boxList, bool op);
    void selectBoxItems(BoxItem *box, bool op);
    void selectBoxItems(PointItem *point, bool op);
    void selectBoxItems(bool op);
    void registerItem(BoxItem *b);
    void registerItem(PointItem *p);
    void drawBoxItem(bool op);
    void drawPloyItem(bool op);
    void panImage(bool op);
    QJsonObject writeQJson(QString label,QList<QPointF> _pointsList);
    qreal* write2YOLO(QImage *_image,QList<QPointF> _pointsList);//保存为yolo格式
    void setBoxTask(bool isBoxTask);
    void setClassTask(bool isClassTask);
    void setSegTask(bool isSegTask);
    void setOCRTask(bool isOCRTask);
    cv::Mat json2MaskPic(QString filePath);//将json文件转换为mask图片
    QJsonObject parasJson(QString filePath);//解析json文件
    void movePoints(PointItem *pointItem,QList<QPointF> newList,QList<QPointF> oldList);//移动坐标点的redo和undo
    void stretchPoints(PointItem *pointItem,QList<QList<QPointF>> redoUndoList);//调整点的位置时候的redo和undo
    cv::Mat augMentPics(cv::Mat img,int angle);//增强图片
    qreal* voc2YOLO(int width,int height,cv::Point point_L_U,cv::Point point_R_L);//将VOC格式转换我YOLO格式
    int* YOLO2VOC(int id,qreal w,qreal h,qreal x,qreal y);//将YOLO格式转换成VOC格式
    void generateImgCoor(QString augPicBasePath,int rote_degree,bool isAug,cv::Mat img,QList<int*> voc_lst);//获得旋转后的坐标
    void gvAugMent(QString gvDir);

public slots:
    void changeBoxTypeName(QString name);
    void changePointTypeName(QString name);
    void changePointLabelName(QString name);
    void selectedBoxItemInfo(QRect rect, QString typeName)
    {
        _typeName = typeName;
    }
    void copy();
    void cut();
    void delete_box();
    void delete_ploy();//删除多边形
    void editPoints();
    void paste();
    void clipboardDataChanged();
    void exportMaskPic();//导出分割后的模板图
    void saveBoxItemsToVOC();//导出VOC格式的标注
    void augMent();//
    void augMentConfirm(int,int,bool,bool,bool,QString);//确认增强

private slots:
    void moveBox(QRectF newRect, QRectF oldRect);
    void closePoint();
    void setCurentPointsList();
    void clearUndoStack();//清空undo栈

signals:
    void imageLoaded(QSize imageSize);
    void cursorMoved(QPointF cursorPos);
    void boxSelected(QRect boxRect, QString typeName);
    void typeChange(QString typeName);//标签值改变
    void typeChange(int index);//标签索引改变
    void labelChange(QString label);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) ;
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent(QKeyEvent *keyEvent);

    void deleteBoxItems();
    void editPointItems();

private:
    QImage *_image;
    QGraphicsPixmapItem *_pixmapItem = nullptr;
    BoxItem* _boxItem = nullptr;//框
    PointItem* _pointItem=nullptr;//点
    PointItem* _moveItem=nullptr;//当前正在移动的点
    PointItem* _currentSelected=nullptr;//当前选中的多边形框
    QString _typeName;
    QStringList _typeNameList;
    double _zoomFactor = 1;
    QPointF _dragStart, _dragEnd;
    bool _isPanning = false;
    bool _isDrawing = false;
    bool _isBox = false;//是否为标注事件
    bool _isMoving = false;
    bool _isMouseMoved = false;
    bool _isPointSelected=false;//多边形是否被选中
    QPointF _leftTopPoint;
    QPointF _rightBottomPoint;
    QString _boxItemFileName,_pointItemFileName,_fileName,_boxItemVOCFileName,_trainVOCFileName,_classFileName;//保存的txt 文本和json文件的名称和图片名称
    QUndoStack *_undoStack;
    BoxItemMimeData *_boxItemMimeData = nullptr;
    QList<QPointF> _pastePos,_oldPointList;
    QPointF _clickedPos;
    bool _leftClick=false;//是否左击来执行双击操作
    bool _isClose=false;//是否封口
    bool _isSucc;
    bool _isBoxTask=false,_isSegTask=false,_isOCRTask=false,_isClassTask=false;//是否为标注任务和分割任务或者OCR任务和分类任务
    AugDialog augDialog;
    QString _currentPath;//当前图片的路径
    QList<QList<QPointF>> _undoRedoList;//redo列表
    void loadBoxItemsFromFile();
    void loadClassFromFile();//加载分类标注文件
    void loadPointItemsFromFile();//加载已经标注好的分割json文件
    void saveBoxItemsToFile();
    void saveClassToFile();
    void savePointItemsToFile();
    void saveOCRPointItemsToFile();//保存OCR的标记结果

};
#endif // CUSTOMSCENE_H
