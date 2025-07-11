/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "model.h"
#include "kernel/kernel.h"

Model::Model(Kernel* core) : Item(core) {}

Model::Model(const Model* item) : Item(item) {}

Model::~Model() {
    for (Model* model : kernel->models->items) {
        for (QString modelSpecificFloatAttribute : model->modelSpecificFloatAttributes.keys()) {
            model->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : model->modelSpecificAngleAttributes.keys()) {
            model->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixture->model == this) {
            fixture->model = nullptr;
        }
        for (QString modelSpecificFloatAttribute : fixture->modelSpecificFloatAttributes.keys()) {
            fixture->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : fixture->modelSpecificAngleAttributes.keys()) {
            fixture->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Group* group : kernel->groups->items) {
        for (QString modelSpecificFloatAttribute : group->modelSpecificFloatAttributes.keys()) {
            group->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : group->modelSpecificAngleAttributes.keys()) {
            group->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Intensity* intensity : kernel->intensities->items) {
        for (QString modelSpecificFloatAttribute : intensity->modelSpecificFloatAttributes.keys()) {
            intensity->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : intensity->modelSpecificAngleAttributes.keys()) {
            intensity->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Color* color : kernel->colors->items) {
        for (QString modelSpecificFloatAttribute : color->modelSpecificFloatAttributes.keys()) {
            color->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : color->modelSpecificAngleAttributes.keys()) {
            color->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Position* position : kernel->positions->items) {
        for (QString modelSpecificFloatAttribute : position->modelSpecificFloatAttributes.keys()) {
            position->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : position->modelSpecificAngleAttributes.keys()) {
            position->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Raw* raw : kernel->raws->items) {
        raw->modelSpecificChannelValues.remove(this);
        for (QString modelSpecificFloatAttribute : raw->modelSpecificFloatAttributes.keys()) {
            raw->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : raw->modelSpecificAngleAttributes.keys()) {
            raw->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Effect* effect : kernel->effects->items) {
        for (QString modelSpecificFloatAttribute : effect->modelSpecificFloatAttributes.keys()) {
            effect->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : effect->modelSpecificAngleAttributes.keys()) {
            effect->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Cuelist* cuelist : kernel->cuelists->items) {
        for (QString modelSpecificFloatAttribute : cuelist->modelSpecificFloatAttributes.keys()) {
            cuelist->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : cuelist->modelSpecificAngleAttributes.keys()) {
            cuelist->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
        for (Cue* cue : cuelist->cues->items) {
            for (QString modelSpecificFloatAttribute : cue->modelSpecificFloatAttributes.keys()) {
                cue->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
            }
            for (QString modelSpecificAngleAttribute : cue->modelSpecificAngleAttributes.keys()) {
                cue->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
            }
        }
    }
}

QString Model::name() {
    if (stringAttributes.value(kernel->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + stringAttributes.value(kernel->MODELCHANNELSATTRIBUTEID);
    }
    return Item::name();
}

QString Model::info() {
    QString info = Item::info();
    info += "\n" + kernel->MODELCHANNELSATTRIBUTEID + " Channels: " + stringAttributes.value(kernel->MODELCHANNELSATTRIBUTEID);
    info += "\n" + kernel->MODELPANRANGEATTRIBUTEID + " Pan Range: " + QString::number(floatAttributes.value(kernel->MODELPANRANGEATTRIBUTEID)) + "°";
    info += "\n" + kernel->MODELTILTRANGEATTRIBUTEID + " Tilt Range: " + QString::number(floatAttributes.value(kernel->MODELTILTRANGEATTRIBUTEID)) + "°";
    info += "\n" + kernel->MODELMINZOOMATTRIBUTEID + " Minimal Zoom: " + QString::number(floatAttributes.value(kernel->MODELMINZOOMATTRIBUTEID)) + "°";
    info += "\n" + kernel->MODELMAXZOOMATTRIBUTEID + " Maximal Zoom: " + QString::number(floatAttributes.value(kernel->MODELMAXZOOMATTRIBUTEID)) + "°";
    return info;
}
