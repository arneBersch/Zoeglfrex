/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelisttable.h"

CuelistTable::CuelistTable(Kernel *core) : ItemTable(core, Keys::Cuelist, "Cuelist", "Cuelists") {
    floatAttributes[kernel->CUELISTDIMMERATTRIBUTEID] = {"Dimmer", 100, 0, 100, "%"};
    intAttributes[kernel->CUELISTPRIORITYATTRIBUTEID] = {"Priority", 100, 0, 200};
}

void CuelistTable::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == kernel->CUELISTCUEATTRIBUTEID) {
        if (!((value.size() >= 2) && (value.first() == Keys::Cue)) && text.isEmpty()) {
            kernel->terminal->error("Can' set the current Cue of Cuelists because an invalid value was given.");
            return;
        }
        if (!value.isEmpty()) {
            value.removeFirst();
        }
        QString cueId = kernel->terminal->keysToId(value);
        if (!text.isEmpty()) {
            cueId = text;
        }
        if (cueId.isEmpty()) {
            kernel->terminal->error("Can't set Cue of Cuelists because of no valid Cue ID was given.");
            return;
        }
        int cuelistCounter = 0;
        for (QString id : ids) {
            Cuelist* cuelist = getItem(id);
            if (cuelist == nullptr) {
                cuelist = addItem(id);
            }
            Cue *cue = cuelist->cues->getItem(cueId);
            if (cue == nullptr) {
                kernel->terminal->warning("Can't set Cue of Cuelist " + cuelist->name() + " because Cue " + cueId + " doesn't exist in this Cuelist.");
            } else {
                cuelist->lastCue = cuelist->currentCue;
                cuelist->currentCue = cue;
                cuelist->totalFadeFrames = 0;
                cuelist->remainingFadeFrames = 0;
                cuelistCounter++;
            }
            emit dataChanged(index(getItemRow(cuelist->id), 0), index(getItemRow(cuelist->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        if (cuelistCounter >= 1) {
            kernel->terminal->success("Set Cue of " + QString::number(cuelistCounter) + " Cuelists to Cue " + cueId + ".");
        }
    } else {
        ItemTable::setAttribute(ids, attributes, value, text);
    }
}

Cuelist* CuelistTable::addItem(QString id) {
    Cuelist* cuelist = ItemTable<Cuelist>::addItem(id);
    cuelist->cues->cuelist = cuelist;
    return cuelist;
}
