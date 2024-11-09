#include "transition.h"

Transition::Transition() {}

QString Transition::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(fade) + "s";
    }
    return Item::name();
}

Transition* Transition::copy() {
    Transition* transition = new Transition();
    transition->id = id;
    transition->label = label;
    transition->fade = fade;
    return transition;
}
