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

Group::Group(const Group* item) : Item(item->kernel) {
    label = item->label;
    fixtures = item->fixtures;
}

Group::~Group() {
    if (kernel->cuelistView->currentGroup == this) {
        kernel->cuelistView->currentGroup = nullptr;
        kernel->cuelistView->loadCue();
    }
    for (Cue *cue : kernel->cues->items) {
        cue->intensities.remove(this);
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

