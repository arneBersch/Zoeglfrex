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

Cue* CueList::recordCue(QString id, Transition *transition) {
    Cue *cue = new Cue(kernel);
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
