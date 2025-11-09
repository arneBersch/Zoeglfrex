/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"

DmxEngine::DmxEngine(QWidget* parent) : QWidget(parent) {
    settings = new QSettings("Zoeglfrex");

    QHBoxLayout* layout = new QHBoxLayout();
    setLayout(layout);

    highlightButton = new QPushButton("Highlight");
    highlightButton->setCheckable(true);
    connect(highlightButton, &QPushButton::clicked, this, [this] {
        settings->setValue("cuelistview/highlight", highlightButton->isChecked());
    });
    highlightButton->setChecked(settings->value("cuelistview/highlight", true).toBool());
    layout->addWidget(highlightButton);

    soloButton = new QPushButton("Solo");
    soloButton->setCheckable(true);
    connect(soloButton, &QPushButton::clicked, this, [this] {
        settings->setValue("cuelistview/solo", soloButton->isChecked());
    });
    soloButton->setChecked(settings->value("cuelistview/solo", true).toBool());
    layout->addWidget(soloButton);

    fadeProgressBar = new QProgressBar();
    fadeProgressBar->setRange(0, 10);
    fadeProgressBar->setValue(5);
    layout->addWidget(fadeProgressBar);

    skipFadeButton = new QPushButton("Skip Fade");
    skipFadeButton->setCheckable(true);
    connect(skipFadeButton, &QPushButton::clicked, this, [this] {
        settings->setValue("cuelistview/skipfade", skipFadeButton->isChecked());
    });
    skipFadeButton->setChecked(settings->value("cuelistview/skipfade", true).toBool());
    layout->addWidget(skipFadeButton);

    new QShortcut(Qt::SHIFT | Qt::Key_H, this, [this] { highlightButton->click(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_S, this, [this] { soloButton->click(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_F, this, [this] { skipFadeButton->click(); }, Qt::ApplicationShortcut);

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

    groupKeys.clear();
    groupFixtureKeys.clear();
    QSet<int> fixtureKeys;
    QSqlQuery groupFixturesQuery;
    if (groupFixturesQuery.exec("SELECT groups.key, group_fixtures.valueitem_key FROM groups, group_fixtures WHERE groups.key = group_fixtures.item_key ORDER BY groups.sortkey")) {
        while (groupFixturesQuery.next()) {
            const int groupKey = groupFixturesQuery.value(0).toInt();
            const int fixtureKey = groupFixturesQuery.value(1).toInt();
            if (!groupKeys.contains(groupKey)) {
                groupKeys.append(groupKey);
                groupFixtureKeys[groupKey] = QSet<int>();
            }
            groupFixtureKeys[groupKey].insert(fixtureKey);
            fixtureKeys.insert(fixtureKey);
        }
    } else {
        qWarning() << Q_FUNC_INFO << groupFixturesQuery.executedQuery() << groupFixturesQuery.lastError().text();
        return;
    }

    QSqlQuery cuelistQuery;
    if (!cuelistQuery.exec("SELECT key, currentcue_key, lastcue_key, priority FROM cuelists WHERE currentcue_key IS NOT NULL ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QHash<int, QHash<int, int>> oldGroupEffectFrames = groupEffectFrames;
    groupEffectFrames.clear();
    QHash<int, int> oldCuelistCurrentCueKeys = cuelistCurrentCueKeys;
    cuelistCurrentCueKeys.clear();
    QHash<int, int> oldCuelistRemainingFadeFrames = cuelistRemainingFadeFrames;
    cuelistRemainingFadeFrames.clear();
    QHash<int, int> oldCuelistTotalFadeFrames = cuelistTotalFadeFrames;
    cuelistTotalFadeFrames.clear();
    QHash<int, float> fixtureIntensities;
    QHash<int, ColorData> fixtureColors;
    QHash<int, int> fixtureColorPriorities;
    QHash<int, PositionData> fixturePositions;
    QHash<int, int> fixturePositionPriorities;
    QHash<int, QHash<int, uint8_t>> fixtureChannelRaws;
    QHash<int, QHash<int, int>> fixtureChannelRawPriorities;
    while (cuelistQuery.next()) {
        const int cuelistKey = cuelistQuery.value(0).toInt();
        const int currentCueKey = cuelistQuery.value(1).toInt();
        const int lastCueKey = cuelistQuery.value(2).toInt();
        const int priority = cuelistQuery.value(3).toInt();
        if (!skipFadeButton->isChecked()) {
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
        }
        cuelistCurrentCueKeys[cuelistKey] = currentCueKey;
        const float fade = (float)cuelistRemainingFadeFrames.value(cuelistKey, 0) / (float)cuelistTotalFadeFrames.value(cuelistKey, 1);
        QHash<int, float> currentCueFixtureIntensities;
        QHash<int, float> lastCueFixtureIntensities;
        QHash<int, ColorData> currentCueFixtureColors;
        QHash<int, ColorData> lastCueFixtureColors;
        QHash<int, PositionData> currentCueFixturePositions;
        QHash<int, PositionData> lastCueFixturePositions;
        QHash<int, RawData> currentCueFixtureRaws;
        QHash<int, RawData> lastCueFixtureRaws;
        renderCue(currentCueKey, oldGroupEffectFrames, &currentCueFixtureIntensities, &currentCueFixtureColors, &currentCueFixturePositions, &currentCueFixtureRaws);
        if (fade > 0) {
            renderCue(lastCueKey, oldGroupEffectFrames, &lastCueFixtureIntensities, &lastCueFixtureColors, &lastCueFixturePositions, &lastCueFixtureRaws);
        }
        for (const int fixtureKey : fixtureKeys) {
            float currentIntensity = currentCueFixtureIntensities.value(fixtureKey, 0);
            float lastIntensity = lastCueFixtureIntensities.value(fixtureKey, 0);
            currentIntensity += (lastIntensity - currentIntensity) * fade;
            if (currentIntensity > fixtureIntensities.value(fixtureKey, 0)) {
                fixtureIntensities[fixtureKey] = currentIntensity;
            }
            if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                if (lastCueFixtureColors.contains(fixtureKey)) {
                    ColorData lastColor = lastCueFixtureColors.value(fixtureKey);
                    if (currentCueFixtureColors.contains(fixtureKey)) {
                        ColorData currentColor = currentCueFixtureColors.value(fixtureKey);
                        lastColor.red = currentColor.red + (lastColor.red - currentColor.red) * fade;
                        lastColor.green = currentColor.green + (lastColor.green - currentColor.green) * fade;
                        lastColor.blue = currentColor.blue + (lastColor.blue - currentColor.blue) * fade;
                        lastColor.quality = currentColor.quality + (lastColor.quality - currentColor.quality) * fade;
                    }
                    fixtureColors[fixtureKey] = lastColor;
                    fixtureColorPriorities[fixtureKey] = priority;
                } else if (currentCueFixtureColors.contains(fixtureKey)) {
                    fixtureColors[fixtureKey] = currentCueFixtureColors.value(fixtureKey);
                    fixtureColorPriorities[fixtureKey] = priority;
                }
            }
            if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                if (lastCueFixturePositions.contains(fixtureKey)) {
                    PositionData lastPosition = lastCueFixturePositions.value(fixtureKey);
                    if (currentCueFixturePositions.contains(fixtureKey)) {
                        PositionData currentPosition = currentCueFixturePositions.value(fixtureKey);
                        if (std::abs(currentPosition.pan - lastPosition.pan) > 180) {
                            if (lastPosition.pan > currentPosition.pan) {
                                currentPosition.pan += 360;
                            } else {
                                lastPosition.pan += 360;
                            }
                        }
                        lastPosition.pan = currentPosition.pan + (lastPosition.pan - currentPosition.pan) * fade;
                        while (lastPosition.pan >= 360) {
                            lastPosition.pan -= 360;
                        }
                        lastPosition.tilt = currentPosition.tilt + (lastPosition.tilt - currentPosition.tilt) * fade;
                        lastPosition.zoom = currentPosition.zoom + (lastPosition.zoom - currentPosition.zoom) * fade;
                        lastPosition.focus = currentPosition.focus + (lastPosition.focus - currentPosition.focus) * fade;
                    }
                    fixturePositions[fixtureKey] = lastPosition;
                    fixturePositionPriorities[fixtureKey] = priority;
                } else if (currentCueFixturePositions.contains(fixtureKey)) {
                    fixturePositions[fixtureKey] = currentCueFixturePositions.value(fixtureKey);
                    fixturePositionPriorities[fixtureKey] = priority;
                }
            }
            QSet<int> rawChannels;
            RawData currentRaws = currentCueFixtureRaws.value(fixtureKey);
            for (const int channel : currentRaws.channelValues.keys()) {
                rawChannels.insert(channel);
            }
            RawData lastRaws = lastCueFixtureRaws.value(fixtureKey);
            for (const int channel : lastRaws.channelValues.keys()) {
                rawChannels.insert(channel);
            }
            for (const int channel : rawChannels) {
                if (!fixtureChannelRaws.contains(fixtureKey)) {
                    fixtureChannelRaws[fixtureKey] = QHash<int, uint8_t>();
                    fixtureChannelRawPriorities[fixtureKey] = QHash<int, int>();
                }
                if (priority >= fixtureChannelRawPriorities.value(fixtureKey).value(channel, 0)) {
                    fixtureChannelRawPriorities[fixtureKey][channel] = priority;
                    if (lastRaws.channelValues.contains(channel)) {
                        uint8_t lastValue = lastRaws.channelValues.value(channel);
                        if (currentRaws.channelValues.contains(channel)) {
                            uint8_t currentValue = currentRaws.channelValues.value(channel);
                            if (currentRaws.channelFading.contains(channel)) {
                                lastValue = currentValue + (lastValue - currentValue) * fade;
                            } else {
                                lastValue = currentValue;
                            }
                        }
                        fixtureChannelRaws[fixtureKey][channel] = lastValue;
                    } else if (currentRaws.channelValues.contains(channel)) {
                        fixtureChannelRaws[fixtureKey][channel] = currentRaws.channelValues.value(channel);
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
    QSet<int> currentFixtureKeys;
    QSqlQuery currentFixtureQuery;
    if (currentFixtureQuery.exec("SELECT key FROM currentfixtures")) {
        while (currentFixtureQuery.next()) {
            currentFixtureKeys.insert(currentFixtureQuery.value(0).toInt());
        }
    } else {
        qWarning() << Q_FUNC_INFO << currentFixtureQuery.executedQuery() << currentFixtureQuery.lastError().text();
    }
    QSqlQuery fixtureQuery;
    if (!fixtureQuery.exec("SELECT key, universe, address, xposition, yposition FROM fixtures")) {
        qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        return;
    }
    QList<Preview2d::PreviewFixture> previewFixtures;
    QHash<int, QByteArray> dmxUniverses;
    QHash<int, float> lastFrameFixturePan = fixturePan;
    fixturePan.clear();
    while (fixtureQuery.next()) {
        const int fixtureKey = fixtureQuery.value(0).toInt();
        const int universe = fixtureQuery.value(1).toInt();
        const int address = fixtureQuery.value(2).toInt();
        float dimmer = fixtureIntensities.value(fixtureKey);
        const ColorData color = fixtureColors.value(fixtureKey);
        float red = color.red;
        float green = color.green;
        float blue = color.blue;
        float quality = color.quality;
        if (currentFixtureKeys.contains(fixtureKey) && highlightButton->isChecked()) {
            dimmer = 100;
            red = 100;
            green = 100;
            blue = 100;
            quality = 0;
        } else if (!currentFixtureKeys.contains(fixtureKey) && soloButton->isChecked()) {
            dimmer = 0;
        }
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
                                dmxUniverses[universe][channel - 1] = (uint8_t)((int)value % 256);
                            } else {
                                dmxUniverses[universe][channel - 1] = (uint8_t)((int)value / 256);
                            }
                        }
                    }
                    for (const int channel : fixtureChannelRaws.value(fixtureKey, QHash<int, uint8_t>()).keys()) {
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

void DmxEngine::renderCue(const int cueKey, QHash<int, QHash<int, int>> oldGroupEffectFrames, QHash<int, float>* fixtureIntensities, QHash<int, ColorData>* fixtureColors, QHash<int, PositionData>* fixturePositions, QHash<int, RawData>* fixtureRaws) {
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
                    const float dimmer = getFixtureIntensity(fixtureKey, intensityKey);
                    if (fixtureIntensities->value(fixtureKey, 0) <= dimmer) {
                        (*fixtureIntensities)[fixtureKey] = dimmer;
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
                    (*fixtureColors)[fixtureKey] = getFixtureColor(fixtureKey, colorKey);
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
                    (*fixturePositions)[fixtureKey] = getFixturePosition(fixtureKey, positionKey);
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
                const RawData raws = getFixtureRaws(fixtureKey, rawKeys);
                if (fixtureRaws->contains(fixtureKey)) {
                    for (const int channel : raws.channelValues.keys()) {
                        (*fixtureRaws)[fixtureKey].channelValues[channel] = raws.channelValues.value(channel);
                        (*fixtureRaws)[fixtureKey].channelFading[channel] = raws.channelFading.value(channel);
                    }
                } else {
                    (*fixtureRaws)[fixtureKey] = raws;
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
                effectKeys.append(rawQuery.value(0).toInt());
            }
            for (const int effectKey : effectKeys) {
                if (!groupEffectFrames.contains(groupKey)) {
                    groupEffectFrames[groupKey] = QHash<int, int>();
                }
                if (oldGroupEffectFrames.value(groupKey).contains(effectKey)) {
                    groupEffectFrames[groupKey][effectKey] = oldGroupEffectFrames.value(groupKey).value(effectKey) + 1;
                } else {
                    groupEffectFrames[groupKey][effectKey] = 1;
                }
                QSqlQuery effectAttributesQuery;
                effectAttributesQuery.prepare("SELECT steps, hold, fade, phase FROM effects WHERE key = :effect");
                effectAttributesQuery.bindValue(":effect", effectKey);
                if (effectAttributesQuery.exec()) {
                    if (effectAttributesQuery.next()) {
                        const int stepAmount = effectAttributesQuery.value(0).toInt();
                        const int standardHoldFrames = (effectAttributesQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                        const int standardFadeFrames = (effectAttributesQuery.value(2).toFloat() * 1000 / FRAMEDURATION);
                        const int standardPhase = effectAttributesQuery.value(3).toFloat();
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
                            QHash<int, float> fixtureKeyPhase;
                            QSqlQuery fixturePhaseQuery;
                            fixturePhaseQuery.prepare("SELECT foreignitem_key, value FROM effect_fixture_phase WHERE item_key = :effect");
                            fixturePhaseQuery.bindValue(":effect", effectKey);
                            if (fixturePhaseQuery.exec()) {
                                while (fixturePhaseQuery.next()) {
                                    fixtureKeyPhase[fixturePhaseQuery.value(0).toInt()] = fixturePhaseQuery.value(1).toFloat();
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
                                    const int step = positionStepQuery.value(0).toInt();
                                    if (step <= stepAmount) {
                                        if (!stepRawKeys.contains(step)) {
                                            stepRawKeys[step] = QList<int>();
                                        }
                                        stepRawKeys[step].append(positionStepQuery.value(1).toInt());
                                    }
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << rawStepQuery.executedQuery() << rawStepQuery.lastError().text();
                            }
                            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                                int frames = (int)(groupEffectFrames.value(groupKey).value(effectKey) + (fixtureKeyPhase.value(fixtureKey, standardPhase) / 360)) % totalFrames;
                                int currentStep = 1;
                                float fade = 0;
                                for (int step = 1; step <= stepAmount; step++) {
                                    if (frames >= 0) {
                                        currentStep = step;
                                        fade = 0;
                                    }
                                    frames -= stepHoldFrames.value(step, standardHoldFrames);
                                    if ((frames >= 0) && (currentStep == step)) {
                                        fade = frames / stepFadeFrames.value(step, standardFadeFrames);
                                    }
                                    frames -= stepFadeFrames.value(step, standardFadeFrames);
                                }
                                int nextStep = currentStep + 1;
                                if (nextStep > stepAmount) {
                                    nextStep = 1;
                                }
                                if (!stepIntensityKeys.isEmpty()) {
                                    float dimmer = 0;
                                    if (stepIntensityKeys.contains(currentStep)) {
                                        dimmer = getFixtureIntensity(fixtureKey, stepIntensityKeys.value(currentStep));
                                    }
                                    if (fade > 0) {
                                        float nextDimmer = 0;
                                        if (stepIntensityKeys.contains(nextStep)) {
                                            nextDimmer = getFixtureIntensity(fixtureKey, stepIntensityKeys.value(nextStep));
                                        }
                                        dimmer += (nextDimmer - dimmer) * fade;
                                    }
                                    if (dimmer >= fixtureIntensities->value(fixtureKey, 0)) {
                                        (*fixtureIntensities)[fixtureKey] = dimmer;
                                    }
                                }
                                if (!stepColorKeys.isEmpty()) {
                                    ColorData color;
                                    if (stepColorKeys.contains(currentStep)) {
                                        color = getFixtureColor(fixtureKey, stepColorKeys.value(currentStep));
                                    }
                                    if (fade > 0) {
                                        ColorData nextColor;
                                        if (stepColorKeys.contains(nextStep)) {
                                            nextColor = getFixtureColor(fixtureKey, stepColorKeys.value(nextStep));
                                        }
                                        color.red += (nextColor.red - color.red) * fade;
                                        color.green += (nextColor.green - color.green) * fade;
                                        color.blue += (nextColor.blue - color.blue) * fade;
                                        color.quality += (nextColor.quality - color.quality) * fade;
                                    }
                                    (*fixtureColors)[fixtureKey] = color;
                                }
                                if (!stepColorKeys.isEmpty()) {
                                    PositionData position;
                                    if (stepPositionKeys.contains(currentStep)) {
                                        position = getFixturePosition(fixtureKey, stepPositionKeys.value(currentStep));
                                    }
                                    if (fade > 0) {
                                        PositionData nextPosition;
                                        if (stepPositionKeys.contains(nextStep)) {
                                            nextPosition = getFixturePosition(fixtureKey, stepPositionKeys.value(nextStep));
                                        }
                                        position.pan += (nextPosition.pan - position.pan) * fade;
                                        position.tilt += (nextPosition.tilt - position.tilt) * fade;
                                        position.zoom += (nextPosition.zoom - position.zoom) * fade;
                                        position.focus += (nextPosition.focus - position.focus) * fade;
                                    }
                                    (*fixturePositions)[fixtureKey] = position;
                                }
                                if (stepPositionKeys.contains(currentStep)) {
                                    (*fixturePositions)[fixtureKey] = getFixturePosition(fixtureKey, stepPositionKeys.value(currentStep));
                                }
                            }
                        }
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << effectAttributesQuery.executedQuery() << effectAttributesQuery.lastError().text();
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << effectQuery.executedQuery() << effectQuery.lastError().text();
        }
    }
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

DmxEngine::RawData DmxEngine::getFixtureRaws(const int fixtureKey, const QList<int> rawKeys) {
    RawData channels;
    for (const int rawKey : rawKeys) {
        bool fading = false;
        QSqlQuery fadingQuery;
        fadingQuery.prepare("SELECT fade FROM raws WHERE key = :raw");
        fadingQuery.bindValue(":raw", rawKey);
        if (fadingQuery.exec()) {
            if (fadingQuery.next()) {
                fading = (fadingQuery.value(0).toInt() == 1);
            }
        } else {
            qWarning() << Q_FUNC_INFO << fadingQuery.executedQuery() << fadingQuery.lastError().text();
        }
        QSqlQuery itemQuery;
        itemQuery.prepare("SELECT key, value FROM raw_channel_values WHERE item_key = :raw");
        itemQuery.bindValue(":raw", rawKey);
        if (itemQuery.exec()) {
            while (itemQuery.next()) {
                const int channel = itemQuery.value(0).toInt();
                channels.channelValues[channel] = (uint8_t)itemQuery.value(1).toUInt();
                channels.channelFading[channel] = fading;
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
                const int channel = itemQuery.value(0).toInt();
                channels.channelValues[channel] = (uint8_t)modelExceptionQuery.value(1).toUInt();
                channels.channelFading[channel] = fading;
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
                const int channel = itemQuery.value(0).toInt();
                channels.channelValues[channel] = (uint8_t)fixtureExceptionQuery.value(1).toUInt();
                channels.channelFading[channel] = fading;
            }
        } else {
            qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
        }
    }
    return channels;
}
