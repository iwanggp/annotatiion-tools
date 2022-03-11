#include "pointitem.h"
#include <QBrush>
#include <QLinearGradient>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
PointItem::PointItem(QRectF sceneRect, QSize imageSize, QStringList &targetTypeNameList, QString targetTypeName):
    _typeNameList(targetTypeNameList),
    _textRect(),
    _textName(),
    _typeName(targetTypeName),
    _color(Qt::darkGreen),//外框颜色
    _dragStart(0,0),
    _dragEnd(0,0),
    _sceneRect(sceneRect),
    _grabberWidth(120),
    _grabberHeight(120),
    _imageSize(imageSize),
    _pointList(),
    _pointsList()
{
    _textRect.setDefaultTextColor(QColor(255,255,255,255));
    _textRect.setFlag(QGraphicsItem::ItemIgnoresTransformations);
    _textRect.setParentItem(this);
    this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    this->setAcceptHoverEvents(true);
    _oldCursor = Qt::ArrowCursor;
    initContextMenu();
}

void PointItem::setTopmost()
{
    QList<QGraphicsItem *> list = collidingItems(Qt::IntersectsItemBoundingRect);
    foreach (QGraphicsItem *it, list) {
        if (it->type() == QGraphicsItem::UserType + 1) {
            it->stackBefore(this);
        }
    }
}

/**
 * @brief PointItem::mousePressEvent
 * @param event
 * 鼠标按下的事件相应
 */
void PointItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{


    event->setAccepted(true);

    switch (event->buttons()) {
    case Qt::LeftButton:
        _isEdit=false;
        _dragStart = event->pos();
        qDebug()<<"edit is run...................currentPoint"<<event->scenePos() <<"-----_currentPointList------"<<_currentPointList<<_isItemSelected;
        _currentPos=event->scenePos();
        if(!_currentPointList.isEmpty())
        {
            if(!isContainsPoint(_currentPointList,_currentPos)) _isEdit=false;
            else {
                if(_isOCRTask) emit getLabel(this->typeName());
                PointGrabberID id=getSelectedGrabber(P_Inner);
                setGrabberCursor(id);
                _isEdit=true;
                _taskStatus = P_Moving;//是否移动多边形
            }
            this->setNearPoint(_currentPos);
        }
        break;

    case Qt::RightButton:
        this->setSelected(true);//这里有个bug
        break;
    default:
        break;
    }
}
/**
 * @brief PointItem::mouseReleaseEvent 鼠标离开事件处理
 * @param event
 */
void PointItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{

    QList<QPointF> pointsList;
    event->setAccepted(true);
    if (1) {
        switch (_taskStatus) {
        case P_Moving:
            _dragEnd = event->pos();
            pointsList=calculateMoveRect(_dragStart, _dragEnd);
            break;
        case P_Stretching:
            pointsList=calculateStretchRect(_dragEnd);
            break;
        default:
            break;
        }
        _taskStatus = P_Waiting;
    }
    _isMouseMoved = false;
    //    else {
    //        this->setSelected(true);
    //    }
}

void PointItem::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{

    qDebug()<<"mouseMoveEvent..............."<<_taskStatus;
    event->setAccepted(true);
    QList<QPointF> pointsList;
    if (!_currentPointList.isEmpty()) {
        _isMouseMoved = true;
        switch (_taskStatus) {
        case PointTaskStatus::P_Moving:
            _isEdit=true;
            this->setPointList(calculateMoveRect(_dragStart, event->pos()));
            pointsList=calculateMoveRect(_dragStart, _dragEnd);
            qDebug()<<"P_Moving..........."<<pointsList;
            break;
        case PointTaskStatus::P_Stretching:
            _isEdit=true;
            this->setPointList(calculateStretchRect(event->pos()));
            break;
        case PointTaskStatus::P_Waiting:
            _isEdit=false;
            _taskStatus = P_Waiting;//等待
            PointGrabberID id=getSelectedGrabber(P_Select);
            setGrabberCursor(id);
        }
    }
}

void PointItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent *event )
{
    event->setAccepted(true);
    if(_pointsList.isEmpty())
    {

    }
    QCursor _temp=Qt::ArrowCursor;
    QApplication::setOverrideCursor(_temp);
    //    if (this->isSelected()){
    //        QApplication::setOverrideCursor(_oldCursor);
    //    }
}

