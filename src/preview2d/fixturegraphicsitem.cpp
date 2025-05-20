/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturegraphicsitem.h"

FixtureGraphicsItem::FixtureGraphicsItem() {}

QRectF FixtureGraphicsItem::boundingRect() const {
    return QRectF(-ellipseWidth / 2, -ellipseWidth / 2, ellipseWidth, ellipseWidth);
}

void FixtureGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(red, green, blue));

    QPainterPath path;
    int beamLength = maxBeamLength * std::sin(tilt * M_PI / 180);
    path.arcTo(-beamLength, -beamLength, 2 * beamLength, 2 * beamLength, 90 - pan - (zoom / 2), zoom);
    painter->drawPath(path);

    painter->drawEllipse(-ellipseWidth / 2, -ellipseWidth / 2, ellipseWidth, ellipseWidth);
}
