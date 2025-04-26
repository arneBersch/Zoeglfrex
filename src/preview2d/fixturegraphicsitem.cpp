#include "fixturegraphicsitem.h"

FixtureGraphicsItem::FixtureGraphicsItem() {}

QRectF FixtureGraphicsItem::boundingRect() const {
    return QRectF(x, y, 50, 50);
}

void FixtureGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(red, green, blue));
    painter->drawEllipse(x, y, 50, 50);
}
