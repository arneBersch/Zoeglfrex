/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "preview2d.h"
#include "kernel/kernel.h"

Preview2d::Preview2d(Kernel* core) {
    kernel = core;

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

void Preview2d::updateImage() {
    bool sceneRectChanged = false;
    if (fixtureCirclesAmount != fixtureCircles.size()) {
        fixtureCirclesAmount = fixtureCircles.size();
        sceneRectChanged = true;
    }
    for (Fixture* fixture : fixtureCircles.keys()) {
        FixtureGraphicsItem *fixtureGraphicsItem = fixtureCircles.value(fixture);
        QPointF position(100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONXATTRIBUTEID), -100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONYATTRIBUTEID));
        if (position != fixtureGraphicsItem->pos()) {
            sceneRectChanged = true;
            fixtureGraphicsItem->setPos(position);
        }
        if (!scene->items().contains(fixtureGraphicsItem)) {
            sceneRectChanged = true;
            scene->addItem(fixtureGraphicsItem);
        }
    }
    scene->update();
    if (sceneRectChanged) {
        view->setSceneRect(scene->itemsBoundingRect());
    }
}