void PointItem::hoverEnterEvent ( QGraphicsSceneHoverEvent *event )
{
    event->setAccepted(true);
    if (this->isSelected()){
        //        _selectedGrabber = getSelectedGrabber(event->pos());
        //        setGrabberCursor(_selectedGrabber);
    }
}
/**
 * @brief PointItem::hoverMoveEvent
 * @param event
 * 鼠标经过时的事件处理
 */
void PointItem::hoverMoveEvent ( QGraphicsSceneHoverEvent *event )
{
    event->setAccepted(true);
    if (!_currentPointList.isEmpty()){
        _currentPos=event->scenePos();
        if(!isContainsPoint(_currentPointList,_currentPos))
        {
            PointGrabberID id=getSelectedGrabber(P_Select);
            setGrabberCursor(id);
            _isEdit=false;
            _taskStatus=P_Waiting;
            this->setNearPoint(_currentPos);//在调整多边形的时候当鼠标靠近时候的效果
        }else{
            PointGrabberID id=getSelectedGrabber(P_Inner);
            setGrabberCursor(id);
            _isEdit=true;
        }
    }
}

QList<QPointF> PointItem::calculateMoveRect(QPointF dragStart, QPointF dragEnd)
{
    QList<QPointF> _newPointList;
    if(!_oldPointsList.isEmpty()){
        for(auto point:_oldPointsList){
            qreal x = dragEnd.x() - dragStart.x() +point.x();
            qreal y = dragEnd.y() - dragStart.y() +point.y();
            if (x <= _sceneRect.left()) {
                x = _sceneRect.left();
            }
            if (y <= _sceneRect.top()) {
                y = _sceneRect.top();
            }
            if (_sceneRect.right()-x <= _rect.width()) {
                x = _sceneRect.right() - _rect.width();
            }

            if (_sceneRect.bottom()-y <= _rect.height()) {
                y = _sceneRect.bottom() - _rect.height();
            }

            QPointF points(x,y);
            _newPointList.append(points);
        }
    }
    return _newPointList;
}


QList<QPointF> PointItem::calculateStretchRect(QPointF dragEnd)
{
    QPointF realPoint;
    QList<QPointF> oldList=_currentPointList;
    QPointF   dragStart=_currentPointList.at(_movePoint);
    qreal dx = dragEnd.x() - dragStart.x()+_currentPointList.at(_movePoint).x();
    qreal dy = dragEnd.y() - dragStart.y()+_currentPointList.at(_movePoint).y();
    if (dx <= _sceneRect.left()) dx = _sceneRect.left();
    if (dy <= _sceneRect.top())  dy = _sceneRect.top();
    if(dx>=_sceneRect.right())  dx=_sceneRect.right();
    if(dy>=_sceneRect.bottom()) dy=_sceneRect.bottom();

    realPoint=QPointF(dx,dy);//防止异常的点
    _currentPointList.replace(_movePoint,realPoint);
    if(_isLastPoint){
        _currentPointList.replace(0,realPoint);
    }
    return _currentPointList;
}

void PointItem::setRect(const qreal x, qreal y, qreal w, qreal h)
{
}

/**
 * @brief PointItem::setPoints
 * @param pointsList
 * 设置选中的点加上掩膜的效果
 */
void PointItem::setPoints(QList<QPointF> pointsList){
    _maskPointsList.clear();
    prepareGeometryChange();
    _boundingRect.setRect(0, 0,this->_imageSize.width(), this->_imageSize.height());//设置刷新有点bug没解决
    //    if(_isEdit) _maskPointsList=pointsList;
    _maskPointsList=pointsList;
    this->update();
}

/**
 * @brief PointItem::setCurrentPoints
 * @param pointsList
 * @param event
 * 选择当前选择的多边形条目
 */
