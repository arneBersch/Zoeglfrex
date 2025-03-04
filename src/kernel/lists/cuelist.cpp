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
    floatAttributes[FADEATTRIBUTEID] ={"Fade", 0, 0, 60, "s"};
}

void CueList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attributeString = attributes.value(Keys::Attribute);
    if (attributeString == INTENSITIESATTRIBUTEID) {
        if (value.isEmpty() || (value.first() != Keys::Intensity)) {
            kernel->terminal->error("Can't set Cue Intensities because no Intensity was given.");
            return;
        }
        value.removeFirst();
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Intensities because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            int cueCounter = 0;
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Cue Intensites in Cue " + id + " because it doesn't exist.");
                } else {
                    cue->intensities.remove(group);
                    cueCounter++;
                }
            }
            kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Intensity entries.");
        } else {
            Intensity* intensity = kernel->intensities->getItem(kernel->keysToId(value));
            if (intensity == nullptr) {
                kernel->terminal->error("Can't set Cue Intensities because Intensity " + kernel->keysToId(value) + " doesn't exist.");
                return;
            }
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Cue Intensities of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->intensities[group] = intensity;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Intensities Attribute of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to Intensity " + intensity->name() + ".");
        }
    } else if (attributeString == COLORSATTRIBUTEID) {
        if (value.isEmpty() || (value.first() != Keys::Color)) {
            kernel->terminal->error("Can't set the Cue Colors because no Color was given.");
            return;
        }
        value.removeFirst();
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Colors because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            int cueCounter = 0;
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Cue Colors of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->colors.remove(group);
                    cueCounter++;
                }
            }
            kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Color entries.");
        } else {
            Color* color = kernel->colors->getItem(kernel->keysToId(value));
            if (color == nullptr) {
                kernel->terminal->error("Can't set Cue Colors because an invalid Color ID was given.");
                return;
            }
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Cue Colors of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->colors[group] = color;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Colors of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to Color " + color->name() + ".");
        }
    } else if (attributeString == RAWSATTRIBUTEID) {
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Raws because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            int cueCounter = 0;
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't delete Cue Raw Entry in Cue " + id + " because it doesn't exist.");
                } else {
                    cue->raws.remove(group);
                    cueCounter++;
                }
            }
            kernel->terminal->success("Deleted " + QString::number(cueCounter) + " Cue Raw entries.");
        } else {
            if ((value.isEmpty()) || (value.first() != Keys::Raw)) {
                kernel->terminal->error("Can't set Cue Raws because this requires no value or at least one Raw.");
                return;
            }
            value.removeFirst();
            QList<QString> rawIds = kernel->keysToSelection(value, Keys::Raw);
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
            int cueCounter = 0;
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    kernel->terminal->warning("Can't set Raws of Cue " + id + " because it doesn't exist.");
                } else {
                    cue->raws[group] = raws;
                    cueCounter++;
                }
            }
            kernel->terminal->success("Set Raws of " + QString::number(cueCounter) + " Cues at Group " + group->name() + " to " + QString::number(raws.length()) + " Raws.");
        }
    } else {
        kernel->terminal->error("Can't set Cue Attribute " + attributeString + ".");
    }
}
