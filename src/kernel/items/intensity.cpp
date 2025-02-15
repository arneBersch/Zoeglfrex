#include "intensity.h"
#include "kernel/kernel.h"

Intensity::Intensity(Kernel* core) : Item(core) {}

Intensity::Intensity(const Intensity* item) : Item(item->kernel) {
    label = item->label;
    dimmer = item->dimmer;
}

Intensity::~Intensity() {
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->intensities.keys()) {
            if (cue->intensities.value(group) == this) {
                cue->intensities.remove(group);
            }
        }
    }
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
