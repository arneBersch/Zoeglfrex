#include "transition.h"
#include "kernel/kernel.h"

Transition::Transition(Kernel* core) : Item(core) {}

Transition::~Transition() {
    kernel->cues->deleteTransition(this);
}

QString Transition::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(fade) + "s";
    }
    return Item::name();
}

Transition* Transition::copy() {
    Transition* transition = new Transition(kernel);
    transition->id = id;
    transition->label = label;
    transition->fade = fade;
    return transition;
}
