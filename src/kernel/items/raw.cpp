/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "raw.h"
#include "kernel/kernel.h"

Raw::Raw(Kernel* core) : Item(core) {}

Raw::Raw(const Raw* item) : Item(item) {
    label = item->label;
    channel = item->channel;
    value = item->value;
}

Raw::~Raw() {
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->raws.keys()) {
            if (cue->raws.contains(group)) {
                cue->raws[group].removeAll(this);
                if (cue->raws[group].isEmpty()) {
                    cue->raws.remove(group);
                }
            }
        }
    }
}

QString Raw::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(channel) + " @ " + QString::number(value);
    }
    return Item::name();
}

QString Raw::info() {
    QString info = Item::info();
    info += "\n" + kernel->raws->CHANNELATTRIBUTEID + " Channel: " + QString::number(channel);
    info += "\n" + kernel->raws->VALUEATTRIBUTEID + " Value: " + QString::number(value);
    return info;
}
