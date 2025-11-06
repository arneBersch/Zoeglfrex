/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"

DmxEngine::DmxEngine() {
    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &DmxEngine::generateDmx);
    timer->start(25);
}

void DmxEngine::generateDmx() {
    QSqlQuery cuelistQuery;
    if (!cuelistQuery.exec("SELECT currentcue_key, priority FROM cuelists WHERE currentcue_key IS NOT NULL ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QMap<int, float> fixtureIntensities;
    QMap<int, int> fixtureIntensityPriorities;
    QMap<int, ColorData> fixtureColors;
    QMap<int, int> fixtureColorPriorities;
    QMap<int, PositionData> fixturePositions;
    QMap<int, int> fixturePositionPriorities;
    while (cuelistQuery.next()) {
        const int cueKey = cuelistQuery.value(0).toInt();
        const int priority = cuelistQuery.value(1).toInt();
        const QMap<int, int> fixtureIntensityKeys = getCurrentCueItems(cueKey, "cue_group_intensities");
        for (const int fixtureKey : fixtureIntensityKeys.keys()) {
            if (priority >= fixtureIntensityPriorities.value(fixtureKey, 0)) {
                fixtureIntensityPriorities[fixtureKey] = priority;
                const float dimmer = getFixtureValue(fixtureKey, fixtureIntensityKeys.value(fixtureKey), "intensities", "dimmer", "intensity_model_dimmer", "intensity_fixture_dimmer");
                fixtureIntensities[fixtureKey] = dimmer;
            }
        }
        const QMap<int, int> fixtureColorKeys = getCurrentCueItems(cueKey, "cue_group_colors");
        for (const int fixtureKey : fixtureColorKeys.keys()) {
            if (priority >= fixtureColorPriorities.value(fixtureKey, 0)) {
                fixtureColorPriorities[fixtureKey] = priority;
                const int colorKey = fixtureColorKeys.value(fixtureKey);
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
                color.quality = getFixtureValue(fixtureKey, fixtureColorKeys.value(fixtureKey), "colors", "quality", "color_model_quality", "color_fixture_quality");
                fixtureColors[fixtureKey] = color;
            }
        }
        const QMap<int, int> fixturePositionKeys = getCurrentCueItems(cueKey, "cue_group_positions");
        for (const int fixtureKey : fixturePositionKeys.keys()) {
            if (priority >= fixturePositionPriorities.value(fixtureKey, 0)) {
                fixturePositionPriorities[fixtureKey] = priority;
                const int positionKey = fixturePositionKeys.value(fixtureKey);
                PositionData position;
                position.pan = getFixtureValue(fixtureKey, positionKey, "positions", "pan", "position_model_pan", "position_fixture_pan");
                position.tilt = getFixtureValue(fixtureKey, positionKey, "positions", "tilt", "position_model_tilt", "position_fixture_tilt");
                position.zoom = getFixtureValue(fixtureKey, positionKey, "positions", "zoom", "position_model_zoom", "position_fixture_zoom");
                position.focus = getFixtureValue(fixtureKey, positionKey, "positions", "focus", "position_model_focus", "position_fixture_focus");
                fixturePositions[fixtureKey] = position;
            }
        }
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
        previewFixture.color = QColor(red * 2.55, green * 2.55, blue * 2.55, dimmer * 2.55);
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
