/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"

DmxEngine::DmxEngine(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout();
    setLayout(layout);

    fadeProgressBar = new QProgressBar();
    fadeProgressBar->setRange(0, 10);
    fadeProgressBar->setValue(5);
    layout->addWidget(fadeProgressBar);

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &DmxEngine::generateDmx);
    timer->start(FRAMEDURATION);
}

void DmxEngine::generateDmx() {
    int currentCuelistKey = -1;
    QSqlQuery currentCuelistQuery;
    if (currentCuelistQuery.exec("SELECT cuelist_key FROM currentitems")) {
        if (currentCuelistQuery.next()) {
            currentCuelistKey = currentCuelistQuery.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << currentCuelistQuery.executedQuery() << currentCuelistQuery.lastError().text();
    }
    QSqlQuery cuelistQuery;
    if (!cuelistQuery.exec("SELECT key, currentcue_key, lastcue_key, priority FROM cuelists WHERE currentcue_key IS NOT NULL ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QMap<int, int> oldCuelistCurrentCueKeys = cuelistCurrentCueKeys;
    cuelistCurrentCueKeys.clear();
    QMap<int, int> oldCuelistRemainingFadeFrames = cuelistRemainingFadeFrames;
    cuelistRemainingFadeFrames.clear();
    QMap<int, int> oldCuelistTotalFadeFrames = cuelistTotalFadeFrames;
    cuelistTotalFadeFrames.clear();
    QMap<int, float> fixtureIntensities;
    QMap<int, ColorData> fixtureColors;
    QMap<int, int> fixtureColorPriorities;
    QMap<int, PositionData> fixturePositions;
    QMap<int, int> fixturePositionPriorities;
    QMap<int, QMap<int, uchar>> fixtureChannelRaws;
    QMap<int, QMap<int, int>> fixtureChannelRawPriorities;
    while (cuelistQuery.next()) {
        const int cuelistKey = cuelistQuery.value(0).toInt();
        const int currentCueKey = cuelistQuery.value(1).toInt();
        const int lastCueKey = cuelistQuery.value(2).toInt();
        const int priority = cuelistQuery.value(3).toInt();
        if (oldCuelistCurrentCueKeys.value(cuelistKey, -1) != currentCueKey) {
            QSqlQuery fadeQuery;
            fadeQuery.prepare("SELECT fade FROM cues WHERE key = :key");
            fadeQuery.bindValue(":key", currentCueKey);
            if (fadeQuery.exec()) {
                if (fadeQuery.next()) {
                    const int fadeFrames = (fadeQuery.value(0).toFloat() * 1000 / FRAMEDURATION);
                    if (fadeFrames > 0) {
                        cuelistRemainingFadeFrames[cuelistKey] = fadeFrames;
                        cuelistTotalFadeFrames[cuelistKey] = fadeFrames;
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << fadeQuery.executedQuery() << fadeQuery.lastError().text();
            }
        } else if (oldCuelistRemainingFadeFrames.value(cuelistKey, 0) > 0) {
            cuelistRemainingFadeFrames[cuelistKey] = (oldCuelistRemainingFadeFrames.value(cuelistKey, 0) - 1);
            cuelistTotalFadeFrames[cuelistKey] = oldCuelistTotalFadeFrames.value(cuelistKey);
        }
        cuelistCurrentCueKeys[cuelistKey] = currentCueKey;
        const float fade = (float)cuelistRemainingFadeFrames.value(cuelistKey, 0) / (float)cuelistTotalFadeFrames.value(cuelistKey, 1);
        const QMap<int, int> currentCueFixtureIntensityKeys = getCurrentCueItems(currentCueKey, "cue_group_intensities");
        const QMap<int, int> currentCueFixtureColorKeys = getCurrentCueItems(currentCueKey, "cue_group_colors");
        const QMap<int, int> currentCueFixturePositionKeys = getCurrentCueItems(currentCueKey, "cue_group_positions");
        const QMap<int, QList<int>> currentCueFixtureRawKeys = getCurrentCueItemList(currentCueKey, "cue_group_raws");
        const QMap<int, QList<int>> currentCueFixtureEffectKeys = getCurrentCueItemList(currentCueKey, "cue_group_effects");
        if (fade > 0) {
            const QMap<int, int> lastCueFixtureIntensityKeys = getCurrentCueItems(lastCueKey, "cue_group_intensities");
            QSet<int> intensityFixtureKeys;
            for (const int fixtureKey : currentCueFixtureIntensityKeys.keys()) {
                intensityFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : lastCueFixtureIntensityKeys.keys()) {
                intensityFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : intensityFixtureKeys) {
                float lastIntensity = 0;
                if (lastCueFixtureIntensityKeys.contains(fixtureKey)) {
                    lastIntensity = getFixtureIntensity(fixtureKey, lastCueFixtureIntensityKeys.value(fixtureKey));
                }
                float currentIntensity = 0;
                if (currentCueFixtureIntensityKeys.contains(fixtureKey)) {
                    currentIntensity = getFixtureIntensity(fixtureKey, currentCueFixtureIntensityKeys.value(fixtureKey));
                }
                currentIntensity += (lastIntensity - currentIntensity) * fade;
                if (currentIntensity > fixtureIntensities.value(fixtureKey, 0)) {
                    fixtureIntensities[fixtureKey] = currentIntensity;
                }
            }
            const QMap<int, int> lastCueFixtureColorKeys = getCurrentCueItems(lastCueKey, "cue_group_colors");
            QSet<int> colorFixtureKeys;
            for (const int fixtureKey : currentCueFixtureColorKeys.keys()) {
                colorFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : lastCueFixtureColorKeys.keys()) {
                colorFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : colorFixtureKeys) {
                if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                    fixtureColorPriorities[fixtureKey] = priority;
                    if (currentCueFixtureColorKeys.contains(fixtureKey)) {
                        ColorData currentColor = getFixtureColor(fixtureKey, currentCueFixtureColorKeys.value(fixtureKey));
                        if (lastCueFixtureColorKeys.contains(fixtureKey)) {
                            ColorData lastColor = getFixtureColor(fixtureKey, lastCueFixtureColorKeys.value(fixtureKey));
                            currentColor.red += (lastColor.red - currentColor.red) * fade;
                            currentColor.green += (lastColor.green - currentColor.green) * fade;
                            currentColor.blue += (lastColor.blue - currentColor.blue) * fade;
                            currentColor.quality += (lastColor.quality - currentColor.quality) * fade;
                        }
                        fixtureColors[fixtureKey] = currentColor;
                    } else if (lastCueFixtureColorKeys.contains(fixtureKey)) {
                        fixtureColors[fixtureKey] = getFixtureColor(fixtureKey, lastCueFixtureColorKeys.value(fixtureKey));
                    }
                }
            }
            const QMap<int, int> lastCueFixturePositionKeys = getCurrentCueItems(lastCueKey, "cue_group_positions");
            QSet<int> positionFixtureKeys;
            for (const int fixtureKey : currentCueFixturePositionKeys.keys()) {
                positionFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : lastCueFixturePositionKeys.keys()) {
                positionFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : positionFixtureKeys) {
                if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                    fixturePositionPriorities[fixtureKey] = priority;
                    if (currentCueFixturePositionKeys.contains(fixtureKey)) {
                        PositionData currentPosition = getFixturePosition(fixtureKey, currentCueFixturePositionKeys.value(fixtureKey));
                        if (lastCueFixturePositionKeys.contains(fixtureKey)) {
                            PositionData lastPosition = getFixturePosition(fixtureKey, lastCueFixturePositionKeys.value(fixtureKey));
                            currentPosition.pan += (lastPosition.pan - currentPosition.pan) * fade;
                            currentPosition.tilt += (lastPosition.tilt - currentPosition.tilt) * fade;
                            currentPosition.zoom += (lastPosition.zoom - currentPosition.zoom) * fade;
                            currentPosition.focus += (lastPosition.focus - currentPosition.focus) * fade;
                        }
                        fixturePositions[fixtureKey] = currentPosition;
                    } else if (lastCueFixturePositionKeys.contains(fixtureKey)) {
                        fixturePositions[fixtureKey] = getFixturePosition(fixtureKey, lastCueFixturePositionKeys.value(fixtureKey));
                    }
                }
            }
            const QMap<int, QList<int>> lastCueFixtureRawKeys = getCurrentCueItemList(lastCueKey, "cue_group_raws");
            QSet<int> rawFixtureKeys;
            for (const int fixtureKey : currentCueFixtureRawKeys.keys()) {
                rawFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : lastCueFixtureRawKeys.keys()) {
                rawFixtureKeys.insert(fixtureKey);
            }
            for (const int fixtureKey : rawFixtureKeys) {
                QSet<int> channels;
                QMap<int, uchar> currentChannelValues;
                if (currentCueFixtureRawKeys.contains(fixtureKey)) {
                    currentChannelValues = getFixtureRaws(fixtureKey, currentCueFixtureRawKeys.value(fixtureKey));
                    for (const int channel : currentChannelValues.keys()) {
                        channels.insert(channel);
                    }
                }
                QMap<int, uchar> lastChannelValues;
                if (lastCueFixtureRawKeys.contains(fixtureKey)) {
                    lastChannelValues = getFixtureRaws(fixtureKey, lastCueFixtureRawKeys.value(fixtureKey));
                    for (const int channel : lastChannelValues.keys()) {
                        channels.insert(channel);
                    }
                }
                for (const int channel : channels) {
                    if (!fixtureChannelRaws.contains(fixtureKey)) {
                        fixtureChannelRaws[fixtureKey] = QMap<int, uchar>();
                        fixtureChannelRawPriorities[fixtureKey] = QMap<int, int>();
                    }
                    if (priority >= fixtureChannelRawPriorities.value(fixtureKey).value(channel, 0)) {
                        fixtureChannelRawPriorities[fixtureKey][channel] = priority;
                        if (currentChannelValues.contains(channel)) {
                            uchar value = currentChannelValues.value(channel);
                            if (lastChannelValues.contains(channel)) {
                                value += (lastChannelValues.value(channel) - value) * fade;
                            }
                            fixtureChannelRaws[fixtureKey][channel] = value;
                        } else if (lastChannelValues.contains(channel)) {
                            fixtureChannelRaws[fixtureKey][channel] = lastChannelValues.value(channel);
                        }
                    }
                }
            }
        } else {
            for (const int fixtureKey : currentCueFixtureIntensityKeys.keys()) {
                const float dimmer = getFixtureIntensity(fixtureKey, currentCueFixtureIntensityKeys.value(fixtureKey));
                if (dimmer > fixtureIntensities.value(fixtureKey, 0)) {
                    fixtureIntensities[fixtureKey] = dimmer;
                }
            }
            for (const int fixtureKey : currentCueFixtureColorKeys.keys()) {
                if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                    fixtureColorPriorities[fixtureKey] = priority;
                    fixtureColors[fixtureKey] = getFixtureColor(fixtureKey, currentCueFixtureColorKeys.value(fixtureKey));
                }
            }
            for (const int fixtureKey : currentCueFixturePositionKeys.keys()) {
                if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                    fixturePositionPriorities[fixtureKey] = priority;
                    fixturePositions[fixtureKey] = getFixturePosition(fixtureKey, currentCueFixturePositionKeys.value(fixtureKey));
                }
            }
            for (const int fixtureKey : currentCueFixtureRawKeys.keys()) {
                const QMap<int, uchar> channelValues = getFixtureRaws(fixtureKey, currentCueFixtureRawKeys.value(fixtureKey));
                for (const int channel : channelValues.keys()) {
                    if (!fixtureChannelRaws.contains(fixtureKey)) {
                        fixtureChannelRaws[fixtureKey] = QMap<int, uchar>();
                        fixtureChannelRawPriorities[fixtureKey] = QMap<int, int>();
                    }
                    if (priority >= fixtureChannelRawPriorities.value(fixtureKey).value(channel, 0)) {
                        fixtureChannelRaws[fixtureKey][channel] = channelValues.value(channel);
                    }
                }
            }
        }
    }
    if (cuelistTotalFadeFrames.contains(currentCuelistKey)) {
        fadeProgressBar->setRange(0, cuelistTotalFadeFrames.value(currentCuelistKey));
        fadeProgressBar->setValue(oldCuelistTotalFadeFrames.value(currentCuelistKey) - cuelistRemainingFadeFrames.value(currentCuelistKey, 0));
    } else {
        fadeProgressBar->setRange(0, 1);
        fadeProgressBar->setValue(1);
    }
    QSqlQuery fixtureQuery;
    if (!fixtureQuery.exec("SELECT key, universe, address, xposition, yposition FROM fixtures")) {
        qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        return;
    }
    QList<Preview2d::PreviewFixture> previewFixtures;
    QMap<int, QByteArray> dmxUniverses;
    QMap<int, float> lastFrameFixturePan = fixturePan;
    fixturePan.clear();
    while (fixtureQuery.next()) {
        const int fixtureKey = fixtureQuery.value(0).toInt();
        const int universe = fixtureQuery.value(1).toInt();
        const int address = fixtureQuery.value(2).toInt();
        const float dimmer = fixtureIntensities.value(fixtureKey);
        const ColorData color = fixtureColors.value(fixtureKey);
        float red = color.red;
        float green = color.green;
        float blue = color.blue;
        float quality = color.quality;
        const PositionData position = fixturePositions.value(fixtureKey);
        float pan = position.pan;
        float tilt = position.tilt;
        float zoom = position.zoom;
        float focus = position.focus;
        Preview2d::PreviewFixture previewFixture;
        previewFixture.x = fixtureQuery.value(3).toFloat();
        previewFixture.y = fixtureQuery.value(4).toFloat();
        previewFixture.color = QColor((red / 100) * (dimmer / 100) * 255, (green / 100) * (dimmer / 100) * 255, (blue / 100) * (dimmer / 100) * 255);
        previewFixture.pan = pan;
        previewFixture.tilt = tilt;
        previewFixture.zoom = zoom;
        previewFixtures.append(previewFixture);
        if (address > 0) {
            QSqlQuery modelQuery;
            modelQuery.prepare("SELECT models.channels, models.panrange, models.tiltrange, models.minzoom, models.maxzoom, fixtures.rotation, fixtures.invertpan FROM fixtures, models WHERE fixtures.key = :key AND fixtures.model_key = models.key");
            modelQuery.bindValue(":key", fixtureKey);
            if (modelQuery.exec()) {
                if (modelQuery.next()) {
                    const QString channels = modelQuery.value(0).toString();
                    const float panRange = modelQuery.value(1).toFloat();
                    const float tiltRange = modelQuery.value(2).toFloat();
                    const float minZoom = modelQuery.value(3).toFloat();
                    const float maxZoom = modelQuery.value(4).toFloat();
                    const float rotation = modelQuery.value(5).toFloat();
                    const int invertPan = modelQuery.value(6).toInt();
                    if (!dmxUniverses.contains(universe)) {
                        dmxUniverses[universe] = QByteArray(512, 0);
                    }
                    if (!channels.contains('D')) {
                        red *= (dimmer / 100);
                        green *= (dimmer / 100);
                        blue *= (dimmer / 100);
                    }
                    const float white = std::min(std::min(red, green), blue);
                    if (channels.contains('W')) {
                        red -= white * (quality / 100);
                        green -= white * (quality / 100);
                        blue -= white * (quality / 100);
                    }
                    if (invertPan == 1) {
                        pan = rotation - pan;
                    } else {
                        pan = rotation + pan;
                    }
                    while (pan >= 360) {
                        pan -= 360;
                    }
                    while (pan < 0) {
                        pan += 360;
                    }
                    pan = pan / panRange * 100;
                    pan = std::min<float>(pan, 100);
                    const float lastFramePan = lastFrameFixturePan.value(fixtureKey, 0);
                    for (float angle = pan; angle <= panRange; angle += 360) {
                        const float anglePan = angle / panRange * 100;
                        if (std::abs(lastFramePan - anglePan) < std::abs(lastFramePan - pan)) {
                            pan = anglePan;
                        }
                    }
                    fixturePan[fixtureKey] = pan;
                    tilt = 50 + (tilt / (tiltRange / 2) * 50);
                    tilt = std::min<float>(tilt, 100);
                    tilt = std::max<float>(tilt, 0);
                    zoom = (zoom - minZoom) / (maxZoom - minZoom) * 100;
                    zoom = std::min<float>(zoom, 100);
                    zoom = std::max<float>(zoom, 0);
                    for (int channel = address; channel < (address + channels.size()); channel++) {
                        QChar channelType = channels.at(channel - address);
                        const bool fine = (channelType != channelType.toUpper());
                        channelType = channelType.toUpper();
                        float value = 0;
                        if (channelType == QChar('D')) { // Dimmer
                            value = dimmer;
                        } else if (channelType == QChar('R')) { // Red
                            value = red;
                        } else if (channelType == QChar('G')) { // Green
                            value = green;
                        } else if (channelType == QChar('B')) { // Blue
                            value = blue;
                        } else if (channelType == QChar('W')) { // White
                            value = white;
                        } else if (channelType == QChar('C')) { // Cyan
                            value = (100 - red);
                        } else if (channelType == QChar('M')) { // Magenta
                            value = (100 - green);
                        } else if (channelType == QChar('Y')) { // Yellow
                            value = (100 - blue);
                        } else if (channelType == QChar('P')) { // Pan
                            value = pan;
                        } else if (channelType == QChar('T')) { // Tilt
                            value = tilt;
                        } else if (channelType == QChar('Z')) { // Zoom
                            value = zoom;
                        } else if (channelType == QChar('F')) { // Focus
                            value = focus;
                        } else if (channelType == QChar('0')) { // DMX 0
                            value = 0;
                        } else if (channelType == QChar('1')) { // DMX 255
                            value = 100;
                        } else {
                            Q_ASSERT(false);
                        }
                        Q_ASSERT((value <= 100) && (value >= 0));
                        if (channel <= 512) {
                            value *= 655.35;
                            if (fine) {
                                dmxUniverses[universe][channel - 1] = (uchar)((int)value % 256);
                            } else {
                                dmxUniverses[universe][channel - 1] = (uchar)((int)value / 256);
                            }
                        }
                    }
                    for (const int channel : fixtureChannelRaws.value(fixtureKey, QMap<int, uchar>()).keys()) {
                        const int dmxChannel = address + channel - 1;
                        if (dmxChannel <= 512) {
                            dmxUniverses[universe][dmxChannel - 1] = fixtureChannelRaws.value(fixtureKey).value(channel);
                        }
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << modelQuery.executedQuery() << modelQuery.lastError().text();
            }
        }
    }
    for (const int universe : dmxUniverses.keys()) {
        emit sendUniverse(universe, dmxUniverses.value(universe));
    }
    emit updatePreviewFixtures(previewFixtures);
}

QMap<int, int> DmxEngine::getCurrentCueItems(const int cueId, const QString table) {
    QSqlQuery query;
    query.prepare("SELECT group_fixtures.valueitem_key, " + table + ".valueitem_key FROM group_fixtures, groups, " + table + " WHERE group_fixtures.item_key = groups.key AND " + table + ".foreignitem_key = groups.key AND " + table + ".item_key = :cue ORDER BY groups.sortkey");
    query.bindValue(":cue", cueId);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QMap<int, int>();
    }
    QMap<int, int> fixtureKeys;
    while (query.next()) {
        fixtureKeys[query.value(0).toInt()] = query.value(1).toInt();
    }
    return fixtureKeys;
}

QMap<int, QList<int>> DmxEngine::getCurrentCueItemList(const int cueId, const QString table) {
    QSqlQuery query;
    query.prepare("SELECT group_fixtures.valueitem_key, " + table + ".valueitem_key FROM group_fixtures, groups, " + table + " WHERE group_fixtures.item_key = groups.key AND " + table + ".foreignitem_key = groups.key AND " + table + ".item_key = :cue ORDER BY groups.sortkey");
    query.bindValue(":cue", cueId);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QMap<int, QList<int>>();
    }
    QMap<int, QList<int>> fixtureKeyLists;
    while (query.next()) {
        const int fixtureKey = query.value(0).toInt();
        if (!fixtureKeyLists.contains(fixtureKey)) {
            fixtureKeyLists[fixtureKey] = QList<int>();
        }
        fixtureKeyLists[fixtureKey].append(query.value(1).toInt());
    }
    return fixtureKeyLists;
}

float DmxEngine::getFixtureValue(const int fixtureKey, const int itemKey, const QString itemTable, const QString itemTableAttribute, const QString modelExceptionTable, const QString fixtureExceptionTable) {
    QSqlQuery fixtureExceptionQuery;
    fixtureExceptionQuery.prepare("SELECT value FROM " + fixtureExceptionTable + " WHERE item_key = :item AND foreignitem_key = :fixture");
    fixtureExceptionQuery.bindValue(":item", itemKey);
    fixtureExceptionQuery.bindValue(":fixture", fixtureKey);
    if (!fixtureExceptionQuery.exec()) {
        qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
        return 0;
    }
    if (fixtureExceptionQuery.next()) {
        return fixtureExceptionQuery.value(0).toFloat();
    }
    QSqlQuery modelExceptionQuery;
    modelExceptionQuery.prepare("SELECT " + modelExceptionTable + ".value FROM " + modelExceptionTable + ", fixtures WHERE " + modelExceptionTable + ".item_key = :item AND " + modelExceptionTable + ".foreignitem_key = fixtures.model_key AND fixtures.key = :fixture");
    modelExceptionQuery.bindValue(":item", itemKey);
    modelExceptionQuery.bindValue(":fixture", fixtureKey);
    if (!modelExceptionQuery.exec()) {
        qWarning() << Q_FUNC_INFO << modelExceptionQuery.executedQuery() << modelExceptionQuery.lastError().text();
        return 0;
    }
    if (modelExceptionQuery.next()) {
        return modelExceptionQuery.value(0).toFloat();
    }
    QSqlQuery itemQuery;
    itemQuery.prepare("SELECT " + itemTableAttribute + " FROM " + itemTable + " WHERE key = :item");
    itemQuery.bindValue(":item", itemKey);
    if (!itemQuery.exec()) {
        qWarning() << Q_FUNC_INFO << itemQuery.executedQuery() << itemQuery.lastError().text();
        return 0;
    }
    if (itemQuery.next()) {
        return itemQuery.value(0).toFloat();
    }
    return 0;
}

float DmxEngine::getFixtureIntensity(const int fixtureKey, const int intensityKey) {
    return getFixtureValue(fixtureKey, intensityKey, "intensities", "dimmer", "intensity_model_dimmer", "intensity_fixture_dimmer");
}

DmxEngine::ColorData DmxEngine::getFixtureColor(const int fixtureKey, const int colorKey) {
    const float hue = getFixtureValue(fixtureKey, colorKey, "colors", "hue", "color_model_hue", "color_fixture_hue");
    const float saturation = getFixtureValue(fixtureKey, colorKey, "colors", "saturation", "color_model_saturation", "color_fixture_saturation");
    ColorData color;
    const float h = (hue / 60.0);
    const int i = (int)h;
    const float f = h - i;
    const float p = (100 - saturation);
    const float q = (100 - (saturation * f));
    const float t = (100 - (saturation * (1 - f)));
    if (i == 0) {
        color.red = 100;
        color.green = t;
        color.blue = p;
    } else if (i == 1) {
        color.red = q;
        color.green = 100.0;
        color.blue = p;
    } else if (i == 2) {
        color.red = p;
        color.green = 100.0;
        color.blue = t;
    } else if (i == 3) {
        color.red = p;
        color.green = q;
        color.blue = 100.0;
    } else if (i == 4) {
        color.red = t;
        color.green = p;
        color.blue = 100.0;
    } else if (i == 5) {
        color.red = 100.0;
        color.green = p;
        color.blue = q;
    }
    color.quality = getFixtureValue(fixtureKey, colorKey, "colors", "quality", "color_model_quality", "color_fixture_quality");
    return color;
}

DmxEngine::PositionData DmxEngine::getFixturePosition(const int fixtureKey, const int positionKey) {
    PositionData position;
    position.pan = getFixtureValue(fixtureKey, positionKey, "positions", "pan", "position_model_pan", "position_fixture_pan");
    position.tilt = getFixtureValue(fixtureKey, positionKey, "positions", "tilt", "position_model_tilt", "position_fixture_tilt");
    position.zoom = getFixtureValue(fixtureKey, positionKey, "positions", "zoom", "position_model_zoom", "position_fixture_zoom");
    position.focus = getFixtureValue(fixtureKey, positionKey, "positions", "focus", "position_model_focus", "position_fixture_focus");
    return position;
}

QMap<int, uchar> DmxEngine::getFixtureRaws(const int fixtureKey, const QList<int> rawKeys) {
    QMap<int, uchar> channels;
    for (const int rawKey : rawKeys) {
        QSqlQuery itemQuery;
        itemQuery.prepare("SELECT key, value FROM raw_channel_values WHERE item_key = :raw");
        itemQuery.bindValue(":raw", rawKey);
        if (itemQuery.exec()) {
            while (itemQuery.next()) {
                channels[itemQuery.value(0).toInt()] = (uchar)itemQuery.value(1).toUInt();
            }
        } else {
            qWarning() << Q_FUNC_INFO << itemQuery.executedQuery() << itemQuery.lastError().text();
        }
        QSqlQuery modelExceptionQuery;
        modelExceptionQuery.prepare("SELECT raw_model_channel_values.key, raw_model_channel_values.value FROM raw_model_channel_values, fixtures WHERE raw_model_channel_values.item_key = :raw AND raw_model_channel_values.foreignitem_key = fixtures.model_key AND fixtures.key = :fixture");
        modelExceptionQuery.bindValue(":raw", rawKey);
        modelExceptionQuery.bindValue(":fixture", fixtureKey);
        if (modelExceptionQuery.exec()) {
            while (modelExceptionQuery.next()) {
                channels[modelExceptionQuery.value(0).toInt()] = (uchar)modelExceptionQuery.value(1).toUInt();
            }
        } else {
            qWarning() << Q_FUNC_INFO << modelExceptionQuery.executedQuery() << modelExceptionQuery.lastError().text();
        }
        QSqlQuery fixtureExceptionQuery;
        fixtureExceptionQuery.prepare("SELECT key, value FROM raw_fixture_channel_values WHERE item_key = :raw AND foreignitem_key = :fixture");
        fixtureExceptionQuery.bindValue(":raw", rawKey);
        fixtureExceptionQuery.bindValue(":fixture", fixtureKey);
        if (fixtureExceptionQuery.exec()) {
            while (fixtureExceptionQuery.next()) {
                channels[fixtureExceptionQuery.value(0).toInt()] = (uchar)fixtureExceptionQuery.value(1).toUInt();
            }
        } else {
            qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
        }
    }
    return channels;
}
