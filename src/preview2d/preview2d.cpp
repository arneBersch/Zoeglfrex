#include "preview2d.h"
#include "kernel/kernel.h"

Preview2d::Preview2d(Kernel* core) {
    kernel = core;

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

    setWindowTitle("Zöglfrex 2D Preview");
    resize(500, 300);
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
