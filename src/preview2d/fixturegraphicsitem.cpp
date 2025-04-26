#include "fixturegraphicsitem.h"

FixtureGraphicsItem::FixtureGraphicsItem() {}

QRectF FixtureGraphicsItem::boundingRect() const {
    return QRectF(-ellipseWidth / 2, -ellipseWidth / 2, ellipseWidth, ellipseWidth);
}

void FixtureGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(red, green, blue));

    QPainterPath path;
    path.lineTo(-20, 100);
    path.lineTo(20, 100);
    path.closeSubpath();
    painter->rotate(pan);
    painter->drawPath(path);
    painter->rotate(-pan);

    painter->drawEllipse(-ellipseWidth / 2, -ellipseWidth / 2, ellipseWidth, ellipseWidth);
}
