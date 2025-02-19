#ifndef INTENSITY_H
#define INTENSITY_H

#include <QtWidgets>

#include "item.h"
#include "fixture.h"

class Intensity : public Item {
public:
    Intensity(Kernel* core);
    Intensity(const Intensity* item);
    ~Intensity();
    float dimmer = 100;
    QMap<Fixture*, float> fixtureDimmers;
    QString name() override;
    QString info() override;
};

#endif // INTENSITY_H
