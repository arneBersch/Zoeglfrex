#include "transition.h"
#include "kernel/kernel.h"

Transition::Transition(Kernel* core) : Item(core) {}

Transition::Transition(const Transition* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    fade = item->fade;
}

Transition::~Transition() {
    kernel->cues->deleteTransition(this);
}

QString Transition::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(fade) + "s";
    }
    return Item::name();
}
