/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"
#include "kernel/kernel.h"

Cuelist::Cuelist(Kernel* core) : Item(core) {
    cues = new CueTable(core);
}

Cuelist::Cuelist(const Cuelist* item) : Item(item) {
    cues = new CueTable(item->cues);
}

Cuelist::~Cuelist() {
    delete cues;
    if (kernel->cuelistView->currentCuelist == this) {
        kernel->cuelistView->currentCuelist = nullptr;
        kernel->cuelistView->currentCue = nullptr;
        kernel->cuelistView->reload();
    }
}

QString Cuelist::name() {
    if (stringAttributes.value(kernel->LABELATTRIBUTEID).isEmpty()) {
        return Item::name() + QString::number(cues->items.size()) + " Cues";
    }
    return Item::name();
}

QString Cuelist::info() {
    QString info = Item::info();
    QString cue = "No Cue selected.";
    if (currentCue != nullptr) {
        cue = currentCue->name();
    }
    info += "\n" + kernel->CUELISTCUEATTRIBUTEID + " Current Cue: " + cue;

    info += "\n" + kernel->CUELISTPRIORITYATTRIBUTEID + " Priority: " + QString::number(intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID));

    info += "\n" + kernel->CUELISTMOVEWHILEDARKATTRIBUTEID + " Move while Dark: ";
    if (boolAttributes.value(kernel->CUELISTMOVEWHILEDARKATTRIBUTEID)) {
        info += "True";
    } else {
        info += "False";
    }

    return info;
}

void Cuelist::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    cues->saveItemsToFile(fileStream);
    if (currentCue != nullptr) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", kernel->CUELISTCUEATTRIBUTEID);
        fileStream->writeCharacters(currentCue->id);
        fileStream->writeEndElement();
    }
}

void Cuelist::goToCue(QString cueId) {
    Cue* cue = cues->getItem(cueId);
    if (cue == nullptr) {
        kernel->terminal->error("Can't load Cue " + cueId + " of Cuelist " + name() + " because this Cue doesn't exist.");
        return;
    }
    previousCue = currentCue;
    currentCue = cue;
    totalFadeFrames = kernel->dmxEngine->PROCESSINGRATE * currentCue->floatAttributes[kernel->CUEFADEATTRIBUTEID] + 0.5;
    remainingFadeFrames = totalFadeFrames;
    kernel->cuelistView->reload();
}

void Cuelist::go() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        return;
    }
    if ((cues->items.indexOf(currentCue) + 1) < cues->items.size()) {
        goToCue(cues->items[cues->items.indexOf(currentCue) + 1]->id);
    }
}

void Cuelist::goBack() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        return;
    }
    if (cues->items.indexOf(currentCue) > 0) {
        goToCue(cues->items[cues->items.indexOf(currentCue) - 1]->id);
    }
}
