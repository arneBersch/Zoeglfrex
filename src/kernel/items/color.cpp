#include "color.h"

Color::Color() {}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(red) + "%, " + QString::number(green) + "%, " + QString::number(blue) + "%";
    }
    return Item::name();
}

Color* Color::copy() {
    Color* color = new Color();
    color->id = id;
    color->label = label;
    color->red = red;
    color->green = green;
    color->blue = blue;
    return color;
}
