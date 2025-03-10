/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cue.h"
#include "kernel/kernel.h"

Cue::Cue(Kernel *core) : Item(core) {}

Cue::Cue(const Cue* item) : Item(item) {
    intensities = item->intensities;
    colors = item->colors;
    raws = item->raws;
}

Cue::~Cue() {
    if (kernel->cuelistView->currentCue == this) {
        kernel->cuelistView->currentCue = nullptr;
        kernel->cuelistView->loadCue();
    }
}

QString Cue::info() {
    QString info = Item::info();
    QString intensityValues;
    QString colorValues;
    QString rawValues;
    for (Group* group : kernel->groups->items) {
        if (intensities.contains(group)) {
            intensityValues += group->id + " @ " + intensities.value(group)->id + "; ";
        }
        if (colors.contains(group)) {
            colorValues += group->id + " @ " + colors.value(group)->id + "; ";
        }
        if (raws.contains(group)) {
            rawValues += group->id + " @ ";
            for (Raw* raw : raws.value(group)) {
                rawValues += raw->id + ", ";
            }
            rawValues.chop(2);
            rawValues += "; ";
        }
    }
    intensityValues.chop(2);
    colorValues.chop(2);
    rawValues.chop(2);
    info += "\n" + kernel->cues->INTENSITIESATTRIBUTEID + " Intensities: " + intensityValues;
    info += "\n" + kernel->cues->COLORSATTRIBUTEID + " Colors: " + colorValues;
    info += "\n" + kernel->cues->RAWSATTRIBUTEID + " Raws: " + rawValues;
    info += "\n" + kernel->cues->FADEATTRIBUTEID + " Fade: " + QString::number(floatAttributes.value(kernel->cues->FADEATTRIBUTEID)) + "s";
    return info;
}
