#include "cue.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::Cue(const Cue* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    fade = item->fade;
    intensities = item->intensities;
    colors = item->colors;
}

Cue::~Cue() {}

QString Cue::info() {
    QString info = Item::info();
    info += "\nFade: " + QString::number(fade) + "s";
    return info;
}
