/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "color.h"
#include "kernel/kernel.h"

Color::Color(Kernel* core) : Item(core) {}

Color::Color(const Color* item) : Item(item) {
    label = item->label;
    hue = item->hue;
    saturation = item->saturation;
}

Color::~Color() {
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->colors.keys()) {
            if (cue->colors.value(group) == this) {
                cue->colors.remove(group);
            }
        }
    }
}

QString Color::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(hue) + "°, " + QString::number(saturation) + "%";
    }
    return Item::name();
}

QString Color::info() {
    QString info = Item::info();
    info += "\n" + kernel->colors->HUEATTRIBUTEID + " Hue: " + QString::number(hue) + "°";
    info += "\n" + kernel->colors->SATURATIONATTRIBUTEID + " Saturation: " + QString::number(saturation) + "%";
    return info;
}
