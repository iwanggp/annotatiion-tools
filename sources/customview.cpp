#include "customview.h"
#include<QDebug>
CustomView::CustomView(QObject* parent)
{
    //    _cursor = Qt::ArrowCursor;
}

void CustomView::drawBoxItem(bool checked)
{
    if (checked) {
        _cursor = Qt::CrossCursor;
    } else {
        _cursor = Qt::ArrowCursor;
    }
    qDebug()<<"cursor drawBoxItem ....."<<_cursor;
    bool cursorFlag = false;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            qgraphicsitem_cast<BoxItem *>(item)->setOldCursor(_cursor);
            if (item->isSelected()) {
                QPoint viewPoint = mapFromGlobal(QCursor::pos());
                QPointF scenePoint = mapToScene(viewPoint);
                if (item->contains(scenePoint)) {
                    cursorFlag = true;
                    break;
                }
            }
        }
    }

    if (!cursorFlag) {
        QApplication::setOverrideCursor(_cursor);
        qDebug()<<"QApplication is run......";
    }
}
void CustomView::setPointTask(bool isPointTask){
    _pointTask=isPointTask;
}
void CustomView::drawPloyItem(bool checked)
{
    if (checked) {
        _cursor = Qt::CrossCursor;
    } else {
        _cursor = Qt::ArrowCursor;
    }
    qDebug()<<"cursor draw Ploy is ....."<<_cursor;
    bool cursorFlag = false;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            qgraphicsitem_cast<PointItem *>(item)->setOldCursor(_cursor);
            if (item->isSelected()) {
                QPoint viewPoint = mapFromGlobal(QCursor::pos());
                QPointF scenePoint = mapToScene(viewPoint);
                if (item->contains(scenePoint)) {
                    cursorFlag = true;
                    break;
                }
            }
        }
    }

    if (!cursorFlag) {
        QApplication::setOverrideCursor(_cursor);
        qDebug()<<"QApplication is run......";
    }
}


void CustomView::editPloyItem(bool checked)
{
    if (checked) {
        _cursor = Qt::CrossCursor;
    } else {
        _cursor = Qt::ArrowCursor;
    }
    bool cursorFlag = false;
    foreach (QGraphicsItem *item, scene()->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            qgraphicsitem_cast<PointItem *>(item)->setOldCursor(_cursor);
            if (item->isSelected()) {
                QPoint viewPoint = mapFromGlobal(QCursor::pos());
                QPointF scenePoint = mapToScene(viewPoint);
                if (item->contains(scenePoint)) {
                    cursorFlag = true;
                    break;
                }
            }
        }
    }

    if (!cursorFlag) {
        QApplication::setOverrideCursor(_cursor);
        qDebug()<<"QApplication is run......";
    }
}
void CustomView::panImage(bool checked)
{
    _isPanning = checked;
    if (checked) {
        _cursor = Qt::OpenHandCursor;
    } else {
        _cursor = Qt::ArrowCursor;
        foreach (QGraphicsItem *item, scene()->items()) {
            if (item->type() == QGraphicsItem::UserType+1) {
                if(_pointTask) qgraphicsitem_cast<PointItem *>(item)->setOldCursor(_cursor);//??????????????????
                else qgraphicsitem_cast<BoxItem *>(item)->setOldCursor(_cursor);//????????????
            }
        }
    }
    QApplication::setOverrideCursor(_cursor);
}

void CustomView::enterEvent(QEvent *event)
{
    QApplication::setOverrideCursor(_cursor);
}

void CustomView::leaveEvent(QEvent *event)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void CustomView::mousePressEvent(QMouseEvent *event)
{
    if (_isPanning) {
        qDebug()<<"_imageview is presss";
        _startFlag = true;
        _panStartX = event->x();
        _panStartY = event->y();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CustomView::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isPanning) {
        _startFlag = false;
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void CustomView::mouseMoveEvent(QMouseEvent *event)
{
    if (_isPanning && _startFlag) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panStartY));
        _panStartX = event->x();
        _panStartY = event->y();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

