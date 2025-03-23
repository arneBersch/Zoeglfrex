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
}

Fixture::~Fixture() {
    if (kernel->cuelistView->currentFixture == this) {
        kernel->cuelistView->currentFixture = nullptr;
        kernel->cuelistView->loadCue();
    }
    for (Model* currentModel : kernel->models->items) {
        for (QString fixtureSpecificFloatAttribute : currentModel->fixtureSpecificFloatAttributes.keys()) {
            currentModel->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : currentModel->fixtureSpecificAngleAttributes.keys()) {
            currentModel->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        for (QString fixtureSpecificFloatAttribute : fixture->fixtureSpecificFloatAttributes.keys()) {
            fixture->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : fixture->fixtureSpecificAngleAttributes.keys()) {
            fixture->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Group* group : kernel->groups->items) {
        group->fixtures.removeAll(this);
        for (QString fixtureSpecificFloatAttribute : group->fixtureSpecificFloatAttributes.keys()) {
            group->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : group->fixtureSpecificAngleAttributes.keys()) {
            group->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Intensity* intensity : kernel->intensities->items) {
        for (QString fixtureSpecificFloatAttribute : intensity->fixtureSpecificFloatAttributes.keys()) {
            intensity->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : intensity->fixtureSpecificAngleAttributes.keys()) {
            intensity->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Color* color : kernel->colors->items) {
        for (QString fixtureSpecificFloatAttribute : color->fixtureSpecificFloatAttributes.keys()) {
            color->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : color->fixtureSpecificAngleAttributes.keys()) {
            color->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Raw* raw : kernel->raws->items) {
        raw->fixtureSpecificChannelValues.remove(this);
        for (QString fixtureSpecificFloatAttribute : raw->fixtureSpecificFloatAttributes.keys()) {
            raw->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : raw->fixtureSpecificAngleAttributes.keys()) {
            raw->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
    for (Cue* cue : kernel->cues->items) {
        for (QString fixtureSpecificFloatAttribute : cue->fixtureSpecificFloatAttributes.keys()) {
            cue->fixtureSpecificFloatAttributes[fixtureSpecificFloatAttribute].remove(this);
        }
        for (QString fixtureSpecificAngleAttribute : cue->fixtureSpecificAngleAttributes.keys()) {
            cue->fixtureSpecificAngleAttributes[fixtureSpecificAngleAttribute].remove(this);
        }
    }
}

QString Fixture::name() {
    QString channels = "Dimmer";
    if (model != nullptr) {
        channels = model->stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID);
        if (!model->stringAttributes.value(kernel->models->LABELATTRIBUTEID).isEmpty()) {
            channels = model->stringAttributes.value(kernel->models->LABELATTRIBUTEID);
        }
    }
    if (stringAttributes.value(kernel->fixtures->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + channels + " (" + QString::number(intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID)) + ")";
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
    info += "\n" + kernel->fixtures->ADDRESSATTRIBUTEID + " Address: " + QString::number(intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID));
    return info;
}

void Fixture::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    if (model != nullptr) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->fixtures->MODELATTRIBUTEID);
        fileStream->writeCharacters(model->id);
        fileStream->writeEndElement();
    }
}
