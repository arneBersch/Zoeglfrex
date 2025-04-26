#ifndef PREVIEW2D_H
#define PREVIEW2D_H

#include <QtWidgets>

#include "fixturegraphicsitem.h"
class Kernel;
class Fixture;

class Preview2d : public QWidget {
public:
    Preview2d(Kernel* core);
    void updateImage();
    QMap<Fixture*, FixtureGraphicsItem*> fixtureCircles;
private:
    QGraphicsView* view;
    QGraphicsScene* scene;
    Kernel* kernel;
};

#endif // PREVIEW2D_H
