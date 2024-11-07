#ifndef INTENSITY_H
#define INTENSITY_H

#include <QtWidgets>

#include "item.h"

class Intensity : public Item {
public:
    Intensity();
    float dimmer;
    QString name() override;
};

#endif // INTENSITY_H
