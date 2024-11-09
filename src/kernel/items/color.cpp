#include "color.h"
#include "kernel/kernel.h"

Color::Color(Kernel* core) : Item(core) {}

Color::~Color() {
    kernel->cues->deleteColor(this);
}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(red) + "%, " + QString::number(green) + "%, " + QString::number(blue) + "%";
    }
    return Item::name();
}

Color* Color::copy() {
    Color* color = new Color(kernel);
    color->id = id;
    color->label = label;
    color->red = red;
    color->green = green;
    color->blue = blue;
    return color;
}
