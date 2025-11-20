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
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    view = new QGraphicsView();
    scene = new QGraphicsScene();
    view->setScene(scene);
    view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    layout->addWidget(view);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *zoomOutButton = new QPushButton("-");
    connect(zoomOutButton, &QPushButton::clicked, view, [this]{ view->scale(0.8, 0.8); });
    buttonLayout->addWidget(zoomOutButton);
    QPushButton *zoomInButton = new QPushButton("+");
    connect(zoomInButton, &QPushButton::clicked, view, [this]{ view->scale(1.25, 1.25); });
    buttonLayout->addWidget(zoomInButton);
    layout->addLayout(buttonLayout);
}

void Preview2d::setFixtures(QHash<int, PreviewData> previewData) {
    QHash<int, FixtureGraphicsItem*> oldFixtures = fixtures;
    fixtures.clear();
    for (const int fixtureKey : previewData.keys()) {
        FixtureGraphicsItem* fixture = nullptr;
        if (oldFixtures.contains(fixtureKey)) {
            fixture = oldFixtures.take(fixtureKey);
        } else {
            fixture = new FixtureGraphicsItem();
            scene->addItem(fixture);
        }
        fixtures[fixtureKey] = fixture;
        PreviewData data = previewData.value(fixtureKey);
        QPointF position(100 * data.xPosition, -100 * data.yPosition);
        fixture->setPos(position);
        fixture->label = data.label;
        fixture->color = data.color;
        fixture->pan = data.pan;
        fixture->tilt = data.tilt;
        fixture->zoom = data.zoom;
    }
    for (FixtureGraphicsItem* fixture : oldFixtures.values()) {
        delete fixture;
    }
    scene->update();
    view->setSceneRect(scene->itemsBoundingRect());
}
