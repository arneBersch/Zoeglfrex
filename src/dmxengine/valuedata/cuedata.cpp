/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuedata.h"
#include "effectdata.h"

CueData::CueData() {}

CueData::CueData(const int cueKey, QList<int> groupKeys, QHash<int, QSet<int>> groupFixtureKeys, const bool renderMwD) {
    for (const int groupKey : groupKeys) {
        QList<int> rawKeys;

        if (!renderMwD) {
            const int intensityKey = getItemKey(cueKey, groupKey, "cue_group_intensities");
            if (intensityKey >= 0) {
                for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                    const IntensityData intensity = IntensityData(fixtureKey, intensityKey);
                    if (fixtureIntensities.contains(fixtureKey)) {
                        fixtureIntensities[fixtureKey].merge(intensity);
                    } else {
                        fixtureIntensities[fixtureKey] = intensity;
                    }
                }
                rawKeys.append(getItemRawKeys(intensityKey, "intensity_raws"));
            }
        }

        const int colorKey = getItemKey(cueKey, groupKey, "cue_group_colors");
        if (colorKey >= 0) {
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                fixtureColors[fixtureKey] = ColorData(fixtureKey, colorKey);
            }
            rawKeys.append(getItemRawKeys(colorKey, "color_raws"));
        }

        const int positionKey = getItemKey(cueKey, groupKey, "cue_group_positions");
        if (positionKey >= 0) {
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                fixturePositions[fixtureKey] = PositionData(fixtureKey, positionKey);
            }
            rawKeys.append(getItemRawKeys(positionKey, "position_raws"));
        }

        rawKeys.append(getItemKeys(cueKey, groupKey, "cue_group_raws", "raws"));
        if (!rawKeys.isEmpty()) {
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                const RawData raws = RawData(fixtureKey, rawKeys, renderMwD);
                if (fixtureRaws.contains(fixtureKey)) {
                    fixtureRaws[fixtureKey].merge(raws);
                } else {
                    fixtureRaws[fixtureKey] = raws;
                }
            }
        }

        const QList<int> effectKeys = getItemKeys(cueKey, groupKey, "cue_group_effects", "effects");
        if (!effectKeys.isEmpty()) {
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                EffectData effects = EffectData(fixtureKey, groupKey, effectKeys, renderMwD);
                if (fixtureIntensities.contains(fixtureKey)) {
                    fixtureIntensities[fixtureKey].merge(effects.getIntensity());
                } else {
                    fixtureIntensities[fixtureKey] = effects.getIntensity();
                }
                if (effects.hasColor()) {
                    fixtureColors[fixtureKey] = effects.getColor();
                }
                if (effects.hasPosition()) {
                    fixturePositions[fixtureKey] = effects.getPosition();
                }
                if (fixtureRaws.contains(fixtureKey)) {
                    fixtureRaws[fixtureKey].merge(effects.getRaws());
                } else {
                    fixtureRaws[fixtureKey] = effects.getRaws();
                }
            }
        }
    }
}

int CueData::getItemKey(const int cueKey, const int groupKey, const QString table) {
    QSqlQuery query;
    query.prepare("SELECT valueitem_key FROM " + table + " WHERE item_key = :cue AND foreignitem_key = :group");
    query.bindValue(":cue", cueKey);
    query.bindValue(":group", groupKey);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return -1;
}

