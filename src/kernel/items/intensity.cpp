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
    for (Model *model : kernel->models->items) {
        for (QString groupSpecificIntensityAttribute : model->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : model->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (model->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    model->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Fixture *fixture : kernel->fixtures->items) {
        for (QString groupSpecificIntensityAttribute : fixture->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : fixture->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (fixture->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    fixture->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Group *currentGroup: kernel->groups->items) {
        for (QString groupSpecificIntensityAttribute : currentGroup->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : currentGroup->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (currentGroup->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    currentGroup->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Intensity *intensity : kernel->intensities->items) {
        for (QString groupSpecificIntensityAttribute : intensity->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : intensity->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (intensity->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    intensity->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Color *color : kernel->colors->items) {
        for (QString groupSpecificIntensityAttribute : color->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : color->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (color->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    color->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Raw *raw : kernel->raws->items) {
        for (QString groupSpecificIntensityAttribute : raw->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : raw->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (raw->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    raw->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
            }
        }
    }
    for (Cue *cue : kernel->cues->items) {
        for (QString groupSpecificIntensityAttribute : cue->groupSpecificIntensityAttributes.keys()) {
            for (Group *group : cue->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).keys()) {
                if (cue->groupSpecificIntensityAttributes.value(groupSpecificIntensityAttribute).value(group) == this) {
                    cue->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(group);
                }
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
