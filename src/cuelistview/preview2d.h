#ifndef PREVIEW2D_H
#define PREVIEW2D_H

#include <QtWidgets>
class Kernel;
class Fixture;

class Preview2d {
public:
    Preview2d(Kernel* core);
    void updateImage();
    QGraphicsView *view;
    QMap<Fixture*, QGraphicsEllipseItem*> fixtureCircles;
    const int FIXTURESIZE = 70;
private:
    QGraphicsScene *scene;
    Kernel* kernel;
};

#endif // PREVIEW2D_H
