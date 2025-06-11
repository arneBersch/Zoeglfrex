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
    for (Cuelist *cuelist : kernel->cuelists->items) {
        for (Cue *cue : cuelist->cues->items) {
            for (Group *group : cue->intensities.keys()) {
                if (cue->intensities.value(group) == this) {
                    cue->intensities.remove(group);
                }
            }
        }
    }
}

QString Intensity::name() {
    if (stringAttributes.value(kernel->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(floatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID)) + "%";
    }
    return Item::name();
}

QString Intensity::info() {
    QString info = Item::info();
    info += "\n" + kernel->INTENSITYDIMMERATTRIBUTEID + " Dimmer: " + QString::number(floatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID)) + "%";
    QStringList modelDimmerValues;
    for (Model* model : kernel->models->items) {
        if (modelSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).contains(model)) {
            modelDimmerValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).value(model)) + "%");
        }
    }
    info += "\n    Model Exceptions: " + modelDimmerValues.join("; ");
    QStringList fixtureDimmerValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).contains(fixture)) {
            fixtureDimmerValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).value(fixture)) + "%");
        }
    }
    info += "\n    Fixture Exceptions: " + fixtureDimmerValues.join("; ");
    QStringList raws;
    for (Raw* raw : rawListAttributes.value(kernel->INTENSITYRAWSATTRIBUTEID)) {
        raws.append(raw->name());
    }
    info += "\n" + kernel->INTENSITYRAWSATTRIBUTEID + raws.join(" + ");
    return info;
}

float Intensity::getDimmer(Fixture* fixture) {
    float dimmer = floatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).contains(fixture)) {
        dimmer = fixtureSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).contains(fixture->model)) {
        dimmer = modelSpecificFloatAttributes.value(kernel->INTENSITYDIMMERATTRIBUTEID).value(fixture->model);
    }
    return dimmer;
}
