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

void CueList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> value, QString text) {
    QString attributeString = attribute.value(Keys::Attribute);
    if (attributeString == "2") {
        if (value.isEmpty() || (value.first() != Keys::Intensity)) {
            kernel->terminal->error("Cue Attribute 2 Group Set requires an Intensity.");
            return;
        }
        value.removeFirst();
        Group* group = kernel->groups->getItem(attribute.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Attribute 2 because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
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
        } else {
            Intensity* intensity = kernel->intensities->getItem(kernel->keysToId(value));
            if (intensity == nullptr) {
                kernel->terminal->error("Cue Attribute 2 Group Set received an invalid Intensity ID.");
                return;
            }
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Attribute 3 of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->intensities[group] = intensity;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Attribute 2 of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to Intensity " + intensity->name() + ".");
        }
    } else if (attributeString == "3") {
        if (value.isEmpty() || (value.first() != Keys::Color)) {
            kernel->terminal->error("Cue Attribute 3 Group Set requires an Color.");
            return;
        }
        value.removeFirst();
        Group* group = kernel->groups->getItem(attribute.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Attribute 3 because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
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
        } else {
            Color* color = kernel->colors->getItem(kernel->keysToId(value));
            if (color == nullptr) {
                kernel->terminal->error("Cue Attribute 3 Group Set received an invalid Color ID.");
                return;
            }
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Attribute 3 of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->colors[group] = color;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Attribute 3 of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to Color " + color->name() + ".");
        }
    } else if (attributeString == "4") {
        Group* group = kernel->groups->getItem(attribute.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Attribute 4 because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            int cueCounter = 0;
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't delete Cue Raw Entry because Cue " + id + " doesn't exist.");
                } else {
                    cue->raws.remove(group);
                    cueCounter++;
                }
            }
            kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Raw entries.");
        } else {
            if (value.first() != Keys::Raw) {
                kernel->terminal->error("Cue Attribute 4 Group Set requires no value or at least one Raw.");
                return;
            }
            value.removeFirst();
            QList<QString> rawIds = kernel->keysToSelection(value, Keys::Raw);
            if (rawIds.isEmpty()) {
                kernel->terminal->error("Cue Attribute 4 Group Set received invalid Raws.");
                return;
            }
            QList<Raw*> raws;
            for (QString rawId : rawIds) {
                Raw* raw = kernel->raws->getItem(rawId);
                if (raw == nullptr) {
                    kernel->terminal->warning("Can't add Raw " + rawId + " to Cues because it doesn't exist.");
                } else {
                    raws.append(raw);
                }
            }
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Attribute 4 of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->raws[group] = raws;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Attribute 4 of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to " + QString::number(raws.length()) + " Raws.");
        }
    } else if (attributeString == "5") {
        float fade = kernel->keysToValue(value);
        if (fade < 0 || fade > 60) {
            kernel->terminal->error("Can't set Cue Fade because Fade has to be between 0 and 60 seconds.");
            return;
        }
        for (QString id : ids) {
            Cue* cue = getItem(id);
            if (cue == nullptr) {
                cue = addItem(id);
            }
            cue->fade = fade;
        }
        kernel->terminal->success("Set Fade of " + QString::number(ids.length()) + " Cues to " + QString::number(fade) + "s.");
    } else {
        kernel->terminal->error("Can't set Cue attribute " + attributeString + ".");
    }
}
