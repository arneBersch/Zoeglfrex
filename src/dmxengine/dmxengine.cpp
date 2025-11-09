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
        renderCue(currentCueKey, &currentCueFixtureIntensities, &currentCueFixtureColors, &currentCueFixturePositions, &currentCueFixtureRaws);
        if (fade > 0) {
            renderCue(lastCueKey, &lastCueFixtureIntensities, &lastCueFixtureColors, &lastCueFixturePositions, &lastCueFixtureRaws);
        }
        QSet<int> intensityFixtureKeys;
        for (const int fixtureKey : currentCueFixtureIntensities.keys()) {
            intensityFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : lastCueFixtureIntensities.keys()) {
            intensityFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : intensityFixtureKeys) {
            float currentIntensity = currentCueFixtureIntensities.value(fixtureKey, 0);
            float lastIntensity = lastCueFixtureIntensities.value(fixtureKey, 0);
            currentIntensity += (lastIntensity - currentIntensity) * fade;
            if (currentIntensity > fixtureIntensities.value(fixtureKey, 0)) {
                fixtureIntensities[fixtureKey] = currentIntensity;
            }
        }
        QSet<int> colorFixtureKeys;
        for (const int fixtureKey : currentCueFixtureColors.keys()) {
            colorFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : lastCueFixtureColors.keys()) {
            colorFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : colorFixtureKeys) {
            if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                fixtureColorPriorities[fixtureKey] = priority;
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
                } else if (currentCueFixtureColors.contains(fixtureKey)) {
                    fixtureColors[fixtureKey] = currentCueFixtureColors.value(fixtureKey);
                }
            }
        }
        QSet<int> positionFixtureKeys;
        for (const int fixtureKey : currentCueFixturePositions.keys()) {
            positionFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : lastCueFixturePositions.keys()) {
            positionFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : positionFixtureKeys) {
            if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                fixturePositionPriorities[fixtureKey] = priority;
                if (lastCueFixturePositions.contains(fixtureKey)) {
                    PositionData lastPosition = lastCueFixturePositions.value(fixtureKey);
                    if (currentCueFixturePositions.contains(fixtureKey)) {
                        PositionData currentPosition = currentCueFixturePositions.value(fixtureKey);
                        lastPosition.pan = currentPosition.pan + (lastPosition.pan - currentPosition.pan) * fade;
                        lastPosition.tilt = currentPosition.tilt + (lastPosition.tilt - currentPosition.tilt) * fade;
                        lastPosition.zoom = currentPosition.zoom + (lastPosition.zoom - currentPosition.zoom) * fade;
                        lastPosition.focus = currentPosition.focus + (lastPosition.focus - currentPosition.focus) * fade;
                    }
                    fixturePositions[fixtureKey] = lastPosition;
                } else if (currentCueFixturePositions.contains(fixtureKey)) {
                    fixturePositions[fixtureKey] = currentCueFixturePositions.value(fixtureKey);
                }
            }
        }
        QSet<int> rawFixtureKeys;
        for (const int fixtureKey : currentCueFixtureRaws.keys()) {
            rawFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : lastCueFixtureRaws.keys()) {
            rawFixtureKeys.insert(fixtureKey);
        }
        for (const int fixtureKey : rawFixtureKeys) {
            QSet<int> channels;
            RawData currentRaws = currentCueFixtureRaws.value(fixtureKey);
            for (const int channel : currentRaws.channelValues.keys()) {
                channels.insert(channel);
            }
            RawData lastRaws = lastCueFixtureRaws.value(fixtureKey);
            for (const int channel : lastRaws.channelValues.keys()) {
                channels.insert(channel);
            }
            for (const int channel : channels) {
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

void DmxEngine::renderCue(const int cueKey, QHash<int, float>* fixtureIntensities, QHash<int, ColorData>* fixtureColors, QHash<int, PositionData>* fixturePositions, QHash<int, RawData>* fixtureRaws) {
    for (const int groupKey : groupKeys) {
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
            }
        } else {
            qWarning() << Q_FUNC_INFO << positionQuery.executedQuery() << positionQuery.lastError().text();
        }
        QSqlQuery rawQuery;
        rawQuery.prepare("SELECT cue_group_raws.valueitem_key FROM cue_group_raws, raws WHERE cue_group_raws.item_key = :cue AND cue_group_raws.foreignitem_key = :group AND cue_group_raws.valueitem_key = raws.key ORDER BY raws.sortkey");
        rawQuery.bindValue(":group", groupKey);
        rawQuery.bindValue(":cue", cueKey);
        if (rawQuery.exec()) {
            QList<int> rawKeys;
            while (rawQuery.next()) {
                rawKeys.append(rawQuery.value(0).toInt());
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
        } else {
            qWarning() << Q_FUNC_INFO << positionQuery.executedQuery() << positionQuery.lastError().text();
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
