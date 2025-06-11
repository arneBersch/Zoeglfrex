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
    for (Effect *effect : kernel->effects->items) {
        for (int step : effect->positionSteps.keys()) {
            if (effect->positionSteps.value(step) == this) {
                effect->positionSteps.remove(step);
            }
        }
    }
    for (Cuelist* cuelist : kernel->cuelists->items) {
        for (Cue *cue : cuelist->cues->items) {
            for (Group *group : cue->positions.keys()) {
                if (cue->positions.value(group) == this) {
                    cue->positions.remove(group);
                }
            }
        }
    }
}

QString Position::name() {
    if (stringAttributes.value(kernel->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(angleAttributes.value(kernel->POSITIONPANATTRIBUTEID)) + "°, " + QString::number(floatAttributes.value(kernel->POSITIONTILTATTRIBUTEID)) + "°";
    }
    return Item::name();
}

QString Position::info() {
    QString info = Item::info();
    QStringList modelPanValues;
    QStringList modelTiltValues;
    QStringList modelZoomValues;
    for (Model* model : kernel->models->items) {
        if (modelSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).contains(model)) {
            modelPanValues.append(model->name() + " @ " + QString::number(modelSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).contains(model)) {
            modelTiltValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).contains(model)) {
            modelZoomValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).value(model)) + "°");
        }
    }
    QStringList fixturePanValues;
    QStringList fixtureTiltValues;
    QStringList fixtureZoomValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).contains(fixture)) {
            fixturePanValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).contains(fixture)) {
            fixtureTiltValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).contains(fixture)) {
            fixtureZoomValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).value(fixture)) + "°");
        }
    }
    info += "\n" + kernel->POSITIONPANATTRIBUTEID + " Pan: " + QString::number(angleAttributes.value(kernel->POSITIONPANATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelPanValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixturePanValues.join("; ");
    info += "\n" + kernel->POSITIONTILTATTRIBUTEID + " Tilt: " + QString::number(floatAttributes.value(kernel->POSITIONTILTATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelTiltValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureTiltValues.join("; ");
    info += "\n" + kernel->POSITIONZOOMATTRIBUTEID + " Zoom: " + QString::number(floatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelZoomValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureZoomValues.join("; ");
    QStringList raws;
    for (Raw* raw : rawListAttributes.value(kernel->POSITIONRAWSATTRIBUTEID)) {
        raws.append(raw->name());
    }
    info += "\n" + kernel->POSITIONRAWSATTRIBUTEID + raws.join(" + ");
    return info;
}

positionAngles Position::getAngles(Fixture* fixture) {
    positionAngles angles;
    angles.pan = angleAttributes.value(kernel->POSITIONPANATTRIBUTEID);
    if (fixtureSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).contains(fixture)) {
        angles.pan = fixtureSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).value(fixture);
    } else if (modelSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).contains(fixture->model)) {
        angles.pan = modelSpecificAngleAttributes.value(kernel->POSITIONPANATTRIBUTEID).value(fixture->model);
    }
    angles.tilt = floatAttributes.value(kernel->POSITIONTILTATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).contains(fixture)) {
        angles.tilt = fixtureSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).contains(fixture->model)) {
        angles.tilt = modelSpecificFloatAttributes.value(kernel->POSITIONTILTATTRIBUTEID).value(fixture->model);
    }
    angles.zoom = floatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).contains(fixture)) {
        angles.zoom = fixtureSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).contains(fixture->model)) {
        angles.zoom = modelSpecificFloatAttributes.value(kernel->POSITIONZOOMATTRIBUTEID).value(fixture->model);
    }
    return angles;
}
