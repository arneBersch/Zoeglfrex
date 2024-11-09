#include "transition.h"

Transition::Transition() {}

QString Transition::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(fade) + "s";
    }
    return Item::name();
}
