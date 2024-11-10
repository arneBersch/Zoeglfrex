#ifndef TRANSITION_H
#define TRANSITION_H

#include <QtWidgets>

#include "item.h"

class Transition : public Item {
public:
    Transition(Kernel* core);
    Transition(const Transition* item);
    ~Transition();
    float fade;
    QString name() override;
};

#endif // TRANSITION_H