void PointItem::setCurrentPoints(QList<QPointF> pointsList,QGraphicsSceneMouseEvent * event)
{
    _currentPointList.clear();
    _currentPointList=pointsList;
    event->setAccepted(true);
    switch (event->buttons()) {
    case Qt::LeftButton:
        qDebug()<<"mousePressEvent...........setCurrentPoints------------"<<_currentPointList<<"current"<<event->scenePos()<<"------";
        _maskPointsList=_currentPointList;
        setItemSelected(true);
        setTopmost();
        _dragStart = event->scenePos();
        //            emit boxSelected(getRealRect(), _typeName);
        //        }
        break;
    case Qt::RightButton:
        break;
    default:
        break;
    }
    _oldPointsList = _currentPointList;
    this->update();
}
//设置点坐标
void PointItem::setPoint(const QPointF p)
{
    prepareGeometryChange();
    _boundingRect.setRect(p.x(), p.y(),this->_imageSize.width(), this->_imageSize.height());
    this->_imageSize.width();

    if (_sceneRect.intersects(_boundingRect)) {
        if (!_sceneRect.contains(_boundingRect)) {
            _boundingRect = _sceneRect.intersected(_boundingRect);

        }
    }
    //判断边界点
    if(p.x()>=0&&p.y()>=0&&p.x()<=this->_imageSize.width()&&p.y()<=this->_imageSize.height())
    {
        if(!_pointList.isEmpty())
        {
            for(int i=1;i<_pointList.size();i++)
            {
                if(qAbs(p.y()-_pointList.at(i).y())<6&&qAbs(p.x()-_pointList.at(i).x())<6) return;//靠的太近的点不标注
            }
        }
        _pointList.append(p);

    }
    qDebug()<<"Hello no contains.........."<<p<<_pointList;
    if(_pointList.size() != 0 && qAbs(p.y()-_pointList.at(0).y())<8&&qAbs(p.x()-_pointList.at(0).x())<8)//接近圆点的时候放大处理
    {
        if(_pointList.size()  > 2)
        {

           setClosePoint();//当接近起始点的时候再次点击就闭合
        }
    }
    setGrabbers(p,_grabberWidth, _grabberHeight);
    isMoving=false;
    qreal halfpw = (_pen.style() == Qt::NoPen) ? qreal(0) : _pen.widthF() / 2;
    if (halfpw > 0.0)
        _boundingRect.adjust(-halfpw, -halfpw, halfpw, halfpw);
    this->update();
}
//保存鼠标移动点坐标
void PointItem::setMovePoint(const QPointF p)
{
    prepareGeometryChange();
    _boundingRect.setRect(0, 0,this->_imageSize.width(), this->_imageSize.height());//设置刷新有点bug没解决
    _movingPoint=p;
    if(_pointList.size() != 0 && qAbs(p.y()-_pointList.at(0).y())<6&&qAbs(p.x()-_pointList.at(0).x())<6)//接近圆点的时候放大处理
    {
        if(_pointList.size()  > 2)
        {
            setCircle(6,6);
        }
    }else{
        setCircle(3,3);
    }
    qreal halfpw = (_pen.style() == Qt::NoPen) ? qreal(0) : _pen.widthF() / 2;
    if (halfpw > 0.0)
        _boundingRect.adjust(-halfpw, -halfpw, halfpw, halfpw);
    isMoving=true;
    this->update();
}
//双击闭合圆环,常规的标注任务
void PointItem::setClosePoint()
{
    prepareGeometryChange();
    _boundingRect.setRect(0, 0,this->_imageSize.width(), this->_imageSize.height());//设置刷新有点bug没解决
    qreal halfpw = (_pen.style() == Qt::NoPen) ? qreal(0) : _pen.widthF() / 2;
    if (halfpw > 0.0)
        _boundingRect.adjust(-halfpw, -halfpw, halfpw, halfpw);
    if(_pointList.size()>3) _pointList.pop_back();//弹出当前位置双击的点
    qDebug()<<"the _pointList is........"<<_pointList;
    _isClose=true;
    _pointList.push_back(_pointList.at(0));
    _pointsList.append(_pointList);
    _maskPointsList=_pointList;
    qDebug()<<"the _pointsList"<<_pointsList;
    _pointList.clear();//清空列表
    if(_isOCRTask){//当为OCR标注任务时
        bool isOK;
        QString text = QInputDialog::getText(NULL, "Input label Dialog",
                                                           "Please input the box label",
                                                           QLineEdit::Normal,
                                                           "your label",
                                                           &isOK);
        if(isOK) {
               _typeName=text;
               emit getLabel(text);//发送信号给customscene
        }
    }
    emit closePoint();//发送封口的信号
}

/**
 * @brief PointItem::setPointList
 * @param ploy
 * 设置当前多边形的点信息
 */
void PointItem::setPointList(const QList<QPointF> &ploy)
{

    prepareGeometryChange();
    _boundingRect.setRect(0, 0,this->_imageSize.width(), this->_imageSize.height());//设置刷新有点bug没解决
    _currentPointList.clear();
    _currentPointList=ploy;
    if(_isEdit) _maskPointsList = ploy;//加掩膜的效果
    _pointsList=ploy;//当前界面的所有多边形
    if (_sceneRect.intersects(_boundingRect)) {
        if (!_sceneRect.contains(_boundingRect)) {
            _boundingRect = _sceneRect.intersected(_boundingRect);
        }
    } else {
        return;
    }


    qreal halfpw = (_pen.style() == Qt::NoPen) ? qreal(0) : _pen.widthF() / 2;
    if (halfpw > 0.0)
        _boundingRect.adjust(-halfpw, -halfpw, halfpw, halfpw);

    //    // emit the selected box real rect to scene and statusbar.
    //    emit boxSelected(getRealRect(), _typeName);

    this->update();
}

