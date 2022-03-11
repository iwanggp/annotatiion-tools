#ifndef COMMANDS_H
#define COMMANDS_H

//#include <QGraphicsScene>
#include <boxitem.h>
#include <pointitem.h>
#include <QUndoCommand>
#include <customscene.h>

class AddBoxCommand : public QUndoCommand
{
public:
    AddBoxCommand(QGraphicsScene *scene, QList<BoxItem *> *boxList, QUndoCommand *parent = 0);
    AddBoxCommand(QGraphicsScene *scene, BoxItem *box, QUndoCommand *parent = 0);
    ~AddBoxCommand()
    {
        if (_boxList) {
            delete _boxList;
        }
    }
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    QList<BoxItem *> *_boxList = nullptr;
    BoxItem *_box = nullptr;
};
class RemoveBoxesCommand : public QUndoCommand
{
public:
    RemoveBoxesCommand(QGraphicsScene *scene, QList<QGraphicsItem *> boxList, QUndoCommand *parent = 0);
    ~RemoveBoxesCommand()
    {
        if (_boxList) {
            delete _boxList;
        }
    }
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    QList<BoxItem *> *_boxList = nullptr;
};

class SetTargetTypeCommand : public QUndoCommand
{
public:
    SetTargetTypeCommand(QGraphicsScene *scene, QList<BoxItem *> *boxList, const QString &typeName,
                            QUndoCommand *parent = 0);

    ~SetTargetTypeCommand()
    {
        if (_boxList) {
            delete _boxList;
        }
        if (_oldNameList) {
            delete _oldNameList;
        }
    }
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    QString _oldName, _newName;
    QList<BoxItem *> *_boxList = nullptr;
    QStringList *_oldNameList = nullptr;
};

class MoveBoxCommand : public QUndoCommand
{
public:
    MoveBoxCommand(QGraphicsScene *scene, BoxItem *box, const QRectF &newRect, const QRectF &oldRect,
                                             QUndoCommand *parent=0);

    void undo() override;
    void redo() override;
private:
    QGraphicsScene *_scene;
    BoxItem *_box;
    QRectF _oldRect;
    QRectF _newRect;
};

/**
 * @brief The AddPointCommand class
 * 描点操作的测回操作
 */
class AddPointCommand : public QUndoCommand
{
public:
    AddPointCommand(QGraphicsScene *scene, PointItem *point, QUndoCommand *parent = 0);
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    PointItem *_point = nullptr;
    QList<QPointF> _editPoints;//记录操作过的点，为redo操作
    bool _isClose=false;
};
/**
 * @brief The RemoveBoxesCommand class
 * 删除标注框的撤回操作
 */
class RemovePointsCommand : public QUndoCommand
{
public:
    RemovePointsCommand(QGraphicsScene *scene, PointItem* pointItem, QUndoCommand *parent = 0);
    ~RemovePointsCommand()
    {
        if (!_pointItem.isEmpty()) {
             _pointItem.clear();
        }
    }
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    QList<PointItem*> _pointItem;//存储结果
};


/**
 * @brief The RemoveBoxesCommand class
 *移动标注点的撤回操作
 */
class MovePointsCommand : public QUndoCommand
{
public:
    MovePointsCommand(QGraphicsScene *scene, PointItem* pointItem,const QList<QPointF> &oldList,const QList<QPointF> &newList, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    PointItem *_pointItem;
    QList<QPointF> _oldList;
    QList<QPointF> _newList;
};



/**
 * @brief The RemoveBoxesCommand class
 *  调整标注点的撤回操作
 */
class StretchPointsCommand : public QUndoCommand
{
public:
    StretchPointsCommand(QGraphicsScene *scene, PointItem* pointItem,const QList<QList<QPointF>> &redoUndoList,QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;

private:
    QGraphicsScene *_scene;
    PointItem *_pointItem;
    QList<QList<QPointF>> _redoUndoList;
};



#endif // COMMANDS_H
