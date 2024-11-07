#include "color.h"

Color::Color() {}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(red) + " %, " + QString::number(green) + " %, " + QString::number(blue) + " %";
    }
    return Item::name();
}
