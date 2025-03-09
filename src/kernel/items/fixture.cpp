/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixture.h"
#include "kernel/kernel.h"

Fixture::Fixture(Kernel* core) : Item(core) {}

Fixture::Fixture(const Fixture* item) : Item(item) {
    model = item->model;
    address = 0; // using the same address as the other Fixture could result in an address conflict
}

Fixture::~Fixture() {
    for (Model* currentModel : kernel->models->items) {
        for (QString fixtureListAttribute : currentModel->fixtureListAttributes.keys()) {
            currentModel->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : currentModel->fixtureSpecificFloatAttributes.keys()) {
            currentModel->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        for (QString fixtureListAttribute : fixture->fixtureListAttributes.keys()) {
            fixture->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : fixture->fixtureSpecificFloatAttributes.keys()) {
            fixture->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Group* group : kernel->groups->items) {
        for (QString fixtureListAttribute : group->fixtureListAttributes.keys()) {
            group->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : group->fixtureSpecificFloatAttributes.keys()) {
            group->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Intensity* intensity : kernel->intensities->items) {
        for (QString fixtureListAttribute : intensity->fixtureListAttributes.keys()) {
            intensity->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : intensity->fixtureSpecificFloatAttributes.keys()) {
            intensity->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Color* color : kernel->colors->items) {
        for (QString fixtureListAttribute : color->fixtureListAttributes.keys()) {
            color->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : color->fixtureSpecificFloatAttributes.keys()) {
            color->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Raw* raw : kernel->raws->items) {
        for (QString fixtureListAttribute : raw->fixtureListAttributes.keys()) {
            raw->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : raw->fixtureSpecificFloatAttributes.keys()) {
            raw->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
    for (Cue* cue : kernel->cues->items) {
        for (QString fixtureListAttribute : cue->fixtureListAttributes.keys()) {
            cue->fixtureListAttributes[fixtureListAttribute].removeAll(this);
        }
        for (QString fixtureSpecificFloatAttribute : cue->fixtureSpecificFloatAttributes.keys()) {
            cue->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
    }
}

QString Fixture::name() {
    QString channels = "Channel";
    if (model != nullptr) {
        channels = model->channels;
    }
    if (label.isEmpty()) {
        return Item::name() + channels + " (" + QString::number(address) + ")";
    }
    return Item::name();
}

QString Fixture::info() {
    QString info = Item::info();
    info += "\n" + kernel->fixtures->MODELATTRIBUTEID + " Model: ";
    if (model == nullptr) {
        info += "None (Dimmer)";
    } else {
        info += model->name();
    }
    info += "\n" + kernel->fixtures->ADDRESSATTRIBUTEID + " Address: " + QString::number(address);
    return info;
}
