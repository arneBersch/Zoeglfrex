#ifndef CUE_H
#define CUE_H

#include <QtWidgets>

#include "item.h"
#include "transition.h"
#include "group.h"
#include "intensity.h"
#include "color.h"

class Cue : public Item {
public:
    Cue(Kernel *core);
    Cue(const Cue* item);
    ~Cue();
    Transition* transition;
    QMap<Group*, Intensity*> intensities;
    QMap<Group*, Color*> colors;
};

#endif // CUE_H