QRect PointItem::getRealRect()
{
    qreal _xScale = _imageSize.width()*1.0/_sceneRect.width();
    qreal _yScale = _imageSize.height()*1.0/_sceneRect.height();
    QRect r((int)(_rect.left()*_xScale), (int)(_rect.top()*_yScale),
            (int)(_rect.width()*_xScale), (int)(_rect.height()*_yScale));

    return r;
}

void PointItem::setTypeName(QString name)
{
    _typeName = name;
    emit boxSelected(getRealRect(), _typeName);
    this->update();
}

QRectF PointItem::boundingRect() const
{
    return _boundingRect;
}
QList<QPointF> PointItem::currentPoints() const
{
    return _currentPointList;
}
void PointItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    double xScale = scale()/painter->transform().m11();
    double yScale = scale()/painter->transform().m22();//做到缩放圆点不变
    _pen.setWidthF(_penWidth*xScale);
    _pen.setColor(_color);
    _pen.setStyle(Qt::SolidLine);
    painter->setPen(_pen);

    QBrush brush(QColor(255,0,0,0), Qt::SolidPattern);
    painter->setBrush(brush);
    if(!_pointList.isEmpty()) paintPloy(_pointList,painter,xScale,yScale,_circleWidth,_circleHeight);

    if(_pointList.size()>0&&!_isEdit) {
        //做个判断防止第一个点出现异常的情况
        if(_movingPoint.x()>0) painter->drawLine(_pointList.at(_pointList.size()-1),_movingPoint);
    }
    //画整个图形_pointsList表示整个界面上的图形
    if(!_pointsList.isEmpty())
    {
        qDebug()<<"the _ponintsList is ......."<<_pointsList;
        paintPloy(_pointsList,painter,xScale,yScale,3,3);
    }
    if (option->state & QStyle::State_MouseOver) {
    }
    maskPoints(_maskPointsList,painter);//鼠标经过用颜色覆盖的特效
    _maskPointsList.clear();

}
void PointItem::maskPoints(QList<QPointF> pointsList,QPainter *painter)
{
    QVector<QPointF> qv;
    QLinearGradient myGradient;
    for(QPointF p:pointsList){
        qv.append(p);
    }
    QPolygonF pol(qv);
    QPainterPath myPath;
    QBrush brush2(QColor(0,255,57,57), Qt::SolidPattern);
    myPath.addPolygon(pol);
    painter->drawPath(myPath);
    painter->fillPath(myPath,brush2);
    //    for(int i=0;i<pointsList.size();i++)
    //        painter->drawRect(pointsList.at(i).x()-_circleWidth*5/2,pointsList.at(i).y()-8*5/2,_circleWidth*5,8*5);//画矩形
    //    myPath.clear();
}
/**
 * @brief PointItem::setOCRTask 设置是否为OCR任务
 * @param isOCRTask 是否为OCR任务
 */
void PointItem::setOCRTask(bool isOCRTask)
{
    _isOCRTask=isOCRTask;
}
/**
 * 画多边形函数
 * @brief PointItem::paintPloy
 * @param painter 画笔
 * @param _circleWidth 宽度
 * @param _circleHeight 高度
 */
void PointItem::paintPloy(QList<QPointF> _pointList, QPainter *painter, double xScale,double yScale,int _circleWidth, int _circleHeight)
{
    for(int i=0;i<_pointList.size();i++)
    {
        if(_isEdit) painter->drawRect(_pointList.at(i).x()-_circleWidth*xScale/2,_pointList.at(i).y()-_circleHeight*yScale/2,6*yScale,6*yScale);//画矩形
        else painter->drawEllipse(_pointList.at(i),_circleWidth*xScale,_circleHeight*yScale);
        if(i>0) painter->drawLine(_pointList.at(i-1),_pointList.at(i));//两点连线

    }
}
/**
 * @brief PointItem::getSelectedGrabber
 * @param point
 * @return
 * 设置鼠标样式
 */
PointGrabberID PointItem::getSelectedGrabber(PointGrabberID ID)
{
    return ID;
}
/**
 * @brief PointItem::setGrabberCursor
 * @param id
 * 设置鼠标经过的样式
 */
