/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effectlist.h"

EffectList::EffectList(Kernel *core) : ItemList(core, Keys::Effect, "Effect", "Effects") {
    intAttributes[STEPSATTRIBUTEID] = {"Steps", 2, 2, 99};
}

void EffectList::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute.startsWith(INTENSITYSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(INTENSITYSTEPSATTRIBUTEID + ".").length());
        bool ok;
        int step = attribute.toInt(&ok);
        if (!ok) {
            kernel->terminal->error("Can't set Effect Intensity Step because no valid Attribute was given.");
            return;
        }
        if (step < 1) {
            kernel->terminal->error("Can't set Effect Intensity Step because Step has to be at least 1.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                effect->intensitySteps.remove(step);
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Intensity Step " + QString::number(step) + " of Effect " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed Intensity Step " + QString::number(step) + " of " + QString::number(ids.length()) + " Effects.");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Intensity)) {
                    kernel->terminal->error("Can't set Effect Intensity Steps because no Intensity was given.");
                    return;
                }
                value.removeFirst();
            }
            Intensity* intensity = kernel->intensities->getItem(kernel->terminal->keysToId(value));
            if (!text.isEmpty()) {
                intensity = kernel->intensities->getItem(text);
            }
            if (intensity == nullptr) {
                kernel->terminal->error("Can't set Effect Intensity Steps because Intensity " + kernel->terminal->keysToId(value) + " doesn't exist.");
                return;
            }
            int effectCounter = 0;
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                if (step > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Intensity Step " + QString::number(step) + " of Effect " + id + " because this Effect only has " + QString::number(effect->intAttributes.value(STEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->intensitySteps[step] = intensity;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Intensity Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to Intensity " + intensity->name() + ".");
        }
    } else if (attribute.startsWith(COLORSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(COLORSTEPSATTRIBUTEID + ".").length());
        bool ok;
        int step = attribute.toInt(&ok);
        if (!ok) {
            kernel->terminal->error("Can't set Effect Color Step because no valid Attribute was given.");
            return;
        }
        if (step < 1) {
            kernel->terminal->error("Can't set Effect Color Step because Step has to be at least 1.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                effect->colorSteps.remove(step);
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Color Step " + QString::number(step) + " of Effect " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed Color Step " + QString::number(step) + " of " + QString::number(ids.length()) + " Effects.");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Color)) {
                    kernel->terminal->error("Can't set Effect Color Steps because no Color was given.");
                    return;
                }
                value.removeFirst();
            }
            Color* color = kernel->colors->getItem(kernel->terminal->keysToId(value));
            if (!text.isEmpty()) {
                color = kernel->colors->getItem(text);
            }
            if (color == nullptr) {
                kernel->terminal->error("Can't set Effect Color Steps because Color " + kernel->terminal->keysToId(value) + " doesn't exist.");
                return;
            }
            int effectCounter = 0;
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                if (step > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Color Step " + QString::number(step) + " of Effect " + id + " because this Effect only has " + QString::number(effect->intAttributes.value(STEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->colorSteps[step] = color;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Color Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to Color " + color->name() + ".");
        }
    } else if (attribute.startsWith(RAWSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(RAWSTEPSATTRIBUTEID + ".").length());
        bool ok;
        int step = attribute.toInt(&ok);
        if (!ok) {
            kernel->terminal->error("Can't set Effect Raw Step because no valid Attribute was given.");
            return;
        }
        if (step < 1) {
            kernel->terminal->error("Can't set Effect Raw Step because Step has to be at least 1.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                effect->rawSteps.remove(step);
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Raw Step " + QString::number(step) + " of Effect " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed Raw Step " + QString::number(step) + " of " + QString::number(ids.length()) + " Effects.");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Raw)) {
                    kernel->terminal->error("Can't set Effect Raw Steps because no Raw was given.");
                    return;
                }
                value.removeFirst();
            }
            QStringList rawIds = kernel->terminal->keysToSelection(value, Keys::Raw);
            if (!text.isEmpty()) {
                rawIds = text.split("+");
            }
            QList<Raw*> raws;
            for (QString rawId : rawIds) {
                Raw* raw = kernel->raws->getItem(rawId);
                if (raw == nullptr) {
                    kernel->terminal->warning("Can't add Raw " + rawId + " to Effect Step because it doesn't exist.");
                } else {
                    raws.append(raw);
                }
            }
            int effectCounter = 0;
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                if (step > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Raw Step " + QString::number(step) + " of Effect " + id + " because this Effect only has " + QString::number(effect->intAttributes.value(STEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->rawSteps[step] = raws;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Raw Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to " + QString::number(raws.length()) + " Raws.");
        }
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
        for (Effect* effect : items) {
            for (int intensityStep : effect->intensitySteps.keys()) {
                if (intensityStep > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    effect->intensitySteps.remove(intensityStep);
                }
            }
            for (int colorStep : effect->colorSteps.keys()) {
                if (colorStep > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    effect->colorSteps.remove(colorStep);
                }
            }
            for (int rawStep : effect->rawSteps.keys()) {
                if (rawStep > effect->intAttributes.value(STEPSATTRIBUTEID)) {
                    effect->rawSteps.remove(rawStep);
                }
            }
        }
    }
}
