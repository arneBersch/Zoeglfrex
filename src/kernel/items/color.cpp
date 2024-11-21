#include "color.h"
#include "kernel/kernel.h"

Color::Color(Kernel* core) : Item(core) {}

Color::Color(const Color* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    hue = item->hue;
    saturation = item->saturation;
}

Color::~Color() {
    kernel->cues->deleteColor(this);
}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(hue) + "°, " + QString::number(saturation) + "%";
    }
    return Item::name();
}

QString Color::info() {
    QString info = Item::info();
    info += "\nHue: " + QString::number(hue) + "°";
    info += "\nSaturation: " + QString::number(saturation) + "%";
    return info;
}
