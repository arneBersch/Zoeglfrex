/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "group.h"
#include "kernel/kernel.h"

Group::Group(Kernel* core) : Item(core) {}

Group::Group(const Group* item) : Item(item) {
    fixtures = item->fixtures;
}

Group::~Group() {
    if (kernel->cuelistView->currentGroup == this) {
        kernel->cuelistView->currentGroup = nullptr;
        kernel->cuelistView->loadCue();
    }
    for (Model* model : kernel->models->items) {
        for (QString groupSpecificIntensityAttribute : model->groupSpecificIntensityAttributes.keys()) {
            model->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        for (QString groupSpecificIntensityAttribute : fixture->groupSpecificIntensityAttributes.keys()) {
            fixture->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Group* group : kernel->groups->items) {
        for (QString groupSpecificIntensityAttribute : group->groupSpecificIntensityAttributes.keys()) {
            group->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Intensity* intensity : kernel->intensities->items) {
        for (QString groupSpecificIntensityAttribute : intensity->groupSpecificIntensityAttributes.keys()) {
            intensity->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Color* color : kernel->colors->items) {
        for (QString groupSpecificIntensityAttribute : color->groupSpecificIntensityAttributes.keys()) {
            color->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Raw* raw : kernel->raws->items) {
        for (QString groupSpecificIntensityAttribute : raw->groupSpecificIntensityAttributes.keys()) {
            raw->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
    }
    for (Cue *cue : kernel->cues->items) {
        for (QString groupSpecificIntensityAttribute : cue->groupSpecificIntensityAttributes.keys()) {
            cue->groupSpecificIntensityAttributes[groupSpecificIntensityAttribute].remove(this);
        }
        cue->colors.remove(this);
        cue->raws.remove(this);
    }
}

QString Group::name() {
    if (label.isEmpty()) {
        QString response = Item::name();
        for (Fixture* fixture : fixtures) {
            response += fixture->name() + "; ";
        }
        return response;
    }
    return Item::name();
}

QString Group::info() {
    QString info = Item::info();
    QString fixtureNames;
    for (Fixture* fixture : fixtures) {
        fixtureNames += fixture->id + ", ";
    }
    fixtureNames.chop(2);
    info += "\n" + kernel->groups->FIXTURESATTRIBUTEID + " Fixtures: " + fixtureNames;
    return info;
}

