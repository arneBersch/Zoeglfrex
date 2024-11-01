#ifndef TRANSITION_H
#define TRANSITION_H

#include <QtWidgets>

#include "item.h"

class Transition : public Item {
public:
    Transition();
    float fade;
};

#endif // TRANSITION_H
