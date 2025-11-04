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
    setFixedSize(windowWidth, windowWidth);
}

void Preview2d::setFixtures(QList<PreviewFixture> fixtures) {
    previewFixtures = fixtures;
    update();
}

void Preview2d::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter = QPainter(this);
    painter.setPen(Qt::NoPen);
    for (PreviewFixture fixture : previewFixtures) {
        QBrush brush = QBrush(fixture.color);
        painter.setBrush(brush);
        const int beamLength = maxBeamLength * std::sin(fixture.tilt * M_PI / 180);
        const int xCoordinate = (fixture.x * 2) + (windowWidth / 2);
        const int yCoordinate = (fixture.y * 2) + (windowWidth / 2);
        painter.drawPie(xCoordinate - beamLength, yCoordinate - beamLength, 2 * beamLength, 2 * beamLength, 16 * (90 - fixture.pan - (fixture.zoom / 2)), 16 * fixture.zoom);
        painter.drawEllipse(xCoordinate - (fixtureDiameter / 2), yCoordinate - (fixtureDiameter / 2), fixtureDiameter, fixtureDiameter);
    }
}
