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
    ~Cue();
    Transition* transition;
    QMap<Group*, Intensity*> intensities;
    QMap<Group*, Color*> colors;
    Cue* copy() override;
};

#endif // CUE_H
