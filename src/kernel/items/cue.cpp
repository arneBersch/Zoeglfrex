#include "cue.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::Cue(const Cue* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    transition = item->transition;
    intensities = item->intensities;
    colors = item->colors;
}

Cue::~Cue() {}
