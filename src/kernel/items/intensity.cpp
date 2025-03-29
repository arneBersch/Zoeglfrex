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
    for (Effect *effect : kernel->effects->items) {
        for (int step : effect->intensitySteps.keys()) {
            if (effect->intensitySteps.value(step) == this) {
                effect->intensitySteps.remove(step);
            }
        }
    }
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->intensities.keys()) {
            if (cue->intensities.value(group) == this) {
                cue->intensities.remove(group);
            }
        }
    }
}

QString Intensity::name() {
    if (stringAttributes.value(kernel->intensities->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%";
    }
    return Item::name();
}

QString Intensity::info() {
    QString info = Item::info();
    info += "\n" + kernel->intensities->DIMMERATTRIBUTEID + " Dimmer: " + QString::number(floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%";
    QStringList modelDimmerValues;
    for (Model* model : modelSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).keys()) {
        modelDimmerValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(model)) + "%");
    }
    info += "\n    Model Exceptions: " + modelDimmerValues.join("; ");
    QStringList fixtureDimmerValues;
    for (Fixture* fixture : fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).keys()) {
        fixtureDimmerValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(fixture)) + "%");
    }
    info += "\n    Fixture Exceptions: " + fixtureDimmerValues.join("; ");
    return info;
}
