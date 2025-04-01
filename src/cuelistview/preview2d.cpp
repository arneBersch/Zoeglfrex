#include "preview2d.h"
#include "kernel/kernel.h"

Preview2d::Preview2d(Kernel* core) {
    kernel = core;

    scene = new QGraphicsScene();
    view = new QGraphicsView();
    view->setWindowTitle("Zöglfrex 2D Preview");
    view->setScene(scene);
    view->resize(500, 300);
}

void Preview2d::updateImage() {
    for (Fixture* fixture : fixtureCircles.keys()) {
        QGraphicsEllipseItem *ellipse = fixtureCircles.value(fixture);
        ellipse->setPos(100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONXATTRIBUTEID), -100 * fixture->floatAttributes.value(kernel->fixtures->POSITIONYATTRIBUTEID));
        const float red = fixture->red / 100 * fixture->dimmer / 100 * 255;
        const float green = fixture->green / 100 * fixture->dimmer / 100 * 255;
        const float blue = fixture->blue / 100 * fixture->dimmer / 100 * 255;
        ellipse->setBrush(QBrush(QColor(red, green, blue)));
        if (!scene->items().contains(ellipse)) {
            scene->addItem(ellipse);
        }
    }
}
