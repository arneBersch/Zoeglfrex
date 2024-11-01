#ifndef CUE_H
#define CUE_H

#include <QtWidgets>

#include "transition.h"
#include "group.h"
#include "intensity.h"
#include "color.h"

class Cue {
public:
    Cue();
    QString id;
    QString label;
    Transition* transition;
    QMap<Group*, Intensity*> intensities;
    QMap<Group*, Color*> colors;
};

#endif // CUE_H
