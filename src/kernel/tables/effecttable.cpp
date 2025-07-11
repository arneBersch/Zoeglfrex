/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effecttable.h"

EffectTable::EffectTable(Kernel *core) : ItemTable(core, Keys::Effect, "Effect", "Effects") {
    intAttributes[kernel->EFFECTSTEPSATTRIBUTEID] = {"Steps", 2, 2, 99};
    floatAttributes[kernel->EFFECTSTEPHOLDATTRIBUTEID] = {"Step Hold", 1, 0, 600, "s"};
    stepSpecificFloatAttributes[kernel->EFFECTSTEPHOLDATTRIBUTEID] = {"Step Hold", 1, 0, 600, "s"};
    floatAttributes[kernel->EFFECTSTEPFADEATTRIBUTEID] = {"Step Fade", 0, 0, 600, "s"};
    stepSpecificFloatAttributes[kernel->EFFECTSTEPFADEATTRIBUTEID] = {"Step Fade", 0, 0, 600, "s"};
    angleAttributes[kernel->EFFECTPHASEATTRIBUTEID] = {"Phase", 0};
    fixtureSpecificAngleAttributes[kernel->EFFECTPHASEATTRIBUTEID] = {"Phase", 0};
}

void EffectTable::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute.startsWith(kernel->EFFECTINTENSITYSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(kernel->EFFECTINTENSITYSTEPSATTRIBUTEID + ".").length());
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
                if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Intensity Step " + QString::number(step) + " of Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->intensitySteps[step] = intensity;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Intensity Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to Intensity " + intensity->name() + ".");
        }
    } else if (attribute.startsWith(kernel->EFFECTCOLORSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(kernel->EFFECTCOLORSTEPSATTRIBUTEID + ".").length());
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
                if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Color Step " + QString::number(step) + " of Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->colorSteps[step] = color;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Color Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to Color " + color->name() + ".");
        }
    } else if (attribute.startsWith(kernel->EFFECTPOSITIONSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(kernel->EFFECTPOSITIONSTEPSATTRIBUTEID + ".").length());
        bool ok;
        int step = attribute.toInt(&ok);
        if (!ok) {
            kernel->terminal->error("Can't set Effect Position Step because no valid Attribute was given.");
            return;
        }
        if (step < 1) {
            kernel->terminal->error("Can't set Effect Position Step because Step has to be at least 1.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                effect->positionSteps.remove(step);
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Position Step " + QString::number(step) + " of Effect " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed Position Step " + QString::number(step) + " of " + QString::number(ids.length()) + " Effects.");
            }
        } else {
            if (text.isEmpty()) {
                if (value.isEmpty() || (value.first() != Keys::Position)) {
                    kernel->terminal->error("Can't set Effect Position Steps because no Position was given.");
                    return;
                }
                value.removeFirst();
            }
            Position* position = kernel->positions->getItem(kernel->terminal->keysToId(value));
            if (!text.isEmpty()) {
                position = kernel->positions->getItem(text);
            }
            if (position == nullptr) {
                kernel->terminal->error("Can't set Effect Position Steps because Position " + kernel->terminal->keysToId(value) + " doesn't exist.");
                return;
            }
            int effectCounter = 0;
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Position Step " + QString::number(step) + " of Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->positionSteps[step] = position;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Position Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to Position " + position->name() + ".");
        }
    } else if (attribute.startsWith(kernel->EFFECTRAWSTEPSATTRIBUTEID + ".")) {
        attribute.remove(0, QString(kernel->EFFECTRAWSTEPSATTRIBUTEID + ".").length());
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
            if (raws.isEmpty()) {
                kernel->terminal->error("Can't set Effect Raw Set because an invalid Raw selection was given.");
                return;
            }
            int effectCounter = 0;
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    kernel->terminal->warning("Can't set Raw Step " + QString::number(step) + " of Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                } else {
                    effect->rawSteps[step] = raws;
                    effectCounter++;
                }
            }
            kernel->terminal->success("Set Raw Step " + QString::number(step) + " of " + QString::number(effectCounter) + " Effects to " + QString::number(raws.length()) + " Raws.");
        }
    } else if ((attribute.split(".").length() == 2) && stepSpecificFloatAttributes.contains(attribute.split(".").first())) {
        FloatAttribute floatAttribute = stepSpecificFloatAttributes.value(attribute.split(".").first());
        bool ok;
        int step = attribute.split(".").last().toInt(&ok);
        attribute = attribute.split(".").first();
        if (!ok) {
            kernel->terminal->error("Can't set Effect " + floatAttribute.name + " because no valid Step was given.");
            return;
        }
        if (step < 1) {
            kernel->terminal->error("Can't set Effect " + floatAttribute.name + " because Step has to be at least 1.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Effect *effect = getItem(id);
                if (effect == nullptr) {
                    effect = addItem(id);
                }
                effect->stepSpecificFloatAttributes[attribute].remove(step);
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Step " + QString::number(step) + " in Effect " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Step " + QString::number(step) + " in " + QString::number(ids.length()) + " Effects.");
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->terminal->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set Effect " + floatAttribute.name + " of Step " + QString::number(step) + " because no valid value was given.");
                    return;
                }
                if (negativeValue) {
                    newValue *= -1;
                }
            } else {
                bool ok = true;
                newValue = text.toFloat(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set Effect " + floatAttribute.name + " of Step " + QString::number(step) + " because no valid value was given.");
                    return;
                }
            }
            if (difference) {
                for (QString id : ids) {
                    Effect* effect = getItem(id);
                    if (effect == nullptr) {
                        effect = addItem(id);
                    }
                    if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                        kernel->terminal->warning("Can't set " + floatAttribute.name + " of Step " + QString::number(step) + " of Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                    } else {
                        if (!effect->stepSpecificFloatAttributes.value(attribute).contains(step)) {
                            effect->stepSpecificFloatAttributes[attribute][step] = effect->floatAttributes.value(attribute);
                        }
                        effect->stepSpecificFloatAttributes[attribute][step] += newValue;
                        if (effect->stepSpecificFloatAttributes.value(attribute).value(step) < floatAttribute.min) {
                            effect->stepSpecificFloatAttributes[attribute][step] = floatAttribute.min;
                            kernel->terminal->warning("Can't decrease " + floatAttribute.name + " of Step " + QString::number(step) + " in Effect " + effect->name() + " because value must be at least " + QString::number(floatAttribute.min) + floatAttribute.unit + ".");
                        } else if (effect->stepSpecificFloatAttributes.value(attribute).value(step) > floatAttribute.max) {
                            effect->stepSpecificFloatAttributes[attribute][step] = floatAttribute.max;
                            kernel->terminal->warning("Can't increase " + floatAttribute.name + " of Step " + QString::number(step) + " in Effect " + effect->name() + " because value must not exceed " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                        }
                        emit dataChanged(index(getItemRow(effect->id), 0), index(getItemRow(effect->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    }
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of Effect " + getItem(ids.first())->name() + " at Step " + QString::number(step) + + " to " + QString::number(getItem(ids.first())->stepSpecificFloatAttributes.value(attribute).value(step)) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Changed " + floatAttribute.name + " of Step " + QString::number(step) + "at " + QString::number(ids.length()) + " Effects.");
                }
            } else {
                if ((newValue < floatAttribute.min) || (newValue > floatAttribute.max)) {
                    kernel->terminal->error("Can't set Effect " + floatAttribute.name + " of Step " + QString::number(step) + " because " + floatAttribute.name + " has to be a number between " + QString::number(floatAttribute.min) + floatAttribute.unit + " and " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                    return;
                }
                for (QString id : ids) {
                    Effect* effect = getItem(id);
                    if (effect == nullptr) {
                        effect = addItem(id);
                    }
                    if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                        kernel->terminal->warning("Can't set " + floatAttribute.name + " of Step " + QString::number(step) + " in Effect " + effect->name() + " because this Effect only has " + QString::number(effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) + " Steps.");
                    } else {
                        effect->stepSpecificFloatAttributes[attribute][step] = newValue;
                        emit dataChanged(index(getItemRow(effect->id), 0), index(getItemRow(effect->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    }
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of Step " + QString::number(step) + " in Effect " + getItem(ids.first())->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Set " + floatAttribute.name + " of Step " + QString::number(step) + " in " + QString::number(ids.length()) + " Effects to " + QString::number(newValue) + floatAttribute.unit + ".");
                }
            }
        }
    } else {
        ItemTable::setAttribute(ids, attributes, value, text);
        for (Effect* effect : items) {
            for (int intensityStep : effect->intensitySteps.keys()) {
                if (intensityStep > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    effect->intensitySteps.remove(intensityStep);
                }
            }
            for (int colorStep : effect->colorSteps.keys()) {
                if (colorStep > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    effect->colorSteps.remove(colorStep);
                }
            }
            for (int positionStep : effect->positionSteps.keys()) {
                if (positionStep > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    effect->positionSteps.remove(positionStep);
                }
            }
            for (int rawStep : effect->rawSteps.keys()) {
                if (rawStep > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                    effect->rawSteps.remove(rawStep);
                }
            }
            for (QString attribute : stepSpecificFloatAttributes.keys()) {
                for (int step : effect->stepSpecificFloatAttributes.value(attribute).keys()) {
                    if (step > effect->intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID)) {
                        effect->stepSpecificFloatAttributes[attribute].remove(step);
                    }
                }
            }
        }
    }
}

Effect* EffectTable::addItem(QString id) {
    Effect* effect = ItemTable<Effect>::addItem(id);
    for (QString attribute : stepSpecificFloatAttributes.keys()) {
        effect->stepSpecificFloatAttributes[attribute] = QMap<int, float>();
    }
    return effect;
}
