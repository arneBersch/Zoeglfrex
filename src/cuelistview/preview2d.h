#ifndef PREVIEW2D_H
#define PREVIEW2D_H

#include <QtWidgets>
class Kernel;
class Fixture;

class Preview2d : public QGraphicsView {
public:
    Preview2d(Kernel* core);
    void updateImage();
    QMap<Fixture*, QGraphicsEllipseItem*> fixtureCircles;
    const int FIXTURESIZE = 70;
private:
    QGraphicsScene *scene;
    Kernel* kernel;
};

#endif // PREVIEW2D_H
