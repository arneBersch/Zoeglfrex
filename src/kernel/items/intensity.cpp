#include "intensity.h"
#include "kernel/kernel.h"

Intensity::Intensity(Kernel* core) : Item(core) {}

Intensity::~Intensity() {
    kernel->cues->deleteIntensity(this);
}

QString Intensity::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(dimmer) + "%";
    }
    return Item::name();
}

Intensity* Intensity::copy() {
    Intensity* intensity = new Intensity(kernel);
    intensity->label = label;
    intensity->dimmer = dimmer;
    return intensity;
}
