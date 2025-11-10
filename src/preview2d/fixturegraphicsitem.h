/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FIXTUREGRAPHICSITEM_H
#define FIXTUREGRAPHICSITEM_H

#include <QtWidgets>

class FixtureGraphicsItem : public QGraphicsItem {
public:
    FixtureGraphicsItem();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QColor color;
    int pan = 0;
    int tilt = 0;
    int zoom = 0;
private:
    const int ellipseWidth = 50;
    const int maxBeamLength = 150;
};

#endif // FIXTUREGRAPHICSITEM_H