void PointItem::setGrabberCursor(PointGrabberID id)
{
    QCursor cursor;
    switch (id)
    {
    case P_Inner:
        cursor = Qt::OpenHandCursor;
        break;
    case P_Border:
        cursor = Qt::PointingHandCursor;
        break;
    case P_Select:
        cursor=Qt::ArrowCursor;
    default:
        break;
    }
    QApplication::setOverrideCursor(cursor);
    setCursor(cursor);
}
//设置圆圈的大小
void PointItem::setCircle(int width, int height){
    _circleWidth=width;
    _circleHeight=height;
}
void PointItem::setGrabbers(QPointF p,qreal width, qreal height)
{
    qreal w = width/2, h = height/2; // int -> qreal, solve grabber transformation bug

    // drawingRegion contains rect and 8 grabbers
    _grabbers.setRect(p.x()-w/2,     p.y()-h/2,       w, h);
}

void PointItem::mouseMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(false);
}

void PointItem::mousePressEvent(QGraphicsSceneDragDropEvent *event)

{
    event->setAccepted(true);
}

void PointItem::initContextMenu()
{
    _contextMenu.clear();
    foreach (QString name, _typeNameList) {
        _contextMenu.addAction(name);
    }
}

void PointItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)//鼠标右击事件处理
{
    qDebug()<<"right mouse.......isClose"<<_isClose<<"isEdit........."<<_isEdit;
    if(!_isClose&&!_isEdit) return;//当没有封口或者正在标记时候，右键无效
    if(_isEdit)
    {
        if(_isOCRTask){
            bool isOK;
            QString text = QInputDialog::getText(NULL, "Input label Dialog",
                                                               "Please input the box label",
                                                               QLineEdit::Normal,
                                                               this->typeName().toStdString().c_str(),
                                                               &isOK);
            if(isOK) {
                   _typeName=text;
                   emit getLabel(text);//发送信号给customscene类型改变
            }
        }
        QAction *selectedAction = _contextMenu.exec(event->screenPos());
        if (selectedAction) {
            QString name = selectedAction->text();
            if (_typeNameList.contains(name)) {
                emit typeNameChanged(name);
            }
        }
    }
}
/**
 * @brief PointItem::isContainsPoint
 * @param ql 存放点的QList
 * @param q QPointF点坐标
 * @return
 */
bool PointItem::isContainsPoint(QList<QPointF> ql,QPointF q)
{
    QVector<QPointF> qv;
    for(QPointF p:ql){
        qv.append(p);
    }
    QPolygonF pol(qv);

    if(pol.containsPoint(q,Qt::OddEvenFill)){
        return true;
    }
    return false;
}
/**
 * @brief PointItem::isContainsBorderPoint 判断是否包含边界点
 * @param ql 点集
 * @param q 当前点
 * @return
 */
bool PointItem::isContainsBorderPoint(QList<QPointF> ql, QPointF q)
{

    for(int i=0;i<ql.size();i++)//当靠近边界点时候也认为包含
    {
        if(qAbs(ql.at(i).y()-q.y())<6&&qAbs(ql.at(i).x()-q.x())<6) return true;
    }
    return false;
}

/**
 * @brief PointItem::setMaskPointList
 * @param ql
 * 设置当前选中有模板效果
 */
void PointItem::setMaskPointList(QList<QPointF> ql)
{
    _maskPointsList=ql;
}
void PointItem::clearMaskPointList()
{

}
/**
 * @brief PointItem::setNearPoint
 * @param _currentPos
 * 在调整多边形时候，鼠标靠近时的效果展示
 */
void PointItem::setNearPoint(QPointF _currentPos)
{
    for(int i=0;i<_currentPointList.size();i++)//最后一一个点为重复的点，这里不处理后面再处理
    {
        if(qAbs(_currentPointList.at(i).y()-_currentPos.y())<6&&qAbs(_currentPointList.at(i).x()-_currentPos.x())<6)
        {
            qDebug()<<"stretch.................."<<"current index is....."<<i;
            emit stretchCompleted(_currentPointList);//准备开始调整点的位置
            PointGrabberID id=getSelectedGrabber(P_Border);
            setGrabberCursor(id);
            if(i==_currentPointList.size()-1) _isLastPoint=true;
            else _isLastPoint=false;
            _movePoint=i;//正在移动的点索引
            _isEdit=true;
            _taskStatus = P_Stretching;//调整多边形
        }
    }
}
