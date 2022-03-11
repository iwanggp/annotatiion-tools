#include "pointitemmimedata.h"

PointItemMimeData::PointItemMimeData(QList<QGraphicsItem *> items)
{
    foreach (QGraphicsItem *item, items) {
        if (item->type() == QGraphicsItem::UserType +1) {
            BoxItem *b = qgraphicsitem_cast<BoxItem*>(item);
            _itemList.append(b->copy());
        }
    }
}

PointItemMimeData::~PointItemMimeData()
{
    foreach (QGraphicsItem *item, _itemList) {
        delete item;
    }
    _itemList.clear();
}
