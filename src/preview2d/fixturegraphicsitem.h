#ifndef FIXTUREGRAPHICSITEM_H
#define FIXTUREGRAPHICSITEM_H

#include <QtWidgets>

class FixtureGraphicsItem : public QGraphicsItem {
public:
    FixtureGraphicsItem();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int red = 0;
    int green = 0;
    int blue = 0;
    int x = 0;
    int y = 0;
};

#endif // FIXTUREGRAPHICSITEM_H
