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

Model::Model(const Model* item) : Item(item->kernel) {
    label = item->label;
    channels = item->channels;
}

Model::~Model() {
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixture->model == this) {
            fixture->model = nullptr;
        }
    }
}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}

QString Model::info() {
    QString info = Item::info();
    info += "\n" + kernel->models->CHANNELSATTRIBUTEID + " Channels: " + channels;
    return info;
}
