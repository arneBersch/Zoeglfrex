/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effectdata.h"
#include "../dmxengine.h"

QHash<int, QHash<int, int>> EffectData::groupFrames;
QHash<int, QHash<int, int>> EffectData::oldGroupFrames;

EffectData::EffectData(const int fixtureKey, const int groupKey, const QList<int> effectKeys, const bool renderMwD) {
    for (const int effectKey : effectKeys) {
        if (!renderMwD) {
            if (!groupFrames.contains(groupKey)) {
                groupFrames[groupKey] = QHash<int, int>();
            }
            if (oldGroupFrames.value(groupKey, QHash<int, int>()).contains(effectKey)) {
                groupFrames[groupKey][effectKey] = oldGroupFrames.value(groupKey).value(effectKey) + 1;
            } else {
                groupFrames[groupKey][effectKey] = 1;
            }
        }

        QSqlQuery effectAttributesQuery;
        effectAttributesQuery.prepare("SELECT steps, hold, fade, phase, sinefade FROM effects WHERE key = :effect");
        effectAttributesQuery.bindValue(":effect", effectKey);
        if (effectAttributesQuery.exec()) {
            if (effectAttributesQuery.next()) {
                const int stepAmount = effectAttributesQuery.value(0).toInt();
                const int standardHoldFrames = effectAttributesQuery.value(1).toFloat() * 1000 / FRAMEDURATION;
                const int standardFadeFrames = effectAttributesQuery.value(2).toFloat() * 1000 / FRAMEDURATION;
                float phase = effectAttributesQuery.value(3).toFloat();
                const bool sineFade = effectAttributesQuery.value(4).toInt() == 1;

                QHash<int, int> stepHoldFrames = getStepTimeFrames(effectKey, stepAmount, "effect_step_hold");
                QHash<int, int> stepFadeFrames = getStepTimeFrames(effectKey, stepAmount, "effect_step_fade");

                int totalFrames = 0;
                for (int step = 1; step <= stepAmount; step++) {
                    totalFrames += stepHoldFrames.value(step, standardHoldFrames);
                    totalFrames += stepFadeFrames.value(step, standardFadeFrames);
                }
                if (totalFrames <= 0) {
                    return;
                }

                QSqlQuery fixturePhaseQuery;
                fixturePhaseQuery.prepare("SELECT value FROM effect_fixture_phase WHERE item_key = :effect AND foreignitem_key = :fixture");
                fixturePhaseQuery.bindValue(":effect", effectKey);
                fixturePhaseQuery.bindValue(":fixture", fixtureKey);
                if (fixturePhaseQuery.exec()) {
                    if (fixturePhaseQuery.next()) {
                        phase = fixturePhaseQuery.value(0).toFloat();
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << fixturePhaseQuery.executedQuery() << fixturePhaseQuery.lastError().text();
                }

                int frames = groupFrames.value(groupKey, QHash<int, int>()).value(effectKey, 1);
                frames = (int)(frames + (phase / 360) * totalFrames) % totalFrames;
                int currentStep = 1;
                float fade = 1;
                for (int step = 1; step <= stepAmount; step++) {
                    const int fadeFrames = stepFadeFrames.value(step, standardFadeFrames);
                    if ((frames > 0) && (fadeFrames > 0)) {
                        currentStep = step;
                        fade = 1 - (float)frames / (float)fadeFrames;
                    }
                    frames -= fadeFrames;
                    if (frames > 0) {
                        currentStep = step;
                        fade = 0;
                    }
                    frames -= stepHoldFrames.value(step, standardHoldFrames);
                }
                if (sineFade) {
                    fade = 0.5 - (std::cos(M_PI * fade) / 2);
                }

                int lastStep = currentStep - 1;
                if (lastStep < 1) {
                    lastStep = stepAmount;
                }

                if (!renderMwD) {
                    const int currentIntensityKey = getStepKey(currentStep, effectKey, "effect_step_intensities");
                    IntensityData currentIntensity;
                    if (currentIntensityKey >= 0) {
                        currentIntensity = IntensityData(fixtureKey, currentIntensityKey);
                    }
                    int lastIntensityKey = -1;
                    if (fade > 0) {
                        IntensityData lastIntensity;
                        lastIntensityKey = getStepKey(lastStep, effectKey, "effect_step_intensities");
                        if (lastIntensityKey >= 0) {
                            lastIntensity = IntensityData(fixtureKey, lastIntensityKey);
                        }
                        currentIntensity.fade(lastIntensity, fade);
                    }
                    if ((currentIntensityKey >= 0) || (lastIntensityKey >= 0)) {
                        intensityGiven = true;
                        intensity.merge(currentIntensity);
                    }
                }

                const int currentColorKey = getStepKey(currentStep, effectKey, "effect_step_colors");
                ColorData currentColor;
                if (currentColorKey >= 0) {
                    currentColor = ColorData(fixtureKey, currentColorKey);
                }
                int lastColorKey = -1;
                if (fade > 0) {
                    lastColorKey = getStepKey(lastStep, effectKey, "effect_step_colors");
                    if (lastColorKey >= 0) {
                        ColorData lastColor = ColorData(fixtureKey, lastColorKey);
                        if (currentColorKey >= 0) {
                            currentColor.fade(lastColor, fade);
                        } else {
                            currentColor = lastColor;
                        }
                    }
                }
                if ((currentColorKey >= 0) || (lastColorKey >= 0)) {
                    colorGiven = true;
                    color = currentColor;
                }

                const int currentPositionKey = getStepKey(currentStep, effectKey, "effect_step_positions");
                PositionData currentPosition;
                if (currentPositionKey >= 0) {
                    currentPosition = PositionData(fixtureKey, currentPositionKey);
                }
                int lastPositionKey = -1;
                if (fade > 0) {
                    lastPositionKey = getStepKey(lastStep, effectKey, "effect_step_positions");
                    if (lastPositionKey >= 0) {
                        PositionData lastPosition = PositionData(fixtureKey, lastPositionKey);
                        if (currentPositionKey >= 0) {
                            currentPosition.fade(lastPosition, fade);
                        } else {
                            currentPosition = lastPosition;
                        }
                    }
                }
                if ((currentPositionKey >= 0) || (lastPositionKey >= 0)) {
                    positionGiven = true;
                    position = currentPosition;
                }

                QList<int> currentRawKeys = getStepRawKeys(currentStep, effectKey);
                RawData currentRaws;
                if (!currentRawKeys.isEmpty()) {
                    currentRaws = RawData(fixtureKey, currentRawKeys, renderMwD);
                }
                QList<int> lastRawKeys;
                if (fade > 0) {
                    lastRawKeys = getStepRawKeys(lastStep, effectKey);
                    if (!lastRawKeys.isEmpty()) {
                        RawData lastRaws = RawData(fixtureKey, lastRawKeys, renderMwD);
                        currentRaws.fade(lastRaws, fade);
                    }
                }
                if (!currentRawKeys.isEmpty() || !lastRawKeys.isEmpty()) {
                    rawsGiven = true;
                    raws.merge(currentRaws);
                }
            } else {
                qWarning() << Q_FUNC_INFO << effectAttributesQuery.executedQuery() << "Effect with key " + QString::number(effectKey) + " should exist but wasn't found!";
            }
        } else {
            qWarning() << Q_FUNC_INFO << effectAttributesQuery.executedQuery() << effectAttributesQuery.lastError().text();
        }
    }
}

int EffectData::getStepKey(const int step, const int effectKey, const QString table) {
    QSqlQuery query;
    query.prepare("SELECT valueitem_key FROM " + table + " WHERE item_key = :effect AND key = :step");
    query.bindValue(":effect", effectKey);
    query.bindValue(":step", step);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return -1;
}

QList<int> EffectData::getStepRawKeys(const int step, const int effectKey) {
    QList<int> rawKeys;
    QSqlQuery query;
    query.prepare("SELECT effect_step_raws.valueitem_key FROM effect_step_raws, raws WHERE effect_step_raws.item_key = :effect AND effect_step_raws.key = :step AND effect_step_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
    query.bindValue(":effect", effectKey);
    query.bindValue(":step", step);
    if (query.exec()) {
        while (query.next()) {
            rawKeys.append(query.value(0).toInt());
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return rawKeys;
}

QHash<int, int> EffectData::getStepTimeFrames(const int effectKey, const int stepAmount, const QString table) {
    QHash<int, int> frames;
    QSqlQuery query;
    query.prepare("SELECT key, value FROM " + table + " WHERE item_key = :effect");
    query.bindValue(":effect", effectKey);
    if (query.exec()) {
        while (query.next()) {
            const int step = query.value(0).toInt();
            if (step <= stepAmount) {
                frames[step] = query.value(1).toFloat() * 1000 / FRAMEDURATION;
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return frames;
}

void EffectData::nextFrame() {
    oldGroupFrames = groupFrames;
    groupFrames.clear();
}

bool EffectData::hasIntensity() const {
    return intensityGiven;
}

IntensityData EffectData::getIntensity() const {
    return intensity;
}

bool EffectData::hasColor() const {
    return colorGiven;
}

ColorData EffectData::getColor() const {
    return color;
}

bool EffectData::hasPosition() const {
    return positionGiven;
}

PositionData EffectData::getPosition() const {
    return position;
}

bool EffectData::hasRaws() const {
    return rawsGiven;
}

RawData EffectData::getRaws() const {
    return raws;
}
