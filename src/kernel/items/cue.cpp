#include "cue.h"
#include "kernel/kernel.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::Cue(const Cue* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    fade = item->fade;
    intensities = item->intensities;
    colors = item->colors;
}

Cue::~Cue() {
    if (kernel->cuelistView->currentCue == this) {
        kernel->cuelistView->currentCue = nullptr;
        kernel->cuelistView->loadCue();
    }
}

QString Cue::info() {
    QString info = Item::info();
    info += "\n4 Fade: " + QString::number(fade) + "s";
    return info;
}
