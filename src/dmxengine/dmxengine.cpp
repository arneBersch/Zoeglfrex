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

    QList<int> groupKeys;
    QHash<int, QSet<int>> groupFixtureKeys;
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
    if (!cuelistQuery.exec("SELECT key, currentcue_key, COALESCE(lastcue_key, -1), priority FROM cuelists WHERE currentcue_key IS NOT NULL ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QHash<int, QHash<int, int>> oldGroupEffectFrames = groupEffectFrames;
    groupEffectFrames.clear();
    QHash<int, int> oldCuelistCurrentCueKeys = cuelistCurrentCueKeys;
    cuelistCurrentCueKeys.clear();
    QHash<int, int> oldCuelistRemainingTransitionFrames = cuelistRemainingTransitionFrames;
    cuelistRemainingTransitionFrames.clear();
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
                QSqlQuery transitionQuery;
                transitionQuery.prepare("SELECT fade, delay, sinefade FROM cues WHERE key = :key");
                transitionQuery.bindValue(":key", currentCueKey);
                if (transitionQuery.exec()) {
                    if (transitionQuery.next()) {
                        const int standardFadeFrames = (transitionQuery.value(0).toFloat() * 1000 / FRAMEDURATION);
                        const int standardDelayFrames = (transitionQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                        const bool sineFade = (transitionQuery.value(2).toInt() == 1);
                        QHash<int, int> fixtureFadeFrames;
                        int maxFadeFrames = standardFadeFrames;
                        QSqlQuery fixtureFadeQuery;
                        fixtureFadeQuery.prepare("SELECT foreignitem_key, value FROM cue_fixture_fade WHERE item_key = :cue");
                        fixtureFadeQuery.bindValue(":cue", currentCueKey);
                        if (fixtureFadeQuery.exec()) {
                            while (fixtureFadeQuery.next()) {
                                const int fixtureKey = fixtureFadeQuery.value(0).toInt();
                                const int fadeFrames = (fixtureFadeQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                                fixtureFadeFrames[fixtureKey] = fadeFrames;
                                maxFadeFrames = std::max(fadeFrames, maxFadeFrames);
                            }
                        } else {
                            qWarning() << Q_FUNC_INFO << fixtureFadeQuery.executedQuery() << fixtureFadeQuery.lastError().text();
                        }
                        QHash<int, int> fixtureDelayFrames;
                        int maxDelayFrames = standardDelayFrames;
                        QSqlQuery fixtureDelayQuery;
                        fixtureDelayQuery.prepare("SELECT foreignitem_key, value FROM cue_fixture_delay WHERE item_key = :cue");
                        fixtureDelayQuery.bindValue(":cue", currentCueKey);
                        if (fixtureDelayQuery.exec()) {
                            while (fixtureDelayQuery.next()) {
                                const int fixtureKey = fixtureDelayQuery.value(0).toInt();
                                const int delayFrames = (fixtureDelayQuery.value(1).toFloat() * 1000 / FRAMEDURATION);
                                fixtureDelayFrames[fixtureKey] = delayFrames;
                                maxDelayFrames = std::max(delayFrames, maxDelayFrames);
                            }
                        } else {
                            qWarning() << Q_FUNC_INFO << fixtureDelayQuery.executedQuery() << fixtureDelayQuery.lastError().text();
                        }
                        if ((maxFadeFrames + maxDelayFrames) > 0) {
                            cuelistFadeFrames[cuelistKey] = standardFadeFrames;
                            cuelistFixtureFadeFrames[cuelistKey] = fixtureFadeFrames;
                            cuelistDelayFrames[cuelistKey] = standardDelayFrames;
                            cuelistFixtureDelayFrames[cuelistKey] = fixtureDelayFrames;
                            cuelistRemainingTransitionFrames[cuelistKey] = maxFadeFrames + maxDelayFrames;
                            cuelistTransitionFrames[cuelistKey] = maxFadeFrames + maxDelayFrames;
                            cuelistSineFade[cuelistKey] = sineFade;
                        }
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << transitionQuery.executedQuery() << transitionQuery.lastError().text();
                }
            } else if (oldCuelistRemainingTransitionFrames.value(cuelistKey, 0) > 0) {
                cuelistRemainingTransitionFrames[cuelistKey] = (oldCuelistRemainingTransitionFrames.value(cuelistKey) - 1);
            }
        }
        cuelistCurrentCueKeys[cuelistKey] = currentCueKey;

        const int remainingTransitionFrames = cuelistRemainingTransitionFrames.value(cuelistKey, 0);
        const int transitionFrames = cuelistTransitionFrames.value(cuelistKey, 0);

        QHash<int, float> currentCueFixtureIntensities;
        QHash<int, float> lastCueFixtureIntensities;
        QHash<int, ColorData> currentCueFixtureColors;
        QHash<int, ColorData> lastCueFixtureColors;
        QHash<int, PositionData> currentCueFixturePositions;
        QHash<int, PositionData> lastCueFixturePositions;
        QHash<int, QHash<int, RawChannelData>> currentCueFixtureRaws;
        QHash<int, QHash<int, RawChannelData>> lastCueFixtureRaws;

        renderCue(currentCueKey, groupKeys, groupFixtureKeys, oldGroupEffectFrames, &currentCueFixtureIntensities, &currentCueFixtureColors, &currentCueFixturePositions, &currentCueFixtureRaws);
        if ((remainingTransitionFrames > 0) && (lastCueKey >= 0)) {
            renderCue(lastCueKey, groupKeys, groupFixtureKeys, oldGroupEffectFrames, &lastCueFixtureIntensities, &lastCueFixtureColors, &lastCueFixturePositions, &lastCueFixtureRaws);
        }

        for (const int fixtureKey : fixtureKeys) {
            float fade = 0;
            if (remainingTransitionFrames > 0) {
                const int delayFrames = cuelistFixtureDelayFrames.value(cuelistKey).value(fixtureKey, cuelistDelayFrames.value(cuelistKey, 0));
                const int fadeFrames = cuelistFixtureFadeFrames.value(cuelistKey).value(fixtureKey, cuelistFadeFrames.value(cuelistKey, 0));
                if ((transitionFrames - remainingTransitionFrames) <= delayFrames) {
                    fade = 1;
                } else if ((transitionFrames - remainingTransitionFrames - delayFrames) <= fadeFrames) {
                    fade = 1 - (float)(transitionFrames - remainingTransitionFrames - delayFrames) / (float)fadeFrames;
                }
                if (cuelistSineFade.value(cuelistKey, false)) {
                    fade = std::cos(M_PI * (1 - fade)) / 2 + 0.5;
                }
            }
            float currentIntensity = currentCueFixtureIntensities.value(fixtureKey, 0);
            float lastIntensity = lastCueFixtureIntensities.value(fixtureKey, 0);
            currentIntensity += (lastIntensity - currentIntensity) * fade;
            if (currentIntensity > fixtureIntensities.value(fixtureKey, 0)) {
                fixtureIntensities[fixtureKey] = currentIntensity;
            }
            if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                if (lastCueFixtureColors.contains(fixtureKey) && (fade > 0)) {
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
                if (lastCueFixturePositions.contains(fixtureKey) && (fade > 0)) {
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
            QHash<int, RawChannelData> currentRaws = currentCueFixtureRaws.value(fixtureKey);
            for (const int channel : currentRaws.keys()) {
                rawChannels.insert(channel);
            }
            QHash<int, RawChannelData> lastRaws = lastCueFixtureRaws.value(fixtureKey);
            for (const int channel : lastRaws.keys()) {
                rawChannels.insert(channel);
            }
            for (const int channel : rawChannels) {
                if (!fixtureChannelRaws.contains(fixtureKey)) {
                    fixtureChannelRaws[fixtureKey] = QHash<int, uint8_t>();
                    fixtureChannelRawPriorities[fixtureKey] = QHash<int, int>();
                }
                if (priority >= fixtureChannelRawPriorities.value(fixtureKey).value(channel, 0)) {
                    fixtureChannelRawPriorities[fixtureKey][channel] = priority;
                    if (lastRaws.contains(channel) && (fade > 0)) {
                        uint8_t lastValue = lastRaws.value(channel).value;
                        if (currentRaws.contains(channel)) {
                            const uint8_t currentValue = currentRaws.value(channel).value;
                            if (currentRaws.value(channel).fading) {
                                lastValue = currentValue + (lastValue - currentValue) * fade;
                            } else {
                                lastValue = currentValue;
                            }
                        }
                        fixtureChannelRaws[fixtureKey][channel] = lastValue;
                    } else if (currentRaws.contains(channel)) {
                        fixtureChannelRaws[fixtureKey][channel] = currentRaws.value(channel).value;
                    }
                }
            }
        }
    }

    if (cuelistRemainingTransitionFrames.value(currentCuelistKey, 0) > 0) {
        fadeProgressBar->setRange(0, cuelistTransitionFrames.value(currentCuelistKey, 1));
        fadeProgressBar->setValue(cuelistTransitionFrames.value(currentCuelistKey, 0) - cuelistRemainingTransitionFrames.value(currentCuelistKey, 0));
    } else {
        fadeProgressBar->setRange(0, 1);
        fadeProgressBar->setValue(1);
    }

    QHash<int, int> mwdGroupCues;
    QHash<int, int> mwdGroupCueDifference;
    QHash<int, int> mwdGroupPriorities;
    QSqlQuery mwdCuelistQuery;
    if (mwdCuelistQuery.exec("SELECT key, priority, currentcue_key FROM cuelists WHERE movewhiledark = 1 AND currentcue_key IS NOT NULL ORDER BY sortkey")) {
        while (cuelistQuery.next()) {
            const int cuelistKey = mwdCuelistQuery.value(0).toInt();
            const int cuelistPriority = mwdCuelistQuery.value(1).toInt();
            const int cuelistCurrentCueKey = mwdCuelistQuery.value(2).toInt();

            QSqlQuery cuesQuery;
            cuesQuery.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey > (SELECT sortkey FROM cues WHERE key = :currentcue)");
            cuesQuery.bindValue(":cuelist", cuelistKey);
            cuesQuery.bindValue(":currentcue", cuelistCurrentCueKey);
            if (cuesQuery.exec()) {
                while (cuesQuery.next()) {
                    const int cueKey = cuesQuery.value(0).toInt();
                    const int cueDifference = cuesQuery.at() + 1;

                    for (const int groupKey : groupKeys) {
                        if (!mwdGroupCueDifference.contains(groupKey) || (mwdGroupCueDifference.value(groupKey) > cueDifference) || ((mwdGroupCueDifference.value(groupKey) >= cueDifference) && (cuelistPriority >= mwdGroupPriorities.value(groupKey)))) {
                            const QStringList tables = {"cue_group_intensities", "cue_group_colors", "cue_group_positions", "cue_group_raws", "cue_group_effects"};
                            for (QString table : tables) {
                                QSqlQuery cueValueQuery;
                                cueValueQuery.prepare("SELECT valueitem_key FROM " + table + " WHERE item_key = :cue AND foreignitem_key = :group LIMIT 1");
                                cueValueQuery.bindValue(":cue", cueKey);
                                cueValueQuery.bindValue(":group", groupKey);
                                if (cueValueQuery.exec()) {
                                    if (cueValueQuery.next()) {
                                        mwdGroupCues[groupKey] = cueKey;
                                        mwdGroupCueDifference[groupKey] = cueDifference;
                                        mwdGroupPriorities[groupKey] = cuelistPriority;
                                        break;
                                    }
                                } else {
                                    qWarning() << cueValueQuery.executedQuery() << cueValueQuery.lastError().text();
                                }
                            }
                        }
                    }
                }
            } else {
                qWarning() << cuesQuery.executedQuery() << cuesQuery.lastError().text();
            }
        }
    } else {
        qWarning() << mwdCuelistQuery.executedQuery() << mwdCuelistQuery.lastError().text();
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
    if (!fixtureQuery.exec("SELECT key, universe, address, xposition, yposition, CONCAT(id, ' ', label) FROM fixtures")) {
        qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        return;
    }
    QHash<int, Preview2d::PreviewData> previewFixtures;
    QHash<int, QByteArray> dmxUniverses;
    QHash<int, float> lastFrameFixturePan = fixturePan;
    fixturePan.clear();
    while (fixtureQuery.next()) {
        const int fixtureKey = fixtureQuery.value(0).toInt();
        const int universe = fixtureQuery.value(1).toInt();
        const int address = fixtureQuery.value(2).toInt();

        if (!fixtureIntensities.contains(fixtureKey) && !fixtureColors.contains(fixtureKey) && !fixturePositions.contains(fixtureKey) && !fixtureChannelRaws.contains(fixtureKey)) {
            int fixtureCue = -1;
            QList<int> fixtureGroups;
            int cueDifference = -1;
            int priority = 0;

            for (const int groupKey : groupKeys) {
                if (groupFixtureKeys.value(groupKey).contains(fixtureKey)) {
                    fixtureGroups.append(groupKey);
                    if (mwdGroupCueDifference.contains(groupKey) && ((cueDifference < 0) || (mwdGroupCueDifference.value(groupKey) < cueDifference) || ((mwdGroupCueDifference.value(groupKey) <= cueDifference) && (mwdGroupPriorities.value(groupKey) >= priority)))) {
                        cueDifference = mwdGroupCueDifference.value(groupKey);
                        fixtureCue = mwdGroupCues.value(groupKey);
                        priority = mwdGroupPriorities.value(groupKey);
                    }
                }
            }

            if (fixtureCue > 0) {
                QHash<int, QSet<int>> fixtureGroupFixture;
                for (const int groupKey : fixtureGroups) {
                    fixtureGroupFixture[groupKey] = QSet<int>();
                    fixtureGroupFixture[groupKey].insert(fixtureKey);
                }

                QHash<int, float> cueFixtureIntensities;
                QHash<int, ColorData> cueFixtureColors;
                QHash<int, PositionData> cueFixturePositions;
                QHash<int, QHash<int, RawChannelData>> cueFixtureRaws;
                renderCue(fixtureCue, fixtureGroups, fixtureGroupFixture, QHash<int, QHash<int, int>>(), &cueFixtureIntensities, &cueFixtureColors, &cueFixturePositions, &cueFixtureRaws);

                if (cueFixtureColors.contains(fixtureKey)) {
                    fixtureColors[fixtureKey] = cueFixtureColors.value(fixtureKey);
                }
                if (cueFixturePositions.contains(fixtureKey)) {
                    fixturePositions[fixtureKey] = cueFixturePositions.value(fixtureKey);
                }
                if (cueFixtureRaws.contains(fixtureKey)) {
                    fixtureChannelRaws[fixtureKey] = QHash<int, uint8_t>();
                    for (const int channel : cueFixtureRaws.value(fixtureKey).keys()) {
                        if (cueFixtureRaws.value(fixtureKey).value(channel).moveWhileDark) {
                            fixtureChannelRaws[fixtureKey][channel] = cueFixtureRaws.value(fixtureKey).value(channel).value;
                        }
                    }
                }
            }
        }

        float dimmer = fixtureIntensities.value(fixtureKey, 0);

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
        float panAngle = position.pan;
        float tiltAngle = position.tilt;
        float zoomAngle = position.zoom;
        float focus = position.focus;

        Preview2d::PreviewData previewFixture;
        previewFixture.xPosition = fixtureQuery.value(3).toFloat();
        previewFixture.yPosition = fixtureQuery.value(4).toFloat();
        previewFixture.label = fixtureQuery.value(5).toString();
        previewFixture.color = QColor((red / 100) * (dimmer / 100) * 255, (green / 100) * (dimmer / 100) * 255, (blue / 100) * (dimmer / 100) * 255);
        previewFixture.pan = panAngle;
        previewFixture.tilt = tiltAngle;
        previewFixture.zoom = zoomAngle;
        previewFixtures[fixtureKey] = previewFixture;

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
                    const bool invertPan = (modelQuery.value(6).toInt() == 1);
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
                    if (invertPan) {
                        panAngle = rotation - panAngle;
                    } else {
                        panAngle = rotation + panAngle;
                    }
                    while (panAngle >= 360) {
                        panAngle -= 360;
                    }
                    while (panAngle < 0) {
                        panAngle += 360;
                    }
                    float pan = (panAngle / panRange) * 100;
                    const float lastFramePan = lastFrameFixturePan.value(fixtureKey, 0);
                    for (float angle = panAngle; angle <= panRange; angle += 360) {
                        const float anglePan = (angle / panRange) * 100;
                        if (std::abs(lastFramePan - anglePan) < std::abs(lastFramePan - pan)) {
                            pan = anglePan;
                        }
                    }
                    pan = std::min<float>(pan, 100);
                    fixturePan[fixtureKey] = pan;
                    float tilt = 50 + (tiltAngle / (tiltRange / 2) * 50);
                    tilt = std::min<float>(tilt, 100);
                    tilt = std::max<float>(tilt, 0);
                    float zoom = (zoomAngle - minZoom) / (maxZoom - minZoom) * 100;
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
                        Q_ASSERT(value <= 100);
                        Q_ASSERT(value >= 0);
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
    emit sendUniverses(dmxUniverses);
    emit updatePreviewFixtures(previewFixtures);
    if (!skipFadeButton->isChecked()) {
        QSqlQuery cuelistQuery;
        if (cuelistQuery.exec("SELECT key FROM cuelists WHERE currentcue_key IS NOT NULL")) {
            while (cuelistQuery.next()) {
                const int cuelistKey = cuelistQuery.value(0).toInt();
                if (cuelistRemainingTransitionFrames.value(cuelistKey, 0) <= 0) {
                    QSqlQuery followQuery;
                    followQuery.prepare("SELECT key, follow FROM cues WHERE cuelist_key = :cuelist AND sortkey = (SELECT MIN(sortkey) FROM cues WHERE cuelist_key = :cuelist AND sortkey > (SELECT cues.sortkey FROM cues, cuelists WHERE cuelists.key = :cuelist AND cuelists.currentcue_key = cues.key))");
                    followQuery.bindValue(":cuelist", cuelistKey);
                    if (followQuery.exec()) {
                        if (followQuery.next() && (followQuery.value(1).toInt() == 1)) {
                            const int followCueKey = followQuery.value(0).toInt();
                            QSqlQuery cueUpdateQuery;
                            cueUpdateQuery.prepare("UPDATE cuelists SET currentcue_key = :cue WHERE key = :cuelist");
                            cueUpdateQuery.bindValue(":cuelist", cuelistKey);
                            cueUpdateQuery.bindValue(":cue", followCueKey);
                            if (!cueUpdateQuery.exec()) {
                                qWarning() << Q_FUNC_INFO << cueUpdateQuery.executedQuery() << cueUpdateQuery.lastError().text();
                            }
                            emit dbChanged();
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << followQuery.executedQuery() << followQuery.lastError().text();
                    }
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        }
    }
}

void DmxEngine::renderCue(const int cueKey, const QList<int> groupKeys, const QHash<int, QSet<int>> groupFixtureKeys, QHash<int, QHash<int, int>> oldGroupEffectFrames, QHash<int, float>* fixtureIntensities, QHash<int, ColorData>* fixtureColors, QHash<int, PositionData>* fixturePositions, QHash<int, QHash<int, RawChannelData>>* fixtureRaws) {
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
                    if (dimmer >= fixtureIntensities->value(fixtureKey, 0)) {
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
                const QHash<int, RawChannelData> raws = getFixtureRaws(fixtureKey, rawKeys);
                if (fixtureRaws->contains(fixtureKey)) {
                    for (const int channel : raws.keys()) {
                        (*fixtureRaws)[fixtureKey][channel] = raws.value(channel);
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
                const int effectKey = effectQuery.value(0).toInt();
                effectKeys.append(effectKey);
                if (!groupEffectFrames.contains(groupKey)) {
                    groupEffectFrames[groupKey] = QHash<int, int>();
                }
                if (oldGroupEffectFrames.value(groupKey).contains(effectKey)) {
                    groupEffectFrames[groupKey][effectKey] = oldGroupEffectFrames.value(groupKey).value(effectKey) + 1;
                } else {
                    groupEffectFrames[groupKey][effectKey] = 1;
                }
            }
            for (const int fixtureKey : groupFixtureKeys.value(groupKey)) {
                bool intensityInformation = false;
                float intensity = 0;
                bool colorInformation = false;
                ColorData color;
                bool positionInformation = false;
                PositionData position;
                QHash<int, RawChannelData> raws;
                getFixtureEffects(fixtureKey, effectKeys, groupEffectFrames.value(groupKey), &intensityInformation, &intensity, &colorInformation, &color, &positionInformation, &position, &raws);
                if (intensityInformation && (intensity > fixtureIntensities->value(fixtureKey, 0))) {
                    (*fixtureIntensities)[fixtureKey] = intensity;
                }
                if (colorInformation) {
                    (*fixtureColors)[fixtureKey] = color;
                }
                if (positionInformation) {
                    (*fixturePositions)[fixtureKey] = position;
                }
                if (!raws.isEmpty()) {
                    if (fixtureRaws->contains(fixtureKey)) {
                        for (const int channel : raws.keys()) {
                            (*fixtureRaws)[fixtureKey][channel] = raws.value(channel);
                        }
                    } else {
                        (*fixtureRaws)[fixtureKey] = raws;
                    }
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
    if (fixtureExceptionQuery.exec()) {
        if (fixtureExceptionQuery.next()) {
            return fixtureExceptionQuery.value(0).toFloat();
        }
    } else {
        qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
    }
    QSqlQuery modelExceptionQuery;
    modelExceptionQuery.prepare("SELECT " + modelExceptionTable + ".value FROM " + modelExceptionTable + ", fixtures WHERE " + modelExceptionTable + ".item_key = :item AND " + modelExceptionTable + ".foreignitem_key = fixtures.model_key AND fixtures.key = :fixture");
    modelExceptionQuery.bindValue(":item", itemKey);
    modelExceptionQuery.bindValue(":fixture", fixtureKey);
    if (modelExceptionQuery.exec()) {
        if (modelExceptionQuery.next()) {
            return modelExceptionQuery.value(0).toFloat();
        }
    } else {
        qWarning() << Q_FUNC_INFO << modelExceptionQuery.executedQuery() << modelExceptionQuery.lastError().text();
    }
    QSqlQuery itemQuery;
    itemQuery.prepare("SELECT " + itemTableAttribute + " FROM " + itemTable + " WHERE key = :item");
    itemQuery.bindValue(":item", itemKey);
    if (itemQuery.exec()) {
        if (itemQuery.next()) {
            return itemQuery.value(0).toFloat();
        } else {
            qWarning() << Q_FUNC_INFO << itemQuery.executedQuery() << itemTable + " Item with key " + QString::number(itemKey) + " should exist but wasn't found!";
        }
    } else {
        qWarning() << Q_FUNC_INFO << itemQuery.executedQuery() << itemQuery.lastError().text();
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
    const float h = (hue / 60);
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
        color.green = 100;
        color.blue = p;
    } else if (i == 2) {
        color.red = p;
        color.green = 100;
        color.blue = t;
    } else if (i == 3) {
        color.red = p;
        color.green = q;
        color.blue = 100;
    } else if (i == 4) {
        color.red = t;
        color.green = p;
        color.blue = 100;
    } else if (i == 5) {
        color.red = 100;
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

QHash<int, DmxEngine::RawChannelData> DmxEngine::getFixtureRaws(const int fixtureKey, const QList<int> rawKeys) {
    QHash<int, RawChannelData> channels;
    for (const int rawKey : rawKeys) {
        bool fading = false;
        bool moveWhileDark = false;
        QSqlQuery rawAttributesQuery;
        rawAttributesQuery.prepare("SELECT fade, movewhiledark FROM raws WHERE key = :raw");
        rawAttributesQuery.bindValue(":raw", rawKey);
        if (rawAttributesQuery.exec()) {
            if (rawAttributesQuery.next()) {
                fading = (rawAttributesQuery.value(0).toInt() == 1);
                moveWhileDark = (rawAttributesQuery.value(1).toInt() == 1);
            } else {
                qWarning() << Q_FUNC_INFO << rawAttributesQuery.executedQuery() << "Raw with key " + QString::number(rawKey) + " should exist but wasn't found!";
            }
        } else {
            qWarning() << Q_FUNC_INFO << rawAttributesQuery.executedQuery() << rawAttributesQuery.lastError().text();
        }
        QSqlQuery itemQuery;
        itemQuery.prepare("SELECT key, value FROM raw_channel_values WHERE item_key = :raw");
        itemQuery.bindValue(":raw", rawKey);
        if (itemQuery.exec()) {
            while (itemQuery.next()) {
                const int channel = itemQuery.value(0).toInt();
                if (!channels.contains(channel)) {
                    channels[channel] = RawChannelData();
                }
                channels[channel].value = itemQuery.value(1).toUInt();
                channels[channel].fading = fading;
                channels[channel].moveWhileDark = moveWhileDark;
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
                const int channel = modelExceptionQuery.value(0).toInt();
                if (!channels.contains(channel)) {
                    channels[channel] = RawChannelData();
                }
                channels[channel].value = (uint8_t)modelExceptionQuery.value(1).toUInt();
                channels[channel].fading = fading;
                channels[channel].moveWhileDark = moveWhileDark;
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
                const int channel = fixtureExceptionQuery.value(0).toInt();
                if (!channels.contains(channel)) {
                    channels[channel] = RawChannelData();
                }
                channels[channel].value = (uint8_t)fixtureExceptionQuery.value(1).toUInt();
                channels[channel].fading = fading;
                channels[channel].moveWhileDark = moveWhileDark;
            }
        } else {
            qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
        }
    }
    return channels;
}

void DmxEngine::getFixtureEffects(const int fixtureKey, const QList<int> effectKeys, const QHash<int, int> effectFrames, bool* intensityInformation, float* dimmer, bool* colorInformation, ColorData* color, bool* positionInformation, PositionData* position, QHash<int, RawChannelData>* raws) {
    for (const int effectKey : effectKeys) {
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
                    int frames = (int)(effectFrames.value(effectKey, 0) + (phase / 360) * totalFrames) % totalFrames;
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
                        (*intensityInformation) = true;
                        float currentDimmer = 0;
                        if (stepIntensityKeys.contains(currentStep)) {
                            currentDimmer = getFixtureIntensity(fixtureKey, stepIntensityKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            float lastDimmer = 0;
                            if (stepIntensityKeys.contains(lastStep)) {
                                lastDimmer = getFixtureIntensity(fixtureKey, stepIntensityKeys.value(lastStep));
                            }
                            currentDimmer += (lastDimmer - currentDimmer) * fade;
                        }
                        if (currentDimmer >= (*dimmer)) {
                            (*dimmer) = currentDimmer;
                        }
                    }
                    if (!stepColorKeys.isEmpty()) {
                        (*colorInformation) = true;
                        ColorData currentColor;
                        if (stepColorKeys.contains(currentStep)) {
                            currentColor = getFixtureColor(fixtureKey, stepColorKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            ColorData lastColor;
                            if (stepColorKeys.contains(lastStep)) {
                                lastColor = getFixtureColor(fixtureKey, stepColorKeys.value(lastStep));
                            }
                            currentColor.red += (lastColor.red - currentColor.red) * fade;
                            currentColor.green += (lastColor.green - currentColor.green) * fade;
                            currentColor.blue += (lastColor.blue - currentColor.blue) * fade;
                            currentColor.quality += (lastColor.quality - currentColor.quality) * fade;
                        }
                        (*color) = currentColor;
                    }
                    if (!stepPositionKeys.isEmpty()) {
                        (*positionInformation) = true;
                        PositionData currentPosition;
                        if (stepPositionKeys.contains(currentStep)) {
                            currentPosition = getFixturePosition(fixtureKey, stepPositionKeys.value(currentStep));
                        }
                        if (fade > 0) {
                            PositionData lastPosition;
                            if (stepPositionKeys.contains(lastStep)) {
                                lastPosition = getFixturePosition(fixtureKey, stepPositionKeys.value(lastStep));
                            }
                            if (std::abs(currentPosition.pan - lastPosition.pan) > 180) {
                                if (lastPosition.pan > currentPosition.pan) {
                                    currentPosition.pan += 360;
                                } else {
                                    lastPosition.pan += 360;
                                }
                            }
                            currentPosition.pan += (lastPosition.pan - currentPosition.pan) * fade;
                            while (currentPosition.pan >= 360) {
                                currentPosition.pan -= 360;
                            }
                            currentPosition.tilt += (lastPosition.tilt - currentPosition.tilt) * fade;
                            currentPosition.zoom += (lastPosition.zoom - currentPosition.zoom) * fade;
                            currentPosition.focus += (lastPosition.focus - currentPosition.focus) * fade;
                        }
                        (*position) = currentPosition;
                    }
                    if (!stepRawKeys.isEmpty()) {
                        QHash<int, RawChannelData> currentRaws;
                        QHash<int, RawChannelData> lastRaws;
                        for (int step = 1; step <= stepAmount; step++) {
                            if (stepRawKeys.contains(step)) {
                                const QHash<int, RawChannelData> stepRaws = getFixtureRaws(fixtureKey, stepRawKeys.value(step));
                                for (const int channel : stepRaws.keys()) {
                                    if (step == currentStep) {
                                        currentRaws[channel] = stepRaws.value(channel);
                                    } else if (!currentRaws.contains(channel)) {
                                        currentRaws[channel] = RawChannelData();
                                    }
                                }
                                if (step == lastStep) {
                                    lastRaws = stepRaws;
                                }
                            }
                        }
                        if (fade > 0) {
                            for (const int channel : currentRaws.keys()) {
                                if (currentRaws.value(channel).fading) {
                                    RawChannelData channelData = currentRaws.value(channel);
                                    channelData.value += (lastRaws.value(channel, RawChannelData()).value - currentRaws.value(channel, RawChannelData()).value) * fade;
                                    currentRaws[channel] = channelData;
                                }
                            }
                        }
                        for (const int channel : currentRaws.keys()) {
                            (*raws)[channel] = currentRaws.value(channel);
                        }
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
