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
    positions = item->positions;
    raws = item->raws;
    effects = item->effects;
}

Cue::~Cue() {
    if (getCuelist()->currentCue == this) {
        getCuelist()->currentCue = nullptr;
    }
    if (getCuelist()->lastCue == this) {
        getCuelist()->lastCue = nullptr;
    }
}

QString Cue::info() {
    QString info = Item::info();
    QStringList intensityValues;
    QStringList colorValues;
    QStringList positionValues;
    QStringList rawValues;
    QStringList effectValues;
    for (Group* group : kernel->groups->items) {
        if (intensities.contains(group)) {
            intensityValues.append(group->name() + " @ " + intensities.value(group)->name());
        }
        if (colors.contains(group)) {
            colorValues.append(group->name() + " @ " + colors.value(group)->name());
        }
        if (positions.contains(group)) {
            positionValues.append(group->name() + " @ " + positions.value(group)->name());
        }
        if (raws.contains(group)) {
            QStringList rawValueItems;
            for (Raw* raw : raws.value(group)) {
                rawValueItems.append(raw->name());
            }
            rawValues.append(group->name() + " @ " + rawValueItems.join(" + "));
        }
        if (effects.contains(group)) {
            QStringList effectValueItems;
            for (Effect* effect : effects.value(group)) {
                effectValueItems.append(effect->name());
            }
            effectValues.append(group->name() + " @ " + effectValueItems.join(" + "));
        }
    }
    info += "\n" + kernel->CUEINTENSITIESATTRIBUTEID + " Intensities: " + intensityValues.join("; ");
    info += "\n" + kernel->CUECOLORSATTRIBUTEID + " Colors: " + colorValues.join("; ");
    info += "\n" + kernel->CUEPOSITIONSATTRIBUTEID + " Positions: " + positionValues.join("; ");
    info += "\n" + kernel->CUERAWSATTRIBUTEID + " Raws: " + rawValues.join("; ");
    info += "\n" + kernel->CUEEFFECTSATTRIBUTEID + " Effects: " + effectValues.join("; ");
    info += "\n" + kernel->CUEFADEATTRIBUTEID + " Fade: " + QString::number(floatAttributes.value(kernel->CUEFADEATTRIBUTEID)) + "s";
    info += "\n" + kernel->CUEBLOCKATTRIBUTEID + " Block: ";
    if (boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)) {
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
        fileStream->writeAttribute("ID", kernel->CUEINTENSITIESATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        fileStream->writeCharacters(intensities.value(group)->id);
        fileStream->writeEndElement();
    }
    for (Group* group : colors.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->CUECOLORSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        fileStream->writeCharacters(colors.value(group)->id);
        fileStream->writeEndElement();
    }
    for (Group* group : positions.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->CUEPOSITIONSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        fileStream->writeCharacters(positions.value(group)->id);
        fileStream->writeEndElement();
    }
    for (Group* group : raws.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->CUERAWSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        QStringList rawIds;
        for (Raw* raw : raws.value(group)) {
            rawIds.append(raw->id);
        }
        fileStream->writeCharacters(rawIds.join("+"));
        fileStream->writeEndElement();
    }
    for (Group* group : effects.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->CUEEFFECTSATTRIBUTEID);
        fileStream->writeAttribute("Group", group->id);
        QStringList effectIds;
        for (Effect* effect : effects.value(group)) {
            effectIds.append(effect->id);
        }
        fileStream->writeCharacters(effectIds.join("+"));
        fileStream->writeEndElement();
    }
}

Cuelist* Cue::getCuelist() {
    Cuelist* item = nullptr;
    for (Cuelist* list : kernel->cuelists->items) {
        if (list->cues->items.contains(this)) {
            Q_ASSERT(item == nullptr);
            item = list;
        }
    }
    Q_ASSERT(item != nullptr);
    return item;
}
