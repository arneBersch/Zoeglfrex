#ifndef INTENSITY_H
#define INTENSITY_H

#include <QtWidgets>

#include "item.h"

class Intensity : public Item {
public:
    Intensity(Kernel* core);
    Intensity(const Intensity* item);
    ~Intensity();
    float dimmer = 100;
    QString name() override;
    QString info() override;
};

#endif // INTENSITY_H
