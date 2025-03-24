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
        kernel->cuelistView->loadView();
    }
}

QString Cue::info() {
    QString info = Item::info();
    QStringList intensityValues;
    for (Group* group : intensities.keys()) {
        intensityValues.append(group->name() + " @ " + intensities.value(group)->name());
    }
    info += "\n" + kernel->cues->INTENSITIESATTRIBUTEID + " Intensities: " + intensityValues.join("; ");
    QStringList colorValues;
    for (Group* group : colors.keys()) {
        colorValues.append(group->name() + " @ " + colors.value(group)->name());
    }
    info += "\n" + kernel->cues->COLORSATTRIBUTEID + " Colors: " + colorValues.join("; ");
    QStringList rawValues;
    for (Group* group : raws.keys()) {
        QStringList rawValueItems;
        for (Raw* raw : raws.value(group)) {
            rawValueItems.append(raw->name());
        }
        rawValues.append(group->name() + " @ " + rawValueItems.join(" + "));
    }
    info += "\n" + kernel->cues->RAWSATTRIBUTEID + " Raws: " + rawValues.join("; ");
    info += "\n" + kernel->cues->FADEATTRIBUTEID + " Fade: " + QString::number(floatAttributes.value(kernel->cues->FADEATTRIBUTEID)) + "s";
    info += "\n" + kernel->cues->BLOCKATTRIBUTEID + " Block: ";
    if (boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)) {
        info += "True";
    } else {
        info += "False";
    }
    return info;
}

void Cue::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    for (Group* group : intensities.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->cues->INTENSITIESATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        fileStream->writeCharacters(intensities.value(group)->id);
        fileStream->writeEndElement();
    }
    for (Group* group : colors.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->cues->COLORSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        fileStream->writeCharacters(colors.value(group)->id);
        fileStream->writeEndElement();
    }
    for (Group* group : raws.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->cues->RAWSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        QStringList rawIds;
        for (Raw* raw : raws.value(group)) {
            rawIds.append(raw->id);
        }
        fileStream->writeCharacters(rawIds.join("+"));
        fileStream->writeEndElement();
    }
}
