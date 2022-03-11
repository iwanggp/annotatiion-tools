#include "commands.h"
/******************************************************************************
** AddBoxCommand
*/

AddBoxCommand::AddBoxCommand(QGraphicsScene *scene, QList<BoxItem *> *boxList, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _boxList  = boxList;
//    _boxList = new QList<BoxItem *>();
//    for (int i=0; i<boxList.count(); i++) {
//        BoxItem *box = qgraphicsitem_cast<BoxItem *>(boxList.at(i));
//        _boxList->append(box);
//    }
}

AddBoxCommand::AddBoxCommand(QGraphicsScene *scene, BoxItem *box, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _box = box;
//    _boxList = new QList<BoxItem *>();
//    for (int i=0; i<boxList.count(); i++) {
//        BoxItem *box = qgraphicsitem_cast<BoxItem *>(boxList.at(i));
//        _boxList->append(box);
//    }
}
AddPointCommand::AddPointCommand(QGraphicsScene *scene, PointItem *point, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _point = point;
}

/**
 * @brief AddPointCommand::undo
 * 描点的撤回操作
 */
void AddPointCommand::undo()
{
    if (_point) {
        QPointF _temp=_point->pointUndo();
        _editPoints.push_back(_temp);
        QApplication::setOverrideCursor(_point->oldCursor());
        _scene->update();
    }
}
void AddPointCommand::redo()
{
    if(_point){
        if(!_editPoints.isEmpty()){
            QPointF _temp=_editPoints.last();
            _editPoints.pop_back();
            if(_temp.x()>=0&&_temp.y()>=0&&_temp.x()<=_scene->width()&&_temp.y()<=_scene->height()) _point->pointRedo(_temp);
        }
        QApplication::setOverrideCursor(_point->oldCursor());
    }
}
/******************************************************************************
** 删除多点标注框的事件处理
*/

RemovePointsCommand::RemovePointsCommand(QGraphicsScene *scene, PointItem *pointItem,
                                        QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _pointItem.append(pointItem);
    qDebug()<<"undo is run.............."<<_pointItem.size();
}

void RemovePointsCommand::undo()
{
    int _listSize=_pointItem.size();
    for(int i=0;i<_listSize;i++){
        PointItem *item = _pointItem.at(_listSize-i-1);
        _scene->addItem(item);
    }
}

void RemovePointsCommand::redo()
{
    int _listSize=_pointItem.size();
    for (int i=0; i<_listSize; i++) {
        PointItem *item = _pointItem.at(_listSize-i-1);
        _scene->removeItem(item);
        QApplication::setOverrideCursor(item->oldCursor());
    }
}

void AddBoxCommand::undo()
{
    if (_box) {
        _scene->removeItem(_box);
        QApplication::setOverrideCursor(_box->oldCursor());
    }
    if (_boxList) {
        for (int i=0; i<_boxList->count(); i++) {
            BoxItem *item = _boxList->at(i);
            _scene->removeItem(item);
            QApplication::setOverrideCursor(item->oldCursor());
        }
    }
}

void AddBoxCommand::redo()
{
    //    QCursor c = Qt::CrossCursor;
    //    _box->setOldCursor(c);
    if (_boxList) {
        for (int i=0; i<_boxList->count(); i++) {
            BoxItem *item = _boxList->at(i);
            if (!item->rect().isNull())
                _scene->addItem(item);
        }
        reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_boxList, true);
        //    for (int i=0; i<_boxList->count(); i++) {
        //        BoxItem *item = _boxList->at(i);
        //        if (!item->rect().isNull())
        //            _scene->addItem(item);
        //    }
//        reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_boxList, true);
    }

    if (_box) {
        if (!_box->rect().isNull()) {
            _scene->addItem(_box);
            _box->setSelected(true);
        }
    }
}

/******************************************************************************
** RemoveBoxItemCommand
*/

