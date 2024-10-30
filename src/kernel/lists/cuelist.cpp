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

QString CueList::copyCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Cue can't be copied because it doesn't exist.";
        }
        if (getCue(targetId) != nullptr) {
            return "Cue can't be copied because Target ID is already used.";
        }
        Cue *targetCue = recordCue(targetId, cue->transition);
        targetCue->label = cue->label;
        targetCue->intensities = cue->intensities;
        targetCue->colors = cue->colors;
        targetCue->transition = cue->transition;
    }
    return QString();
}

QString CueList::deleteCue(QList<QString> ids) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            return "Cue can't be deleted because it doesn't exist.";
        }
        Cue *cue = cues[cueRow];
        cues.removeAt(cueRow);
        delete cue;
        if (kernel->cuelistView->currentCue == cue) {
            kernel->cuelistView->currentCue = nullptr;
        }
    }
    return QString();
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

QString CueList::deleteCueGroupIntensity(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        return tr("Can't delete Cue Group Intensity: Group %1 doesn't exist.").arg(groupId);
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            return tr("Can't delete Group Intensity because Intensity %1 doesn't exist").arg(id);
        }
        cue->intensities.remove(group);
    }
    return QString();
}

QString CueList::deleteCueGroupColor(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        return tr("Can't delete Cue Group Color: Group %1 doesn't exist.").arg(groupId);
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            return tr("Can't delete Cue Group Color because Color %1 doesn't exist").arg(id);
        }
        cue->colors.remove(group);
    }
    return QString();
}

QString CueList::labelCue(QList<QString> ids, QString label) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Cue can't be labeled because it doesn't exist.";
        }
        cue->label = label;
    }
    return QString();
}

QString CueList::moveCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            return "Cue can't be moved because it doesn't exist.";
        }
        if (getCue(targetId) != nullptr) {
            return "Cue can't be moved because Target ID is already used.";
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
    return QString();
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

QString CueList::recordCueTransition(QList<QString> ids, QString transitionId) {
    Transition* transition = kernel->transitions->getTransition(transitionId);
    if (transition == nullptr) {
        return "Can't record Cue because Transition doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            cue = recordCue(id, transition);
        }
        cue->transition = transition;
    }
    return QString();
}

QString CueList::recordCueIntensity(QList<QString> ids, QString groupId, QString intensityId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        return "Can't record Cue because Group doesn't exist.";
    }
    Intensity* intensity = kernel->intensities->getIntensity(intensityId);
    if (intensity == nullptr) {
        return "Can't record Cue because Intensity doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Can't record Cue because no Transition was specified.";
        }
        cue->intensities[group] = intensity;
    }
    return QString();
}

QString CueList::recordCueColor(QList<QString> ids, QString groupId, QString colorId) {
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        return "Can't record Cue because Group doesn't exist.";
    }
    Color* color = kernel->colors->getColor(colorId);
    if (color == nullptr) {
        return "Can't record Cue because Color doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Can't record Cue because no Transition was specified.";
        }
        cue->colors[group] = color;
    }
    return QString();
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
