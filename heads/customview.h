#ifndef CUSTOMVIEW_H
#define CUSTOMVIEW_H

#include <QObject>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QMouseEvent>
#include <boxitem.h>
#include <pointitem.h>
#include <QMessageBox>
class CustomView : public QGraphicsView
{
    Q_OBJECT

public:
    CustomView(QObject* parent);
    void panImage(bool checked);
    void setPointTask(bool isPointTask);
//    void fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRatioMode);
public slots:
    void drawBoxItem(bool checked);
    void drawPloyItem(bool checked);
    void editPloyItem(bool checked);
private:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    QCursor _cursor;
    bool _isPanning = false;
    bool _startFlag = false;
    bool _pointTask = false;//是否为分割任务
    int _panStartX, _panStartY;
};

#endif // CUSTOMVIEW_H