QList<int> CueData::getItemKeys(const int cueKey, const int groupKey, const QString valueTable, const QString itemTable) {
    QList<int> itemKeys;
    QSqlQuery query;
    query.prepare("SELECT " + valueTable + ".valueitem_key FROM " + valueTable + ", " + itemTable + " WHERE " + valueTable + ".item_key = :cue AND " + valueTable + ".foreignitem_key = :group AND " + valueTable + ".valueitem_key = " + itemTable + ".key ORDER BY " + itemTable + ".sortkey");
    query.bindValue(":group", groupKey);
    query.bindValue(":cue", cueKey);
    if (query.exec()) {
        while (query.next()) {
            itemKeys.append(query.value(0).toInt());
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return itemKeys;
}

QList<int> CueData::getItemRawKeys(const int itemKey, const QString table) {
    QList<int> rawKeys;
    QSqlQuery query;
    query.prepare("SELECT " + table + ".valueitem_key FROM " + table + ", raws WHERE " + table + ".item_key = :item AND " + table + ".valueitem_key = raws.key ORDER BY raws.sortkey");
    query.bindValue(":item", itemKey);
    if (query.exec()) {
        while (query.next()) {
            rawKeys.append(query.value(0).toInt());
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return rawKeys;
}

void CueData::fade(const CueData lastCue, const QHash<int, float> fixtureFades) {
    for (const int fixtureKey : fixtureFades.keys()) {
        const float fade = fixtureFades.value(fixtureKey);
        if (fade > 0) {
            if (!fixtureIntensities.contains(fixtureKey)) {
                fixtureIntensities[fixtureKey] = IntensityData();
            }
            IntensityData lastIntensity = lastCue.fixtureIntensities.value(fixtureKey, IntensityData());
            fixtureIntensities[fixtureKey].fade(lastIntensity, fade);

            if (lastCue.fixtureColors.contains(fixtureKey)) {
                const ColorData lastColor = lastCue.fixtureColors.value(fixtureKey);
                if (fixtureColors.contains(fixtureKey)) {
                    fixtureColors[fixtureKey].fade(lastColor, fade);
                } else {
                    fixtureColors[fixtureKey] = lastColor;
                }
            }

            if (lastCue.fixturePositions.contains(fixtureKey)) {
                const PositionData lastPosition = lastCue.fixturePositions.value(fixtureKey);
                if (fixturePositions.contains(fixtureKey)) {
                    fixturePositions[fixtureKey].fade(lastPosition, fade);
                } else {
                    fixturePositions[fixtureKey] = lastPosition;
                }
            }

            if (lastCue.fixtureRaws.contains(fixtureKey)) {
                const RawData lastRaws = lastCue.fixtureRaws.value(fixtureKey);
                if (fixtureRaws.contains(fixtureKey)) {
                    fixtureRaws[fixtureKey].fade(lastRaws, fade);
                } else {
                    fixtureRaws[fixtureKey] = lastRaws;
                }
            }
        }
    }
}

void CueData::merge(const CueData cue) {
    for (const int fixtureKey : cue.fixtureIntensities.keys()) {
        if (fixtureIntensities.contains(fixtureKey)) {
            fixtureIntensities[fixtureKey].merge(cue.fixtureIntensities.value(fixtureKey));
        } else {
            fixtureIntensities[fixtureKey] = cue.fixtureIntensities.value(fixtureKey);
        }
    }

    for (const int fixtureKey : cue.fixtureColors.keys()) {
        fixtureColors[fixtureKey] = cue.fixtureColors.value(fixtureKey);
    }

    for (const int fixtureKey : cue.fixturePositions.keys()) {
        fixturePositions[fixtureKey] = cue.fixturePositions.value(fixtureKey);
    }

    for (const int fixtureKey : cue.fixtureRaws.keys()) {
        if (fixtureRaws.contains(fixtureKey)) {
            fixtureRaws[fixtureKey].merge(cue.fixtureRaws.value(fixtureKey));
        } else {
            fixtureRaws[fixtureKey] = cue.fixtureRaws.value(fixtureKey);
        }
    }
}

IntensityData CueData::getFixtureIntensity(const int fixtureKey) const {
    return fixtureIntensities.value(fixtureKey, IntensityData());
}

ColorData CueData::getFixtureColor(const int fixtureKey) const {
    return fixtureColors.value(fixtureKey, ColorData());
}

PositionData CueData::getFixturePosition(const int fixtureKey) const {
    return fixturePositions.value(fixtureKey, PositionData());
}

RawData CueData::getFixtureRaws(const int fixtureKey) const {
    return fixtureRaws.value(fixtureKey, RawData());
}