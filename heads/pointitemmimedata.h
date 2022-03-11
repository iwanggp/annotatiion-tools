#ifndef POINTITEMMIMEDATA_H
#define POINTITEMMIMEDATA_H

#include <QObject>
#include <QGraphicsItem>
#include <QMimeData>
#include "boxitem.h"
class PointItemMimeData: public QMimeData
{
    Q_OBJECT
public:
    PointItemMimeData(QList<QGraphicsItem * > items);
    ~PointItemMimeData();
    QList<QGraphicsItem *> items() const
    {
        return _itemList;
    }
private:
    QList<QGraphicsItem *> _itemList;
public:
    PointItemMimeData();
};

#endif // POINTITEMMIMEDATA_H
