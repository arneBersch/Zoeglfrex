#include "cue.h"

Cue::Cue() {}

Cue* Cue::copy() {
    Cue* cue = new Cue();
    cue->id = id;
    cue->label = label;
    cue->transition = transition;
    cue->intensities = intensities;
    cue->colors = colors;
    return cue;
}
