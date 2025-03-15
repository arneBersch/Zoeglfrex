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

Color::Color(const Color* item) : Item(item) {}

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
    if (stringAttributes.value(kernel->colors->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(angleAttributes.value(kernel->colors->HUEATTRIBUTEID)) + "°, " + QString::number(floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID)) + "%";
    }
    return Item::name();
}

QString Color::info() {
    QString info = Item::info();
    info += "\n" + kernel->colors->HUEATTRIBUTEID + " Hue: " + QString::number(angleAttributes.value(kernel->colors->HUEATTRIBUTEID)) + "°";
    info += "\n" + kernel->colors->SATURATIONATTRIBUTEID + " Saturation: " + QString::number(floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID)) + "%";
    QStringList modelSaturationValues;
    for (Model* model : modelSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).keys()) {
        modelSaturationValues.append(model->id + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).value(model)) + "%");
    }
    info += "\n    Model Exceptions: " + modelSaturationValues.join("; ");
    QStringList fixtureSaturationValues;
    for (Fixture* fixture : fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).keys()) {
        fixtureSaturationValues.append(fixture->id + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).value(fixture)) + "%");
    }
    info += "\n    Fixture Exceptions: " + fixtureSaturationValues.join("; ");
    return info;
}
