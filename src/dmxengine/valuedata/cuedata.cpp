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

CueData::CueData(int cueKey, QList<int> groupKeys, QHash<int, QSet<int>> groupFixtureKeys, int FRAMEDURATION) {
    for (const int groupKey : groupKeys) {
        QList<int> rawKeys;
        QSqlQuery intensityQuery;
        intensityQuery.prepare("SELECT valueitem_key FROM cue_group_intensities WHERE item_key = :cue AND foreignitem_key = :group");
        intensityQuery.bindValue(":group", groupKey);
        intensityQuery.bindValue(":cue", cueKey);
        if (intensityQuery.exec()) {
            while (intensityQuery.next()) {
                const int intensityKey = intensityQuery.value(0).toInt();
                for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                    const IntensityData intensity = IntensityData(fixtureKey, intensityKey);
                    if (fixtureIntensities.contains(fixtureKey)) {
                        fixtureIntensities[fixtureKey].merge(intensity);
                    } else {
                        fixtureIntensities[fixtureKey] = intensity;
                    }
                }
                QSqlQuery rawsQuery;
                rawsQuery.prepare("SELECT intensity_raws.valueitem_key FROM intensity_raws, raws WHERE intensity_raws.item_key = :intensity AND intensity_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
                rawsQuery.bindValue(":intensity", intensityKey);
                if (rawsQuery.exec()) {
                    while (rawsQuery.next()) {
                        rawKeys.append(rawsQuery.value(0).toInt());
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << rawsQuery.executedQuery() << rawsQuery.lastError().text();
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << intensityQuery.executedQuery() << intensityQuery.lastError().text();
        }

        QSqlQuery colorQuery;
        colorQuery.prepare("SELECT valueitem_key FROM cue_group_colors WHERE item_key = :cue AND foreignitem_key = :group");
        colorQuery.bindValue(":group", groupKey);
        colorQuery.bindValue(":cue", cueKey);
        if (colorQuery.exec()) {
            while (colorQuery.next()) {
                const int colorKey = colorQuery.value(0).toInt();
                for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                    fixtureColors[fixtureKey] = ColorData(fixtureKey, colorKey);
                }
                QSqlQuery rawsQuery;
                rawsQuery.prepare("SELECT color_raws.valueitem_key FROM color_raws, raws WHERE color_raws.item_key = :color AND color_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
                rawsQuery.bindValue(":color", colorKey);
                if (rawsQuery.exec()) {
                    while (rawsQuery.next()) {
                        rawKeys.append(rawsQuery.value(0).toInt());
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << rawsQuery.executedQuery() << rawsQuery.lastError().text();
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << colorQuery.executedQuery() << colorQuery.lastError().text();
        }

        QSqlQuery positionQuery;
        positionQuery.prepare("SELECT valueitem_key FROM cue_group_positions WHERE item_key = :cue AND foreignitem_key = :group");
        positionQuery.bindValue(":group", groupKey);
        positionQuery.bindValue(":cue", cueKey);
        if (positionQuery.exec()) {
            while (positionQuery.next()) {
                const int positionKey = positionQuery.value(0).toInt();
                for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                    fixturePositions[fixtureKey] = PositionData(fixtureKey, positionKey);
                }
                QSqlQuery rawsQuery;
                rawsQuery.prepare("SELECT position_raws.valueitem_key FROM position_raws, raws WHERE position_raws.item_key = :position AND position_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
                rawsQuery.bindValue(":position", positionKey);
                if (rawsQuery.exec()) {
                    while (rawsQuery.next()) {
                        rawKeys.append(rawsQuery.value(0).toInt());
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << rawsQuery.executedQuery() << rawsQuery.lastError().text();
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << positionQuery.executedQuery() << positionQuery.lastError().text();
        }

        QSqlQuery rawQuery;
        rawQuery.prepare("SELECT cue_group_raws.valueitem_key FROM cue_group_raws, raws WHERE cue_group_raws.item_key = :cue AND cue_group_raws.foreignitem_key = :group AND cue_group_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
        rawQuery.bindValue(":group", groupKey);
        rawQuery.bindValue(":cue", cueKey);
        if (rawQuery.exec()) {
            while (rawQuery.next()) {
                rawKeys.append(rawQuery.value(0).toInt());
            }
        } else {
            qWarning() << Q_FUNC_INFO << rawQuery.executedQuery() << rawQuery.lastError().text();
        }
        if (!rawKeys.isEmpty()) {
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                const RawData raws = RawData(fixtureKey, rawKeys);
                if (fixtureRaws.contains(fixtureKey)) {
                    fixtureRaws[fixtureKey].merge(raws);
                } else {
                    fixtureRaws[fixtureKey] = raws;
                }
            }
        }

        QSqlQuery effectQuery;
        effectQuery.prepare("SELECT cue_group_effects.valueitem_key FROM cue_group_effects, effects WHERE cue_group_effects.item_key = :cue AND cue_group_effects.foreignitem_key = :group AND cue_group_effects.valueitem_key = effects.key ORDER BY effects.sortkey");
        effectQuery.bindValue(":group", groupKey);
        effectQuery.bindValue(":cue", cueKey);
        if (effectQuery.exec()) {
            QList<int> effectKeys;
            while (effectQuery.next()) {
                const int effectKey = effectQuery.value(0).toInt();
                effectKeys.append(effectKey);
            }
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                EffectData effects = EffectData(fixtureKey, groupKey, effectKeys, FRAMEDURATION);
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
        } else {
            qWarning() << Q_FUNC_INFO << effectQuery.executedQuery() << effectQuery.lastError().text();
        }
    }
}

QHash<int, IntensityData> CueData::getFixtureIntensities() const {
    return fixtureIntensities;
}

QHash<int, ColorData> CueData::getFixtureColors() const {
    return fixtureColors;
}

QHash<int, PositionData> CueData::getFixturePositions() const {
    return fixturePositions;
}

QHash<int, RawData> CueData::getFixtureRaws() const {
    return fixtureRaws;
}
