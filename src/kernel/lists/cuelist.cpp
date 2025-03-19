/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"

CueList::CueList(Kernel *core) : ItemList(Keys::Cue, "Cue", "Cues") {
    kernel = core;
    floatAttributes[FADEATTRIBUTEID] = {"Fade", 0, 0, 60, "s"};
}

void CueList::setAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == INTENSITIESATTRIBUTEID) {
        if (text.isEmpty()) {
            if (value.isEmpty() || (value.first() != Keys::Intensity)) {
                kernel->terminal->error("Can't set Cue Intensities because no Intensity was given.");
                return;
            }
            value.removeFirst();
        }
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Intensities because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                if (cue->intensities.contains(group)) {
                    Intensity* oldIntensity = cue->intensities.value(group);
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && items[cueRow]->intensities.contains(group) && (items[cueRow]->intensities.value(group) == oldIntensity)) {
                        items[cueRow]->intensities.remove(group);
                        cueRow++;
                    }
                }
            }
            kernel->terminal->success("Deleted " + QString::number(ids.length()) + + " Cue Intensity entries.");
        } else {
            Intensity* intensity = kernel->intensities->getItem(kernel->terminal->keysToId(value));
            if (!text.isEmpty()) {
                intensity = kernel->intensities->getItem(text);
            }
            if (intensity == nullptr) {
                kernel->terminal->error("Can't set Cue Intensities because Intensity " + kernel->terminal->keysToId(value) + " doesn't exist.");
                return;
            }
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                if (cue->intensities.contains(group)) {
                    Intensity* oldIntensity = cue->intensities.value(group);
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && items[cueRow]->intensities.contains(group) && (items[cueRow]->intensities.value(group) == oldIntensity)) {
                        items[cueRow]->intensities[group] = intensity;
                        cueRow++;
                    }
                } else {
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && !items[cueRow]->intensities.contains(group)) {
                        items[cueRow]->intensities[group] = intensity;
                        cueRow++;
                    }
                }
            }
            kernel->terminal->success("Set Intensities of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to Intensity " + intensity->name() + ".");
        }
    } else if (attribute == COLORSATTRIBUTEID) {
        if (text.isEmpty()) {
            if (value.isEmpty() || (value.first() != Keys::Color)) {
                kernel->terminal->error("Can't set the Cue Colors because no Color was given.");
                return;
            }
            value.removeFirst();
        }
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Colors because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                if (cue->colors.contains(group)) {
                    Color* oldColor = cue->colors.value(group);
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && items[cueRow]->colors.contains(group) && (items[cueRow]->colors.value(group) == oldColor)) {
                        items[cueRow]->colors.remove(group);
                        cueRow++;
                    }
                }
            }
            kernel->terminal->success("Deleted " + QString::number(ids.size()) + " Cue Color entries.");
        } else {
            Color* color = kernel->colors->getItem(kernel->terminal->keysToId(value));
            if (!text.isEmpty()) {
                color = kernel->colors->getItem(text);
            }
            if (color == nullptr) {
                kernel->terminal->error("Can't set Cue Colors because an invalid Color ID was given.");
                return;
            }
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                if (cue->colors.contains(group)) {
                    Color* oldColor = cue->colors.value(group);
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && items[cueRow]->colors.contains(group) && (items[cueRow]->colors.value(group) == oldColor)) {
                        items[cueRow]->colors[group] = color;
                        cueRow++;
                    }
                } else {
                    int cueRow = getItemRow(id);
                    while ((cueRow < items.size()) && !items[cueRow]->colors.contains(group)) {
                        items[cueRow]->colors[group] = color;
                        cueRow++;
                    }
                }
            }
            kernel->terminal->success("Set Colors of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to Color " + color->name() + ".");
        }
    } else if (attribute == RAWSATTRIBUTEID) {
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Raws because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                cue->raws.remove(group);
            }
            kernel->terminal->success("Deleted " + QString::number(ids.length()) + " Cue Raw entries.");
        } else {
            if (text.isEmpty()) {
                if ((value.isEmpty()) || (value.first() != Keys::Raw)) {
                    kernel->terminal->error("Can't set Cue Raws because this requires no value or at least one Raw.");
                    return;
                }
                value.removeFirst();
            }
            QList<QString> rawIds = kernel->terminal->keysToSelection(value, Keys::Raw);
            if (!text.isEmpty()) {
                rawIds = text.split("+");
            }
            if (rawIds.isEmpty()) {
                kernel->terminal->error("Can't set Cue Raws because an invalid Raw selection was given.");
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
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                cue->raws[group] = raws;
            }
            kernel->terminal->success("Set Raws of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to " + QString::number(raws.length()) + " Raws.");
        }
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
    }
}
