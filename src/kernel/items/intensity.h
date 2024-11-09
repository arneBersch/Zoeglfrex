#ifndef INTENSITY_H
#define INTENSITY_H

#include <QtWidgets>

#include "item.h"

class Intensity : public Item {
public:
    Intensity(Kernel* core);
    ~Intensity();
    float dimmer;
    QString name() override;
    Intensity* copy() override;
};

#endif // INTENSITY_H
