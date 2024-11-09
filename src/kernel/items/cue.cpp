#include "cue.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::~Cue() {}

Cue* Cue::copy() {
    Cue* cue = new Cue(kernel);
    cue->id = id;
    cue->label = label;
    cue->transition = transition;
    cue->intensities = intensities;
    cue->colors = colors;
    return cue;
}
