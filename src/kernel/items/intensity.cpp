/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "intensity.h"
#include "kernel/kernel.h"

Intensity::Intensity(Kernel* core) : Item(core) {}

Intensity::Intensity(const Intensity* item) : Item(item) {}

Intensity::~Intensity() {
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->intensities.keys()) {
            if (cue->intensities.value(group) == this) {
                cue->intensities.remove(group);
            }
        }
    }
}

QString Intensity::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%";
    }
    return Item::name();
}

QString Intensity::info() {
    QString info = Item::info();
    info += "\n" + kernel->intensities->DIMMERATTRIBUTEID + " Dimmer: " + QString::number(floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%";
    QString fixtureDimmerValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).contains(fixture)) {
            fixtureDimmerValues += fixture->id + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(fixture)) + "%; ";
        }
    }
    fixtureDimmerValues.chop(2);
    info += "\n    Fixture Exceptions: " + fixtureDimmerValues;
    return info;
}
