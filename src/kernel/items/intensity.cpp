#include "intensity.h"

Intensity::Intensity() {}

QString Intensity::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(dimmer) + " %";
    }
    return Item::name();
}
