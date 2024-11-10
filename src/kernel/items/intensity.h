#ifndef INTENSITY_H
#define INTENSITY_H

#include <QtWidgets>

#include "item.h"

class Intensity : public Item {
public:
    Intensity(Kernel* core);
    Intensity(const Intensity* item);
    ~Intensity();
    float dimmer;
    QString name() override;
};

#endif // INTENSITY_H
