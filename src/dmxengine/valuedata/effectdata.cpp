/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effectdata.h"

QHash<int, QHash<int, int>> EffectData::groupEffectFrames;
QHash<int, QHash<int, int>> EffectData::oldGroupEffectFrames;

EffectData::EffectData(const int fixtureKey, const int groupKey, const QList<int> effectKeys, const int FRAMEDURATION) {
    for (const int effectKey : effectKeys) {
        if (!groupEffectFrames.contains(groupKey)) {
            groupEffectFrames[groupKey] = QHash<int, int>();
        }
        if (oldGroupEffectFrames.value(groupKey, QHash<int, int>()).contains(effectKey)) {
            groupEffectFrames[groupKey][effectKey] = oldGroupEffectFrames.value(groupKey).value(effectKey) + 1;
        } else {
            groupEffectFrames[groupKey][effectKey] = 1;
        }

        QSqlQuery effectAttributesQuery;
        effectAttributesQuery.prepare("SELECT steps, hold, fade, phase, sinefade FROM effects WHERE key = :effect");
        effectAttributesQuery.bindValue(":effect", effectKey);
        if (effectAttributesQuery.exec()) {
            if (effectAttributesQuery.next()) {
                const int stepAmount = effectAttributesQuery.value(0).toInt();
                const int standardHoldFrames = (effectAttributesQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                const int standardFadeFrames = (effectAttributesQuery.value(2).toFloat() * 1000 / FRAMEDURATION);
                float phase = effectAttributesQuery.value(3).toFloat();
                const bool sineFade = (effectAttributesQuery.value(4).toInt() == 1);

                QHash<int, int> stepHoldFrames;
                QSqlQuery stepHoldQuery;
                stepHoldQuery.prepare("SELECT key, value FROM effect_step_hold WHERE item_key = :effect");
                stepHoldQuery.bindValue(":effect", effectKey);
                if (stepHoldQuery.exec()) {
                    while (stepHoldQuery.next()) {
                        const int step = stepHoldQuery.value(0).toInt();
                        if (step <= stepAmount) {
                            stepHoldFrames[step] = (stepHoldQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                        }
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << stepHoldQuery.executedQuery() << stepHoldQuery.lastError().text();
                }

                QHash<int, int> stepFadeFrames;
                QSqlQuery stepFadeQuery;
                stepFadeQuery.prepare("SELECT key, value FROM effect_step_fade WHERE item_key = :effect");
                stepFadeQuery.bindValue(":effect", effectKey);
                if (stepFadeQuery.exec()) {
                    while (stepFadeQuery.next()) {
                        const int step = stepFadeQuery.value(0).toInt();
                        if (step <= stepAmount) {
                            stepFadeFrames[step] = (stepFadeQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                        }
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << stepFadeQuery.executedQuery() << stepFadeQuery.lastError().text();
                }

                int totalFrames = 0;
                for (int step = 1; step <= stepAmount; step++) {
                    totalFrames += stepHoldFrames.value(step, standardHoldFrames);
                    totalFrames += stepFadeFrames.value(step, standardFadeFrames);
                }
                if (totalFrames > 0) {
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

                    QHash<int, int> stepIntensityKeys;
                    QSqlQuery intensityStepQuery;
                    intensityStepQuery.prepare("SELECT key, valueitem_key FROM effect_step_intensities WHERE item_key = :effect");
                    intensityStepQuery.bindValue(":effect", effectKey);
                    if (intensityStepQuery.exec()) {
                        while (intensityStepQuery.next()) {
                            const int step = intensityStepQuery.value(0).toInt();
                            if (step <= stepAmount) {
                                stepIntensityKeys[step] = intensityStepQuery.value(1).toInt();
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << intensityStepQuery.executedQuery() << intensityStepQuery.lastError().text();
                    }

                    QHash<int, int> stepColorKeys;
                    QSqlQuery colorStepQuery;
                    colorStepQuery.prepare("SELECT key, valueitem_key FROM effect_step_colors WHERE item_key = :effect");
                    colorStepQuery.bindValue(":effect", effectKey);
                    if (colorStepQuery.exec()) {
                        while (colorStepQuery.next()) {
                            const int step = colorStepQuery.value(0).toInt();
                            if (step <= stepAmount) {
                                stepColorKeys[step] = colorStepQuery.value(1).toInt();
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << colorStepQuery.executedQuery() << colorStepQuery.lastError().text();
                    }

                    QHash<int, int> stepPositionKeys;
                    QSqlQuery positionStepQuery;
                    positionStepQuery.prepare("SELECT key, valueitem_key FROM effect_step_positions WHERE item_key = :effect");
                    positionStepQuery.bindValue(":effect", effectKey);
                    if (positionStepQuery.exec()) {
                        while (positionStepQuery.next()) {
                            const int step = positionStepQuery.value(0).toInt();
                            if (step <= stepAmount) {
                                stepPositionKeys[step] = positionStepQuery.value(1).toInt();
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << positionStepQuery.executedQuery() << positionStepQuery.lastError().text();
                    }

                    QHash<int, QList<int>> stepRawKeys;
                    QSqlQuery rawStepQuery;
                    rawStepQuery.prepare("SELECT effect_step_raws.key, effect_step_raws.valueitem_key FROM effect_step_raws, raws WHERE effect_step_raws.item_key = :effect AND effect_step_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
                    rawStepQuery.bindValue(":effect", effectKey);
                    if (rawStepQuery.exec()) {
                        while (rawStepQuery.next()) {
                            const int step = rawStepQuery.value(0).toInt();
                            if (step <= stepAmount) {
                                if (!stepRawKeys.contains(step)) {
                                    stepRawKeys[step] = QList<int>();
                                }
                                stepRawKeys[step].append(rawStepQuery.value(1).toInt());
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << rawStepQuery.executedQuery() << rawStepQuery.lastError().text();
                    }

                    int frames = oldGroupEffectFrames.value(groupKey, QHash<int, int>()).value(effectKey, 0);
                    frames = (int)(frames + (phase / 360) * totalFrames) % totalFrames;
                    int currentStep = 1;
                    float fade = 1;
                    for (int step = 1; step <= stepAmount; step++) {
                        if ((frames > 0) && (stepFadeFrames.value(step, standardFadeFrames) > 0)) {
                            currentStep = step;
                            fade = 1 - (float)frames / (float)stepFadeFrames.value(step, standardFadeFrames);
                        }
                        frames -= stepFadeFrames.value(step, standardFadeFrames);
                        if (frames > 0) {
                            currentStep = step;
                            fade = 0;
                        }
                        frames -= stepHoldFrames.value(step, standardHoldFrames);
                    }
                    int lastStep = currentStep - 1;
                    if (lastStep < 1) {
                        lastStep = stepAmount;
                    }
                    if (sineFade) {
                        fade = std::cos(M_PI * (1 - fade)) / 2 + 0.5;
                    }

                    if (!stepIntensityKeys.isEmpty()) {
                        IntensityData currentIntensity;
                        if (stepIntensityKeys.contains(currentStep)) {
                            currentIntensity = IntensityData(fixtureKey, stepIntensityKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            IntensityData lastIntensity;
                            if (stepIntensityKeys.contains(lastStep)) {
                                lastIntensity = IntensityData(fixtureKey, stepIntensityKeys.value(lastStep));
                            }
                            currentIntensity.fade(lastIntensity, fade);
                        }
                        intensity.merge(currentIntensity);
                    }
                    if (!stepColorKeys.isEmpty()) {
                        colorGiven = true;
                        ColorData currentColor;
                        if (stepColorKeys.contains(currentStep)) {
                            currentColor = ColorData(fixtureKey, stepColorKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            ColorData lastColor;
                            if (stepColorKeys.contains(lastStep)) {
                                lastColor = ColorData(fixtureKey, stepColorKeys.value(lastStep));
                            }
                            currentColor.fade(lastColor, fade);
                        }
                        color = currentColor;
                    }

                    if (!stepPositionKeys.isEmpty()) {
                        positionGiven = true;
                        PositionData currentPosition;
                        if (stepPositionKeys.contains(currentStep)) {
                            currentPosition = PositionData(fixtureKey, stepPositionKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            PositionData lastPosition;
                            if (stepPositionKeys.contains(lastStep)) {
                                lastPosition = PositionData(fixtureKey, stepPositionKeys.value(lastStep));
                            }
                            currentPosition.fade(lastPosition, fade);
                        }
                        position = currentPosition;
                    }

                    if (!stepRawKeys.isEmpty()) {
                        RawData currentRaws;
                        RawData lastRaws;
                        for (int step = 1; step <= stepAmount; step++) {
                            if (stepRawKeys.contains(step)) {
                                const RawData stepRaws = RawData(fixtureKey, stepRawKeys.value(step));
                                if (step == currentStep) {
                                    currentRaws = stepRaws;
                                }
                                if (step == lastStep) {
                                    lastRaws = stepRaws;
                                }
                            }
                        }
                        /*if (fade > 0) {
                            for (const int channel : currentRaws.keys()) {
                                if (currentRaws.value(channel).fading) {
                                    RawChannelData channelData = currentRaws.value(channel);
                                    channelData.value += (lastRaws.value(channel, RawChannelData()).value - currentRaws.value(channel, RawChannelData()).value) * fade;
                                    currentRaws[channel] = channelData;
                                }
                            }
                        }*/
                        raws.merge(currentRaws);
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << effectAttributesQuery.executedQuery() << "Effect with key " + QString::number(effectKey) + " should exist but wasn't found!";
            }
        } else {
            qWarning() << Q_FUNC_INFO << effectAttributesQuery.executedQuery() << effectAttributesQuery.lastError().text();
        }
    }
}

void EffectData::nextFrame() {
    oldGroupEffectFrames = groupEffectFrames;
    groupEffectFrames.clear();
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

RawData EffectData::getRaws() const {
    return raws;
}
