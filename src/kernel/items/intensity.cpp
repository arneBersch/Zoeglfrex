#include "intensity.h"
#include "kernel/kernel.h"

Intensity::Intensity(Kernel* core) : Item(core) {}

Intensity::Intensity(const Intensity* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    dimmer = item->dimmer;
}

Intensity::~Intensity() {
    kernel->cues->deleteIntensity(this);
}

QString Intensity::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(dimmer) + "%";
    }
    return Item::name();
}

QString Intensity::info() {
    QString info = Item::info();
    info += "\n2 Dimmer: " + QString::number(dimmer) + "%";
    return info;
}
