#include "preview2d.h"
#include "kernel/kernel.h"

Preview2d::Preview2d(Kernel* core) {
    kernel = core;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    view = new QGraphicsView();
    scene = new QGraphicsScene();
    view->setScene(scene);
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
    for (Fixture* fixture : fixtureCircles.keys()) {
        FixtureGraphicsItem *fixtureGraphicsItem = fixtureCircles.value(fixture);
        fixtureGraphicsItem->setPos(100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONXATTRIBUTEID), -100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONYATTRIBUTEID));
        if (!scene->items().contains(fixtureGraphicsItem)) {
            scene->addItem(fixtureGraphicsItem);
        }
    }
    scene->update();
}
