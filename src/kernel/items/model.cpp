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
    for (Raw* raw : kernel->raws->items) {
        for (QString modelSpecificFloatAttribute : raw->modelSpecificFloatAttributes.keys()) {
            raw->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : raw->modelSpecificAngleAttributes.keys()) {
            raw->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
    for (Cue* cue : kernel->cues->items) {
        for (QString modelSpecificFloatAttribute : cue->modelSpecificFloatAttributes.keys()) {
            cue->modelSpecificFloatAttributes[modelSpecificFloatAttribute].remove(this);
        }
        for (QString modelSpecificAngleAttribute : cue->modelSpecificAngleAttributes.keys()) {
            cue->modelSpecificAngleAttributes[modelSpecificAngleAttribute].remove(this);
        }
    }
}

QString Model::name() {
    if (stringAttributes.value(kernel->models->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID);
    }
    return Item::name();
}

QString Model::info() {
    QString info = Item::info();
    info += "\n" + kernel->models->CHANNELSATTRIBUTEID + " Channels: " + stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID);
    return info;
}
