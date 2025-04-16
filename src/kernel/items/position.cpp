/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "position.h"
#include "kernel/kernel.h"

Position::Position(Kernel* core) : Item(core) {}

Position::Position(const Position* item) : Item(item) {}

Position::~Position() {
    /*for (Effect *effect : kernel->effects->items) {
        for (int step : effect->colorSteps.keys()) {
            if (effect->colorSteps.value(step) == this) {
                effect->colorSteps.remove(step);
            }
        }
    }
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->colors.keys()) {
            if (cue->colors.value(group) == this) {
                cue->colors.remove(group);
            }
        }
    }*/
}

QString Position::name() {
    if (stringAttributes.value(kernel->positions->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°, " + QString::number(angleAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "°";
    }
    return Item::name();
}

QString Position::info() {
    QString info = Item::info();
    info += "\n" + kernel->positions->PANATTRIBUTEID + " Pan: " + QString::number(angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°";
    QStringList modelPanValues;
    QStringList modelTiltValues;
    for (Model* model : kernel->models->items) {
        if (modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(model)) {
            modelPanValues.append(model->name() + " @ " + QString::number(modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(model)) {
            modelTiltValues.append(model->name() + " @ " + QString::number(modelSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).value(model)) + "%");
        }
    }
    info += "\n    Model Exceptions: " + modelPanValues.join("; ");
    QStringList fixturePanValues;
    QStringList fixtureTiltValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(fixture)) {
            fixturePanValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture)) {
            fixtureTiltValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture)) + "%");
        }
    }
    info += "\n    Fixture Exceptions: " + fixturePanValues.join("; ");
    info += "\n" + kernel->positions->TILTATTRIBUTEID + " Tilt: " + QString::number(angleAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "%";
    info += "\n    Model Exceptions: " + modelTiltValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureTiltValues.join("; ");
    return info;
}

positionAngles Position::getAngles(Fixture* fixture) {
    positionAngles angles;
    angles.pan = angleAttributes.value(kernel->positions->PANATTRIBUTEID);
    if (fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(fixture)) {
        angles.pan = fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(fixture);
    } else if (modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(fixture->model)) {
        angles.pan = modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(fixture->model);
    }
    angles.tilt = angleAttributes.value(kernel->positions->TILTATTRIBUTEID);
    if (fixtureSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture)) {
        angles.tilt = fixtureSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture);
    } else if (modelSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture->model)) {
        angles.tilt = modelSpecificAngleAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture->model);
    }
    return angles;
}
