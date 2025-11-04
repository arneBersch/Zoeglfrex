/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "preview2d.h"

Preview2d::Preview2d(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex 2D Preview");
    resize(500, 300);
}

void Preview2d::setFixtures(QList<PreviewFixture> fixtures) {
    previewFixtures = fixtures;
    update();
}

void Preview2d::paintEvent(QPaintEvent* event) {
    const float fixtureDiameter = 50;
    Q_UNUSED(event);
    QPainter painter = QPainter(this);
    painter.setPen(Qt::NoPen);
    for (PreviewFixture fixture : previewFixtures) {
        QBrush brush = QBrush(fixture.color);
        painter.setBrush(brush);
        painter.drawEllipse((fixture.x - (fixtureDiameter / 2)), (fixture.y - (fixtureDiameter / 2)), fixtureDiameter, fixtureDiameter);
    }
}
