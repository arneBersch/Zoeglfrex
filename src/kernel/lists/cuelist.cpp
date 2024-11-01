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

Cue* CueList::getCue(QString id) {
    int cueRow = getCueRow(id);
    if (cueRow < 0 || cueRow >= cues.size()) {
        return nullptr;
    }
    return cues[cueRow];
}

int CueList::getCueRow(QString id) {
    for (int cueRow = 0; cueRow < cues.size(); cueRow++) {
        if (cues[cueRow]->id == id) {
            return cueRow;
        }
    }
    return -1;
}

bool CueList::copyCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Cue can't be copied because it doesn't exist.");
            return false;
        }
        if (getCue(targetId) != nullptr) {
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

bool CueList::deleteCue(QList<QString> ids) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            kernel->terminal->error("Cue can't be deleted because it doesn't exist.");
            return false;
        }
        Cue *cue = cues[cueRow];
        cues.removeAt(cueRow);
        delete cue;
        if (kernel->cuelistView->currentCue == cue) {
            kernel->cuelistView->currentCue = nullptr;
        }
    }
    return true;
}

void CueList::deleteIntensity(Intensity* intensity) {
    for (Cue *cue : cues) {
        for (Group *group : cue->intensities.keys()) {
            if (cue->intensities.value(group) == intensity) {
                cue->intensities.remove(group);
            }
        }
    }
}

void CueList::deleteColor(Color* color) {
    for (Cue *cue : cues) {
        for (Group *group : cue->colors.keys()) {
            if (cue->colors.value(group) == color) {
                cue->colors.remove(group);
            }
        }
    }
}

void CueList::deleteTransition(Transition* transition) {
    QList<QString> invalidCues;
    for (Cue *cue : cues) {
        if (cue->transition == transition) {
            invalidCues.append(cue->id);
        }
    }
    deleteCue(invalidCues);
}

void CueList::deleteGroup(Group *group) {
    for (Cue *cue : cues) {
        cue->intensities.remove(group);
        cue->colors.remove(group);
    }
}

bool CueList::deleteCueGroupIntensity(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Group Intensity: Group " + groupId + " doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't delete Group Intensity because Intensity " + id + " doesn't exist");
            return false;
        }
        cue->intensities.remove(group);
    }
    return true;
}

bool CueList::deleteCueGroupColor(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Group Color: Group " + groupId + " doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't delete Cue Group Color because Color " + id + " doesn't exist");
            return false;
        }
        cue->colors.remove(group);
    }
    return true;
}

bool CueList::labelCue(QList<QString> ids, QString label) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Cue can't be labeled because it doesn't exist.");
            return false;
        }
        cue->label = label;
    }
    return true;
}

bool CueList::moveCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            kernel->terminal->error("Cue can't be moved because it doesn't exist.");
            return false;
        }
        if (getCue(targetId) != nullptr) {
            kernel->terminal->error("Cue can't be moved because Target ID is already used.");
            return false;
        }
        Cue* cue = cues[cueRow];
        cues.removeAt(cueRow);
        cue->id = targetId;
        int position = 0;
        for (int index=0; index < cues.size(); index++) {
            if (greaterId(cues[index]->id, targetId)) {
                position++;
            }
        }
        cues.insert(position, cue);
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
    for (int index=0; index < cues.size(); index++) {
        if (greaterId(cues[index]->id, id)) {
            position++;
        }
    }
    cues.insert(position, cue);
    return cue;
}

bool CueList::recordCueTransition(QList<QString> ids, QString transitionId) {
    Transition* transition = kernel->transitions->getTransition(transitionId);
    if (transition == nullptr) {
        kernel->terminal->error("Can't record Cue because Transition doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            cue = recordCue(id, transition);
        }
        cue->transition = transition;
    }
    return true;
}

bool CueList::recordCueIntensity(QList<QString> ids, QString groupId, QString intensityId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group doesn't exist.");
        return false;
    }
    Intensity* intensity = kernel->intensities->getIntensity(intensityId);
    if (intensity == nullptr) {
        kernel->terminal->error("Can't record Cue because Intensity doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't record Cue because no Transition was specified.");
            return false;
        }
        cue->intensities[group] = intensity;
    }
    return true;
}

bool CueList::recordCueColor(QList<QString> ids, QString groupId, QString colorId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group doesn't exist.");
        return false;
    }
    Color* color = kernel->colors->getColor(colorId);
    if (color == nullptr) {
        kernel->terminal->error("Can't record Cue because Color doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            kernel->terminal->error("Can't record Cue because no Transition was specified.");
            return false;
        }
        cue->colors[group] = color;
    }
    return true;
}

QList<QString> CueList::getIds() {
    QList<QString> ids;
    for (Cue *cue : cues) {
        ids.append(cue->id);
    }
    return ids;
}

int CueList::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return cues.size();
}

QVariant CueList::data(const QModelIndex &index, const int role) const {
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        return cues[row]->id;
    }
    return QVariant();
}
