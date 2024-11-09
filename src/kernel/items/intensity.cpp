#include "intensity.h"

Intensity::Intensity() {}

QString Intensity::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(dimmer) + "%";
    }
    return Item::name();
}

Intensity* Intensity::copy() {
    Intensity* intensity = new Intensity();
    intensity->label = label;
    intensity->dimmer = dimmer;
    return intensity;
}
