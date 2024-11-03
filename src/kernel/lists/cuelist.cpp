/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"

CueList::CueList(Kernel *core) {
    kernel = core;
}

Cue* CueList::getItem(QString id) const {
    int cueRow = getItemRow(id);
    if (cueRow < 0 || cueRow >= items.size()) {
        return nullptr;
    }
    return items[cueRow];
}

int CueList::getItemRow(QString id) const {
    for (int cueRow = 0; cueRow < items.size(); cueRow++) {
        if (items[cueRow]->id == id) {
            return cueRow;
        }
    }
    return -1;
}

Cue* CueList::getItemByRow(int row) const {
    if (row >= items.size() || row < 0) {
        return nullptr;
    }
    return items[row];
}

bool CueList::copyItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->error("Cue can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Cue can't be copied because Target ID is already used.");
            return false;
        }
        Cue *targetCue = recordCue(targetId, cue->transition);
        targetCue->label = cue->label;
        targetCue->intensities = cue->intensities;
        targetCue->colors = cue->colors;
        targetCue->transition = cue->transition;
    }
    return true;
}

bool CueList::deleteItems(QList<QString> ids) {
    for (QString id : ids) {
        int cueRow = getItemRow(id);
        if (cueRow < 0) {
            kernel->terminal->error("Cue can't be deleted because it doesn't exist.");
            return false;
        }
        Cue *cue = items[cueRow];
        items.removeAt(cueRow);
        delete cue;
        if (kernel->cuelistView->currentCue == cue) {
            kernel->cuelistView->currentCue = nullptr;
        }
    }
    return true;
}

void CueList::deleteIntensity(Intensity* intensity) {
    for (Cue *cue : items) {
        for (Group *group : cue->intensities.keys()) {
            if (cue->intensities.value(group) == intensity) {
                cue->intensities.remove(group);
            }
        }
    }
}

void CueList::deleteColor(Color* color) {
    for (Cue *cue : items) {
        for (Group *group : cue->colors.keys()) {
            if (cue->colors.value(group) == color) {
                cue->colors.remove(group);
            }
        }
    }
}

void CueList::deleteTransition(Transition* transition) {
    QList<QString> invalidCues;
    for (Cue *cue : items) {
        if (cue->transition == transition) {
            invalidCues.append(cue->id);
        }
    }
    deleteItems(invalidCues);
}

void CueList::deleteGroup(Group *group) {
    for (Cue *cue : items) {
        cue->intensities.remove(group);
        cue->colors.remove(group);
    }
}

bool CueList::deleteCueGroupIntensity(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Group Intensity: Group " + groupId + " doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue *cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't delete Group Intensity because Intensity " + id + " doesn't exist");
            return false;
        }
        cue->intensities.remove(group);
    }
    return true;
}

bool CueList::deleteCueGroupColor(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Group Color: Group " + groupId + " doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue *cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't delete Cue Group Color because Color " + id + " doesn't exist");
            return false;
        }
        cue->colors.remove(group);
    }
    return true;
}

bool CueList::moveItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int cueRow = getItemRow(id);
        if (cueRow < 0) {
            kernel->terminal->error("Cue can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Cue can't be moved because Target ID is already used.");
            return false;
        }
        Cue* cue = items[cueRow];
        items.removeAt(cueRow);
        cue->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, cue);
    }
    return true;
}

Cue* CueList::recordCue(QString id, Transition *transition) {
    Cue *cue = new Cue;
    cue->id = id;
    cue->label = QString();
    cue->transition = transition;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, cue);
    return cue;
}

bool CueList::recordCueTransition(QList<QString> ids, QString transitionId) {
    Transition* transition = kernel->transitions->getItem(transitionId);
    if (transition == nullptr) {
        kernel->terminal->error("Can't record Cue because Transition doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            cue = recordCue(id, transition);
        }
        cue->transition = transition;
    }
    return true;
}

bool CueList::recordCueIntensity(QList<QString> ids, QString groupId, QString intensityId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group doesn't exist.");
        return false;
    }
    Intensity* intensity = kernel->intensities->getItem(intensityId);
    if (intensity == nullptr) {
        kernel->terminal->error("Can't record Cue because Intensity doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't record Cue because no Transition was specified.");
            return false;
        }
        cue->intensities[group] = intensity;
    }
    return true;
}

bool CueList::recordCueColor(QList<QString> ids, QString groupId, QString colorId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group doesn't exist.");
        return false;
    }
    Color* color = kernel->colors->getItem(colorId);
    if (color == nullptr) {
        kernel->terminal->error("Can't record Cue because Color doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't record Cue because no Transition was specified.");
            return false;
        }
        cue->colors[group] = color;
    }
    return true;
}

QList<QString> CueList::getIds() const {
    QList<QString> ids;
    for (Cue *cue : items) {
        ids.append(cue->id);
    }
    return ids;
}

int CueList::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return items.size();
}
