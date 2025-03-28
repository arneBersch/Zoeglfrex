/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effect.h"
#include "kernel/kernel.h"

Effect::Effect(Kernel* core) : Item(core) {}

Effect::Effect(const Effect* item) : Item(item) {}

Effect::~Effect() {
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->effects.keys()) {
            if (cue->effects.contains(group)) {
                cue->effects[group].removeAll(this);
                if (cue->effects[group].isEmpty()) {
                    cue->effects.remove(group);
                }
            }
        }
    }
}

QString Effect::info() {
    QString info = Item::info();
    return info;
}

void Effect::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
}
