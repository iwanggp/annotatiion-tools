#ifndef POINTITEM_H
#define POINTITEM_H


#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QCursor>
#include <QList>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QMenu>
#include <QApplication>
#include <QDebug>
#include<QInputDialog>
enum  PointGrabberID{
    P_Inner,
    P_Border,//移动点时的状态
    P_Select
};

enum  PointTaskStatus {
    P_Moving = 0,
    P_Stretching,
    P_Waiting
};

class PointItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    PointItem(QRectF sceneRect, QSize imageSize, QStringList &targetTypeNameList, QString targetTypeName);
    ~PointItem()
    {
       ;
    }

    void setTypeName(QString name);
    QString typeName() const
    {
        return _typeName;
    }
    int movePoint(){
        return _movePoint;
    }
    void setPointList(const QList<QPointF> &ploy);
    void setPoint(const QPointF p);
    void setMovePoint(const QPointF p);
    void setClosePoint();//双击闭合圆环
    void setPoints(QList<QPointF>);//设置当前多边形的点
    void setCurrentPoints(QList<QPointF>,QGraphicsSceneMouseEvent * event);
    void paintPloy(QList<QPointF> _pointList,QPainter *painter,double xScale,double yScale,int _circleWidth=5,int _circleHeight=5);
    void setRect(const qreal x, const qreal y, const qreal w, const qreal h);

    bool isContainsPoint(QList<QPointF> ql,QPointF q);
    bool isContainsBorderPoint(QList<QPointF> ql,QPointF q);//判断是否包含边界点
    void maskPoints(QList<QPointF> pointsList,QPainter *painter);
    //设置是否为OCR任务
    void setOCRTask(bool isOCRTask);
    bool getOCRTask(){
        return _isOCRTask;
    }

    void rect(QMap<QString,QList<QPointF>> *info) const
    {
//        qreal ws = 1.0 / _sceneRect.width();
//        qreal hs = 1.0 / _sceneRect.height();

//        info[0] = _rect.center().x()*ws;
//        info[1] = _rect.center().y()*hs;
//        info[2] = _rect.width()*ws;
//        info[3] = _rect.height()*hs;
    }
    //返回屏幕的相关信息
    QRectF sceneRect()
    {
        return _sceneRect;
    }
    QList<QPointF> points()
    {
        return _pointsList;
    }
    void setPointsList(QList<QPointF> ql)
    {
        _currentPointList=ql;
        _isEdit=true;
    }

    QList<QPointF> pointList()
    {
        return _pointList;
    }
    void setPointList2(QList<QPointF> ql)
    {
        _pointList.clear();
        for(auto p:ql) _pointList.push_back(p);
    }
    QCursor oldCursor() const
    {
        return _oldCursor;
    }
    void setOldCursor(QCursor &c)
    {
        _oldCursor = c;
    }
    QPointF pointUndo()//撤销操作
    {
        if(!_pointList.isEmpty())
        {

            QPointF _temp= _pointList.last();
            _pointList.pop_back();
            return _temp;
        }
        return QPointF(0.0,0.0);
    }
    void pointRedo(QPointF p)//重做操作
    {
        _pointList.push_back(p);
    }
    enum { Type = UserType + 1 };
    int type() const
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }
    PointItem *copy()
    {
        PointItem *b = new PointItem(_sceneRect, _imageSize, _typeNameList, _typeName);
//        b->setRect(_rect);
        b->setOldCursor(_oldCursor);
        return b;
    }
    void setEdit()
    {
        _isEdit=true;
    }
    void setItemSelected(bool selected)//设置当前是否选中
    {
        _isItemSelected=selected;
    }
    bool getItemSelected()//是否被选中
    {
        return _isItemSelected;
    }
    PointTaskStatus getTaskStatus(){//获得当前调整的状态值
        return _taskStatus;
    }
    void setMaskPointList(QList<QPointF> q);//设置当前所处的焦点
    void clearMaskPointList();//清空掩膜列表
    void setNearPoint(QPointF _currentPos);
signals:
    void boxSelected(QRect boxRect, QString typeName);
    void typeNameChanged(QString newTypeName);
    void stretchCompleted(QList<QPointF> currentList);
    void moveCompleted(QList<QPointF> newList, QList<QPointF> oldList);
    void closePoint();//定义闭环的函数
    void getCurentPointsList();//获取当前的列表
    void cleanUndoStack();
    void getLabel(QString text);//获得标签值
private:

    virtual QRectF boundingRect() const;
    virtual QList<QPointF> currentPoints() const;
    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent *event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent *event );
    virtual void hoverMoveEvent (QGraphicsSceneHoverEvent * event);

    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent (QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent * event );
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    virtual void mouseMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void mousePressEvent(QGraphicsSceneDragDropEvent *event);

    void setGrabbers(QPointF p,qreal width, qreal height);
    void setCircle(int width,int height);
    void initContextMenu();

    PointGrabberID getSelectedGrabber(PointGrabberID ID);
    void setGrabberCursor(PointGrabberID stretchRectState);
    QList<QPointF> calculateMoveRect(QPointF dragStart, QPointF dragEnd);//调整整体的位置
    QList<QPointF> calculateStretchRect(QPointF dragEnd);//调整点的位置
    QRect getRealRect();
    void setTopmost();

    PointTaskStatus _taskStatus = P_Waiting;
    bool _isMouseMoved = false;

    QSize _imageSize;
    QRectF _sceneRect;
    QRectF _rect;
    QList<QPointF> _pointList,_pointsList,_maskPointsList,_currentPointList;//_pointsList保存点坐标,以及鼠标经过的多边形list
    QList<PointItem> _pointItems;
    int _pointCount=0;
    int _mapCount=0;
    QPolygonF polygon;
    QGraphicsTextItem _textRect, _textName;
    QRectF _boundingRect;
    QPointF _boundingPoint,_movingPoint;
    QStringList _typeNameList;
    QString _typeName;

    QCursor _oldCursor;
    QRectF _oldRect;
    QList<QPointF> _oldPointsList;
    QMenu _contextMenu;
    QColor _color;
    qreal _penWidth = 2;
    QPen _pen;

    int _grabberWidth;
    int _grabberHeight;
    int _circleWidth;
    int _circleHeight;
    int _movePoint;//正在移动的点
    bool _isLastPoint;//是否为最后一个点
    bool isMoving=false;//是否移动
    bool _isClose=false;
    bool _isItemSelected=false;//是否选中
    QRectF _grabbers;
    PointGrabberID _selectedGrabber;
    QPointF _dragStart, _dragEnd;
    QPointF _currentPos;//当前的鼠标点击位置
    bool _isEdit=false;
    bool _isOCRTask=false;//是否为OCR任务

};

#endif // POINTITEM_H
