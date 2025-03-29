/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"

CueList::CueList(Kernel *core) : ItemList(core, Keys::Cue, "Cue", "Cues") {
    floatAttributes[FADEATTRIBUTEID] = {"Fade", 0, 0, 60, "s"};
    boolAttributes[BLOCKATTRIBUTEID] = {"Block", false};
}

void CueList::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == INTENSITIESATTRIBUTEID) { // Intensities
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
                cue->intensities.remove(group);
            }
            if (ids.length() == 1) {
                kernel->terminal->success("Removed Intensity entry of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + ".");
            } else {
                kernel->terminal->success("Removed Intensity entries of " + QString::number(ids.length()) + " Cues at Group " + group->name() + ".");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Intensity)) {
                    kernel->terminal->error("Can't set Cue Intensities because no Intensity was given.");
                    return;
                }
                value.removeFirst();
            }
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
                cue->intensities[group] = intensity;
            }
            if (ids.length()) {
                kernel->terminal->success("Set Intensities of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + " to Intensity " + intensity->name() + ".");
            } else {
                kernel->terminal->success("Set Intensities of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to Intensity " + intensity->name() + ".");
            }
        }
    } else if (attribute == COLORSATTRIBUTEID) { // Colors
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
                cue->colors.remove(group);
            }
            if (ids.length() == 1) {
                kernel->terminal->success("Removed Color entry of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + ".");
            } else {
                kernel->terminal->success("Removed Color entries of " + QString::number(ids.length()) + " Cues at Group " + group->name() + ".");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Color)) {
                    kernel->terminal->error("Can't set the Cue Colors because no Color was given.");
                    return;
                }
                value.removeFirst();
            }
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
                cue->colors[group] = color;
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set Colors of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + " to Color " + color->name() + ".");
            } else {
                kernel->terminal->success("Set Colors of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to Color " + color->name() + ".");
            }
        }
    } else if (attribute == RAWSATTRIBUTEID) { // Raws
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
            if (ids.length() == 1) {
                kernel->terminal->success("Removed Raw entries of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + ".");
            } else {
                kernel->terminal->success("Removed Raw entries of " + QString::number(ids.length()) + " Cues at Group " + group->name() + ".");
            }
        } else {
            if (text.isEmpty()) {
                if ((value.isEmpty()) || (value.first() != Keys::Raw)) {
                    kernel->terminal->error("Can't set Cue Raws because this requires no value or at least one Raw.");
                    return;
                }
                value.removeFirst();
            }
            QStringList rawIds = kernel->terminal->keysToSelection(value, Keys::Raw);
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
            if (ids.length() == 1) {
                kernel->terminal->success("Set Raws of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + " to " + QString::number(raws.length()) + " Raws.");
            } else {
                kernel->terminal->success("Set Raws of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to " + QString::number(raws.length()) + " Raws.");
            }
        }
    } else if (attribute == EFFECTSATTRIBUTEID) { // Effects
        Group* group = kernel->groups->getItem(attributes.value(Keys::Group));
        if (group == nullptr) {
            kernel->terminal->error("Can't set Cue Effects because an invalid Group was given.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Cue *cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                cue->effects.remove(group);
            }
            if (ids.length() == 1) {
                kernel->terminal->success("Removed Effect entries of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + ".");
            } else {
                kernel->terminal->success("Removed Effect entries of " + QString::number(ids.length()) + " Cues at Group " + group->name() + ".");
            }
        } else {
            if (text.isEmpty()) {
                if ((value.isEmpty()) || (value.first() != Keys::Effect)) {
                    kernel->terminal->error("Can't set Cue Effects because this requires no value or at least one Effect.");
                    return;
                }
                value.removeFirst();
            }
            QStringList effectIds = kernel->terminal->keysToSelection(value, Keys::Effect);
            if (!text.isEmpty()) {
                effectIds = text.split("+");
            }
            if (effectIds.isEmpty()) {
                kernel->terminal->error("Can't set Cue Effects because an invalid Effect selection was given.");
                return;
            }
            QList<Effect*> effects;
            for (QString effectId : effectIds) {
                Effect* effect = kernel->effects->getItem(effectId);
                if (effect == nullptr) {
                    kernel->terminal->warning("Can't add Effect " + effectId + " to Cues because it doesn't exist.");
                } else {
                    effects.append(effect);
                }
            }
            for (QString id : ids) {
                Cue* cue = getItem(id);
                if (cue == nullptr) {
                    cue = addItem(id);
                }
                cue->effects[group] = effects;
            }
            if (ids.length() == 1) {
                kernel->terminal->success("Set Effects of Cue " + getItem(ids.first())->name() + " at Group " + group->name() + " to " + QString::number(effects.length()) + " Effectss.");
            } else {
                kernel->terminal->success("Set Effects of " + QString::number(ids.length()) + " Cues at Group " + group->name() + " to " + QString::number(effects.length()) + " Effects.");
            }
        }
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
    }
}
