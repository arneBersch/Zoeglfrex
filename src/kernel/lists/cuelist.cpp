/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"

CueList::CueList(Kernel *core) : ItemList("Cue", "Cues") {
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

void CueList::deleteGroup(Group *group) {
    for (Cue *cue : items) {
        cue->intensities.remove(group);
        cue->colors.remove(group);
    }
}

void CueList::deleteCueGroupIntensity(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Intensity Entry because Group " + groupId + " doesn't exist.");
        return;
    }
    int cueCounter = 0;
    for (QString id : ids) {
        Cue *cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->warning("Can't delete Cue Intensity Entry because Cue " + id + " doesn't exist.");
        } else {
            cue->intensities.remove(group);
            cueCounter++;
        }
    }
    kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Intensity entries.");
}

void CueList::deleteCueGroupColor(QList<QString> ids, QString groupId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't delete Cue Color Entry because Group " + groupId + " doesn't exist.");
        return;
    }
    int cueCounter = 0;
    for (QString id : ids) {
        Cue *cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->warning("Can't delete Cue Color Entry because Cue " + id + " doesn't exist.");
        } else {
            cue->colors.remove(group);
            cueCounter++;
        }
    }
    kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Color entries.");
}

void CueList::setAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> value, QString text) {
    QString attributeString = attribute.value(Keys::Attribute);
    float floatValue = kernel->keysToValue(value);
    if (attributeString == "2") {
        if (value.isEmpty() || (value[0] != Keys::Intensity)) {
            kernel->terminal->error("Cue Attribute 2 Group Set requies an Intensity.");
            return;
        }
        value.removeFirst();
        QString groupId = attribute.value(Keys::Group);
        if (groupId.isEmpty()) {
            kernel->terminal->error("Can't set Cue Attribute 2 because an invalid Group was given.");
            return;
        }
        QString intensityId = kernel->keysToId(value);
        if (intensityId.isEmpty()) {
            kernel->terminal->error("Cue Attribute 2 Group Set received an invalid Intensity ID.");
            return;
        }
            recordCueIntensity(ids, groupId, intensityId);
    } else if (attributeString == "3") {
        if (value.isEmpty() || (value[0] != Keys::Color)) {
            kernel->terminal->error("Cue Attribute 3 Group Set requies an Color.");
            return;
        }
        value.removeFirst();
        QString groupId = attribute.value(Keys::Group);
        if (groupId.isEmpty()) {
            kernel->terminal->error("Can't set Cue Attribute 3 because an invalid Group was given.");
            return;
        }
        QString colorId = kernel->keysToId(value);
        if (colorId.isEmpty()) {
            kernel->terminal->error("Cue Attribute 3 Group Set received an invalid Color ID.");
            return;
        }
        recordCueColor(ids, groupId, colorId);
    } else if (attributeString == "4") {
        if (floatValue < 0 || floatValue > 60) {
            kernel->terminal->error("Can't set Cue Fade because Fade has to be between 0 and 60 seconds.");
            return;
        }
        for (QString id : ids) {
            Cue* cue = getItem(id);
            if (cue == nullptr) {
                cue = recordItem(id);
            }
            cue->fade = floatValue;
        }
        kernel->terminal->success("Set Fade of " + QString::number(ids.length()) + " Cues to " + QString::number(floatValue) + "s.");
    } else {
        kernel->terminal->error("Can't set Cue attribute " + attributeString + ".");
    }
}

void CueList::recordCueIntensity(QList<QString> ids, QString groupId, QString intensityId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group " + groupId + " doesn't exist.");
        return;
    }
    Intensity* intensity = kernel->intensities->getItem(intensityId);
    if (intensity == nullptr) {
        kernel->terminal->error("Can't record Cue because Intensity " + intensityId + " doesn't exist.");
        return;
    }
    int cueCounter = 0;
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->warning("Can't record Cue " + id + " because it doesn't exist.");
        } else {
            cue->intensities[group] = intensity;
            cueCounter++;
        }
    }
    kernel->terminal->success("Recorded " + QString::number(cueCounter) + " Cues with Intensity " + intensityId + " at Group " + groupId + ".");
}

void CueList::recordCueColor(QList<QString> ids, QString groupId, QString colorId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't record Cue because Group " + groupId + " doesn't exist.");
        return;
    }
    Color* color = kernel->colors->getItem(colorId);
    if (color == nullptr) {
        kernel->terminal->error("Can't record Cue because Color " + colorId + " doesn't exist.");
        return;
    }
    int cueCounter = 0;
    for (QString id : ids) {
        Cue* cue = getItem(id);
        if (cue == nullptr) {
            kernel->terminal->warning("Can't record Cue " + id + " because it doesn't exist.");
        } else {
            cue->colors[group] = color;
            cueCounter++;
        }
    }
    kernel->terminal->success("Recorded " + QString::number(cueCounter) + " Cues with Color " + colorId + " at Group " + groupId + ".");
}
