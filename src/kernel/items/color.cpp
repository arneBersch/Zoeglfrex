#include "color.h"
#include "kernel/kernel.h"

Color::Color(Kernel* core) : Item(core) {}

Color::Color(const Color* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    red = item->red;
    green = item->green;
    blue = item->blue;
}

Color::~Color() {
    kernel->cues->deleteColor(this);
}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(red) + "%, " + QString::number(green) + "%, " + QString::number(blue) + "%";
    }
    return Item::name();
}
