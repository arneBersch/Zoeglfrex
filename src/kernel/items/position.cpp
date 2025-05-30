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
        return Item::name() + QString::number(angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°, " + QString::number(floatAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "°";
    }
    return Item::name();
}

QString Position::info() {
    QString info = Item::info();
    QStringList modelPanValues;
    QStringList modelTiltValues;
    QStringList modelZoomValues;
    for (Model* model : kernel->models->items) {
        if (modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(model)) {
            modelPanValues.append(model->name() + " @ " + QString::number(modelSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(model)) {
            modelTiltValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).contains(model)) {
            modelZoomValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).value(model)) + "°");
        }
    }
    QStringList fixturePanValues;
    QStringList fixtureTiltValues;
    QStringList fixtureZoomValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).contains(fixture)) {
            fixturePanValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->positions->PANATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture)) {
            fixtureTiltValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).contains(fixture)) {
            fixtureZoomValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).value(fixture)) + "°");
        }
    }
    info += "\n" + kernel->positions->PANATTRIBUTEID + " Pan: " + QString::number(angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelPanValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixturePanValues.join("; ");
    info += "\n" + kernel->positions->TILTATTRIBUTEID + " Tilt: " + QString::number(floatAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelTiltValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureTiltValues.join("; ");
    info += "\n" + kernel->positions->ZOOMATTRIBUTEID + " Zoom: " + QString::number(floatAttributes.value(kernel->positions->ZOOMATTRIBUTEID)) + "°";
    info += "\n    Model Exceptions: " + modelZoomValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureZoomValues.join("; ");
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
    angles.tilt = floatAttributes.value(kernel->positions->TILTATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture)) {
        angles.tilt = fixtureSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).contains(fixture->model)) {
        angles.tilt = modelSpecificFloatAttributes.value(kernel->positions->TILTATTRIBUTEID).value(fixture->model);
    }
    angles.zoom = floatAttributes.value(kernel->positions->ZOOMATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).contains(fixture)) {
        angles.zoom = fixtureSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).contains(fixture->model)) {
        angles.zoom = modelSpecificFloatAttributes.value(kernel->positions->ZOOMATTRIBUTEID).value(fixture->model);
    }
    return angles;
}
