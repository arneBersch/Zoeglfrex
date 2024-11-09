#ifndef TRANSITION_H
#define TRANSITION_H

#include <QtWidgets>

#include "item.h"

class Transition : public Item {
public:
    Transition(Kernel* core);
    ~Transition();
    float fade;
    QString name() override;
    Transition* copy() override;
};

#endif // TRANSITION_H
