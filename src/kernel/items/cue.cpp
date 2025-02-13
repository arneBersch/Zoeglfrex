#include "cue.h"
#include "kernel/kernel.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::Cue(const Cue* item) : Item(item->kernel) {
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
    info += "\n5 Fade: " + QString::number(fade) + "s";
    return info;
}
