/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "engine.h"

Engine::Engine(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex Engine");

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &Engine::generateDmx);
    timer->start(25);
}

void Engine::generateDmx() {
    QSqlQuery cuelistQuery;
    if (!cuelistQuery.exec("SELECT currentcue_key, priority FROM cuelists ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QMap<int, int> fixtureIntensities;
    QMap<int, int> fixtureIntensityPriorities;
    QMap<int, int> fixtureColors;
    QMap<int, int> fixtureColorPriorities;
    QMap<int, int> fixturePositions;
    QMap<int, int> fixturePositionPriorities;
    while (cuelistQuery.next()) {
        const int cueKey = cuelistQuery.value(0).toInt();
        const int priority = cuelistQuery.value(1).toInt();
        getCurrentCueItems(cueKey, priority, "cue_group_intensities", &fixtureIntensities, &fixtureIntensityPriorities);
        getCurrentCueItems(cueKey, priority, "cue_group_colors", &fixtureColors, &fixtureColorPriorities);
        getCurrentCueItems(cueKey, priority, "cue_group_positions", &fixturePositions, &fixturePositionPriorities);
    }
    QMap<int, QByteArray> dmxUniverses;
    QSqlQuery fixtureQuery;
    if (!fixtureQuery.exec("SELECT key, universe, address FROM fixtures")) {
        qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        return;
    }
    while (fixtureQuery.next()) {
        const int fixtureKey = fixtureQuery.value(0).toInt();
        const int universe = fixtureQuery.value(1).toInt();
        const int address = fixtureQuery.value(2).toInt();
        float dimmer = 0;
        if (fixtureIntensities.contains(fixtureKey)) {
            dimmer = getFixtureValue(fixtureKey, fixtureIntensities.value(fixtureKey), "intensities", "dimmer", "intensity_model_dimmer", "intensity_fixture_dimmer");
        }
        float red = 100;
        float green = 100;
        float blue = 100;
        if (fixtureColors.contains(fixtureKey)) {
            const float hue = getFixtureValue(fixtureKey, fixtureColors.value(fixtureKey), "colors", "hue", "color_model_hue", "color_fixture_hue");
            const float saturation = getFixtureValue(fixtureKey, fixtureColors.value(fixtureKey), "colors", "saturation", "color_model_saturation", "color_fixture_saturation");
            const float h = (hue / 60.0);
            const int i = (int)h;
            const float f = h - i;
            const float p = (100 - saturation);
            const float q = (100 - (saturation * f));
            const float t = (100 - (saturation * (1 - f)));
            if (i == 0) {
                red = 100.0;
                green = t;
                blue = p;
            } else if (i == 1) {
                red = q;
                green = 100.0;
                blue = p;
            } else if (i == 2) {
                red = p;
                green = 100.0;
                blue = t;
            } else if (i == 3) {
                red = p;
                green = q;
                blue = 100.0;
            } else if (i == 4) {
                red = t;
                green = p;
                blue = 100.0;
            } else if (i == 5) {
                red = 100.0;
                green = p;
                blue = q;
            }
        }
        float pan = 0;
        float tilt = 0;
        float zoom = 0;
        float focus = 0;
        if (fixturePositions.contains(fixtureKey)) {
            pan = getFixtureValue(fixtureKey, fixturePositions.value(fixtureKey), "positions", "pan", "position_model_pan", "position_fixture_pan");
            tilt = getFixtureValue(fixtureKey, fixturePositions.value(fixtureKey), "positions", "tilt", "position_model_tilt", "position_fixture_tilt");
            zoom = getFixtureValue(fixtureKey, fixturePositions.value(fixtureKey), "positions", "zoom", "position_model_zoom", "position_fixture_zoom");
            focus = getFixtureValue(fixtureKey, fixturePositions.value(fixtureKey), "positions", "focus", "position_model_focus", "position_fixture_focus");
        }
        if (address > 0) {
            QSqlQuery channelsQuery;
            channelsQuery.prepare("SELECT models.channels FROM fixtures, models WHERE fixtures.key = :key AND fixtures.model_key = models.key");
            channelsQuery.bindValue(":key", fixtureKey);
            if (channelsQuery.exec()) {
                if (channelsQuery.next()) {
                    const QString channels = channelsQuery.value(0).toString();
                    if (!dmxUniverses.contains(universe)) {
                        dmxUniverses[universe] = QByteArray(512, 0);
                    }
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
                        } else if (channelType == QChar('C')) { // Cyan
                            value = (100 - red);
                        } else if (channelType == QChar('M')) { // Magenta
                            value = (100 - green);
                        } else if (channelType == QChar('Y')) { // Yellow
                            value = (100 - blue);
                        } else if (channelType == QChar('P')) { // Pan
                        } else if (channelType == QChar('T')) { // Tilt
                        } else if (channelType == QChar('Z')) { // Zoom
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
                qWarning() << Q_FUNC_INFO << channelsQuery.executedQuery() << channelsQuery.lastError().text();
            }
        }
    }
    for (const int universe : dmxUniverses.keys()) {
        emit sendUniverse(universe, dmxUniverses.value(universe));
    }
}

void Engine::getCurrentCueItems(const int cueId, const int priority, const QString table, QMap<int, int>* fixtureItemKeys, QMap<int, int>* fixtureItemPriorities) {
    QSqlQuery query;
    query.prepare("SELECT group_fixtures.valueitem_key, " + table + ".valueitem_key FROM group_fixtures, groups, " + table + " WHERE group_fixtures.item_key = groups.key AND " + table + ".foreignitem_key = groups.key AND " + table + ".item_key = :cue ORDER BY groups.sortkey");
    query.bindValue(":cue", cueId);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return;
    }
    while (query.next()) {
        const int fixtureKey = query.value(0).toInt();
        if (priority >= fixtureItemPriorities->value(fixtureKey, 0)) {
            (*fixtureItemKeys)[fixtureKey] = query.value(1).toInt();
            (*fixtureItemPriorities)[fixtureKey] = priority;
        }
    }
}

float Engine::getFixtureValue(const int fixtureKey, const int itemKey, const QString itemTable, const QString itemTableAttribute, const QString modelExceptionTable, const QString fixtureExceptionTable) {
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
    Q_ASSERT(false);
    return 0;
}
