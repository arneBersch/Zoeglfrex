/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"

#include "valuedata/intensitydata.h"
#include "valuedata/colordata.h"
#include "valuedata/positiondata.h"
#include "valuedata/cuedata.h"

DmxEngine::DmxEngine(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout();
    setLayout(layout);

    highlightButton = new QPushButton("Highlight");
    highlightButton->setCheckable(true);
    connect(highlightButton, &QPushButton::clicked, this, [this] {
        QSettings().setValue("cuelistview/highlight", highlightButton->isChecked());
    });
    highlightButton->setChecked(QSettings().value("cuelistview/highlight", true).toBool());
    layout->addWidget(highlightButton);

    soloButton = new QPushButton("Solo");
    soloButton->setCheckable(true);
    connect(soloButton, &QPushButton::clicked, this, [this] {
        QSettings().setValue("cuelistview/solo", soloButton->isChecked());
    });
    soloButton->setChecked(QSettings().value("cuelistview/solo", true).toBool());
    layout->addWidget(soloButton);

    fadeProgressBar = new QProgressBar();
    fadeProgressBar->setRange(0, 10);
    fadeProgressBar->setValue(5);
    layout->addWidget(fadeProgressBar);

    skipFadeButton = new QPushButton("Skip Fade");
    skipFadeButton->setCheckable(true);
    connect(skipFadeButton, &QPushButton::clicked, this, [this] {
        QSettings().setValue("cuelistview/skipfade", skipFadeButton->isChecked());
    });
    skipFadeButton->setChecked(QSettings().value("cuelistview/skipfade", true).toBool());
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
    QHash<int, int> oldCuelistCurrentCueKeys = cuelistCurrentCueKeys;
    cuelistCurrentCueKeys.clear();
    QHash<int, int> oldCuelistRemainingTransitionFrames = cuelistRemainingTransitionFrames;
    cuelistRemainingTransitionFrames.clear();
    QHash<int, IntensityData> fixtureIntensities;
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

        CueData currentCue = CueData(currentCueKey, groupKeys, groupFixtureKeys);
        CueData lastCue;
        if ((remainingTransitionFrames > 0) && (lastCueKey >= 0)) {
            lastCue = CueData(lastCueKey, groupKeys, groupFixtureKeys);
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
                    fade = 0.5 - (std::cos(M_PI * fade) / 2);
                }
            }
            IntensityData lastIntensity;
            IntensityData currentIntensity;
            if (currentCue.getFixtureIntensities().contains(fixtureKey) && (fade > 0)) {
                lastIntensity = lastCue.getFixtureIntensities().value(fixtureKey);
            } else if (currentCue.getFixtureIntensities().contains(fixtureKey)) {
                currentIntensity = currentCue.getFixtureIntensities().value(fixtureKey);
            }
            currentIntensity.fade(lastIntensity, fade);
            if (fixtureIntensities.contains(fixtureKey)) {
                fixtureIntensities[fixtureKey].merge(currentIntensity);
            } else {
                fixtureIntensities[fixtureKey] = currentIntensity;
            }
            if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                if (lastCue.getFixtureColors().contains(fixtureKey) && (fade > 0)) {
                    ColorData lastColor = lastCue.getFixtureColors().value(fixtureKey);
                    ColorData currentColor = currentCue.getFixtureColors().value(fixtureKey, lastColor);
                    currentColor.fade(lastColor, fade);
                    fixtureColors[fixtureKey] = currentColor;
                    fixtureColorPriorities[fixtureKey] = priority;
                } else if (currentCue.getFixtureColors().contains(fixtureKey)) {
                    fixtureColors[fixtureKey] = currentCue.getFixtureColors().value(fixtureKey);
                    fixtureColorPriorities[fixtureKey] = priority;
                }
            }
            if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                if (lastCue.getFixturePositions().contains(fixtureKey) && (fade > 0)) {
                    PositionData lastPosition = lastCue.getFixturePositions().value(fixtureKey);
                    PositionData currentPosition = currentCue.getFixturePositions().value(fixtureKey, lastPosition);
                    currentPosition.fade(lastPosition, fade);
                    fixturePositions[fixtureKey] = currentPosition;
                    fixturePositionPriorities[fixtureKey] = priority;
                } else if (currentCue.getFixturePositions().contains(fixtureKey)) {
                    fixturePositions[fixtureKey] = currentCue.getFixturePositions().value(fixtureKey);
                    fixturePositionPriorities[fixtureKey] = priority;
                }
            }
            /*QSet<int> rawChannels;
            RawData currentRaws = currentCueFixtureRaws.value(fixtureKey);
            for (const int channel : currentRaws.keys()) {
                rawChannels.insert(channel);
            }
            RawData lastRaws = lastCueFixtureRaws.value(fixtureKey);
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
            }*/
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
        while (mwdCuelistQuery.next()) {
            const int cuelistKey = mwdCuelistQuery.value(0).toInt();
            const int cuelistPriority = mwdCuelistQuery.value(1).toInt();
            const int cuelistCurrentCueKey = mwdCuelistQuery.value(2).toInt();

            QSqlQuery cuesQuery;
            cuesQuery.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey > (SELECT sortkey FROM cues WHERE key = :currentcue) LIMIT 10");
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
    PositionData::nextFrame();
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

                CueData cue = CueData(fixtureCue, fixtureGroups, fixtureGroupFixture);

                if (cue.getFixtureColors().contains(fixtureKey)) {
                    fixtureColors[fixtureKey] = cue.getFixtureColors().value(fixtureKey);
                }
                if (cue.getFixturePositions().contains(fixtureKey)) {
                    fixturePositions[fixtureKey] = cue.getFixturePositions().value(fixtureKey);
                }
                /*if (cue.getFixtureRaws().contains(fixtureKey)) {
                    fixtureChannelRaws[fixtureKey] = QHash<int, uint8_t>();
                    for (const int channel : cueFixtureRaws.value(fixtureKey).keys()) {
                        if (cueFixtureRaws.value(fixtureKey).value(channel).moveWhileDark) {
                            fixtureChannelRaws[fixtureKey][channel] = cueFixtureRaws.value(fixtureKey).value(channel).value;
                        }
                    }
                }*/
            }
        }

        IntensityData intensity = fixtureIntensities.value(fixtureKey, IntensityData());

        ColorData color = fixtureColors.value(fixtureKey, ColorData());
        if (currentFixtureKeys.contains(fixtureKey) && highlightButton->isChecked()) {
            intensity = IntensityData::highlightValue();
            color = ColorData::highlightValue();
        } else if (!currentFixtureKeys.contains(fixtureKey) && soloButton->isChecked()) {
            intensity = IntensityData();
        }

        PositionData position = fixturePositions.value(fixtureKey, PositionData());

        Preview2d::PreviewData previewFixture;
        previewFixture.xPosition = fixtureQuery.value(3).toFloat();
        previewFixture.yPosition = fixtureQuery.value(4).toFloat();
        previewFixture.label = fixtureQuery.value(5).toString();
        previewFixture.color = color.toQColor(intensity.getDimmer());
        previewFixture.pan = position.getPanAngle();
        previewFixture.tilt = position.getTiltAngle();
        previewFixture.zoom = position.getZoomAngle();
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
                        color.dim(intensity.getDimmer());
                    }
                    const bool whiteChannelGiven = channels.contains('W');

                    for (int channel = address; channel < (address + channels.size()); channel++) {
                        QChar channelType = channels.at(channel - address);
                        const bool fine = (channelType != channelType.toUpper());
                        channelType = channelType.toUpper();

                        float value = 0;
                        if (channelType == QChar('D')) { // Dimmer
                            value = intensity.getDimmer();
                        } else if (channelType == QChar('R')) { // Red
                            value = color.getRed(whiteChannelGiven);
                        } else if (channelType == QChar('G')) { // Green
                            value = color.getGreen(whiteChannelGiven);
                        } else if (channelType == QChar('B')) { // Blue
                            value = color.getBlue(whiteChannelGiven);
                        } else if (channelType == QChar('W')) { // White
                            value = color.getWhite();
                        } else if (channelType == QChar('C')) { // Cyan
                            value = color.getCyan(whiteChannelGiven);
                        } else if (channelType == QChar('M')) { // Magenta
                            value = color.getMagenta(whiteChannelGiven);
                        } else if (channelType == QChar('Y')) { // Yellow
                            value = color.getYellow(whiteChannelGiven);
                        } else if (channelType == QChar('H')) { // Hue
                            value = color.getHue();
                        } else if (channelType == QChar('S')) { // Saturation
                            value = color.getSaturation();
                        } else if (channelType == QChar('P')) { // Pan
                            value = position.getPan(fixtureKey, panRange, rotation, invertPan);
                        } else if (channelType == QChar('T')) { // Tilt
                            value = position.getTilt(tiltRange);
                        } else if (channelType == QChar('Z')) { // Zoom
                            value = position.getZoom(minZoom, maxZoom);
                        } else if (channelType == QChar('F')) { // Focus
                            value = position.getFocus();
                        } else if (channelType == QChar('0')) { // DMX 0
                            value = 0;
                        } else if (channelType == QChar('1')) { // DMX 255
                            value = 1;
                        } else {
                            Q_ASSERT(false);
                        }
                        Q_ASSERT(value <= 1);
                        Q_ASSERT(value >= 0);

                        if (channel <= 512) {
                            value *= 65535;
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