RemoveBoxesCommand::RemoveBoxesCommand(QGraphicsScene *scene, QList<QGraphicsItem *> boxList,
                                        QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _boxList = new QList<BoxItem *>();
    for (int i=0; i<boxList.count(); i++) {
        BoxItem *box = qgraphicsitem_cast<BoxItem *>(boxList.at(i));
        _boxList->append(box);
    }
}

void RemoveBoxesCommand::undo()
{
    for (int i=0; i<_boxList->count(); i++) {
        BoxItem *item = _boxList->at(i);
        if (!item->rect().isNull())
            _scene->addItem(item);
    }
    reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_boxList, true);
}

void RemoveBoxesCommand::redo()
{
    for (int i=0; i<_boxList->count(); i++) {
        BoxItem *item = _boxList->at(i);
        _scene->removeItem(item);
        QApplication::setOverrideCursor(item->oldCursor());
    }
}

/******************************************************************************
** SetTargetTypeCommand
*/

//SetTargetTypeCommand::SetTargetTypeCommand(QGraphicsScene *scene, BoxItem *box, const QString &typeName,
//                                             QUndoCommand *parent)
SetTargetTypeCommand::SetTargetTypeCommand(QGraphicsScene *scene, QList<BoxItem *> *boxList, const QString &typeName,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
//    _box = box;
    _boxList = new QList<BoxItem *>();
    for (int i=0; i<boxList->count(); i++) {
        BoxItem *box = boxList->at(i);
        _boxList->append(box);
    }
    _oldNameList = new QStringList();
    for (int i=0; i<boxList->count(); i++) {
        _oldNameList->append(boxList->at(i)->typeName());
    }
//    _oldName = _box->typeName();
    _newName = typeName;
}

void SetTargetTypeCommand::undo()
{
//    reinterpret_cast<ViewScene *>(_scene)->selectBoxItems(_box, true);
//    _box->setTypeName(_oldName);
    for (int i=0; i<_boxList->count(); i++) {
        BoxItem *item = _boxList->at(i);
        item->setTypeName(_oldNameList->at(i));
    }
    reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_boxList, true);
}

void SetTargetTypeCommand::redo()
{
//    reinterpret_cast<ViewScene *>(_scene)->selectBoxItems(_box, true);
//    _box->setTypeName(_newName);
    for (int i=0; i<_boxList->count(); i++) {
        BoxItem *item = _boxList->at(i);
        item->setTypeName(_newName);
    }
    reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_boxList, true);
}

/******************************************************************************
** MoveBoxCommand
*/

MoveBoxCommand::MoveBoxCommand(QGraphicsScene *scene, BoxItem *box, const QRectF &newRect, const QRectF &oldRect,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _box = box;
    _oldRect = oldRect;
    _newRect = newRect;
}

void MoveBoxCommand::undo()
{
    reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_box, true);
    _box->setRect(_oldRect);
}

void MoveBoxCommand::redo()
{
    reinterpret_cast<CustomScene *>(_scene)->selectBoxItems(_box, true);
    _box->setRect(_newRect);
}

/******************************************************************************
** MovePointCommand
*/

MovePointsCommand::MovePointsCommand(QGraphicsScene *scene, PointItem* pointItem,const QList<QPointF> &oldList,const QList<QPointF> &newList,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _pointItem = pointItem;
    _oldList = oldList;
    _newList = newList;
}

void MovePointsCommand::undo()
{
    _pointItem->setPointList(_newList);
}

void MovePointsCommand::redo()
{
     _pointItem->setPointList(_oldList);
}
/******************************************************************************
** StretchPointsCommand 调整点位置的redo和undo操作
*/

StretchPointsCommand::StretchPointsCommand(QGraphicsScene *scene, PointItem* pointItem,const QList<QList<QPointF>> &redoUndoList,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _scene = scene;
    _pointItem = pointItem;
    _redoUndoList=redoUndoList;
}

void StretchPointsCommand::undo()
{
    _pointItem->setPointList(_redoUndoList.at(_redoUndoList.size()-2));
    _scene->update();
}

void StretchPointsCommand::redo()
{
    _pointItem->setPointList(_redoUndoList.at(_redoUndoList.size()-1));
    _scene->update();
}


