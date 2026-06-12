/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawdata.h"

RawData::RawData() {}

RawData::RawData(const int fixtureKey, const QList<int> rawKeys, const bool renderMwD) {
    for (const int rawKey : rawKeys) {
        bool fading = false;
        bool moveWhileDark = false;
        QSqlQuery rawAttributesQuery;
        rawAttributesQuery.prepare("SELECT fade, movewhiledark FROM raws WHERE key = :raw");
        rawAttributesQuery.bindValue(":raw", rawKey);
        if (rawAttributesQuery.exec()) {
            if (rawAttributesQuery.next()) {
                fading = rawAttributesQuery.value(0).toInt() == 1;
                moveWhileDark = rawAttributesQuery.value(1).toInt() == 1;
            } else {
                qWarning() << Q_FUNC_INFO << rawAttributesQuery.executedQuery() << "Raw with key " + QString::number(rawKey) + " should exist but wasn't found!";
            }
        } else {
            qWarning() << Q_FUNC_INFO << rawAttributesQuery.executedQuery() << rawAttributesQuery.lastError().text();
        }

        if (!renderMwD || moveWhileDark) {
            QSqlQuery itemQuery;
            itemQuery.prepare("SELECT key, value FROM raw_channel_values WHERE item_key = :raw");
            itemQuery.bindValue(":raw", rawKey);
            if (itemQuery.exec()) {
                while (itemQuery.next()) {
                    setChannel(itemQuery.value(0).toInt(), itemQuery.value(1).toUInt(), fading);
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
                    setChannel(modelExceptionQuery.value(0).toInt(), modelExceptionQuery.value(1).toUInt(), fading);
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
                    setChannel(fixtureExceptionQuery.value(0).toInt(), fixtureExceptionQuery.value(1).toUInt(), fading);
                }
            } else {
                qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
            }
        }
    }
}

void RawData::setChannel(const int channel, const uint8_t value, const bool fading) {
    if (!channels.contains(channel)) {
        channels[channel] = RawChannelData();
    }

    channels[channel].value = value;
    channels[channel].fading = fading;
}

void RawData::merge(const RawData raws) {
    for (const int channel : raws.channels.keys()) {
        channels[channel] = raws.channels.value(channel);
    }
}

void RawData::fade(const RawData oldRaws, const float fade) {
    for (const int channel : oldRaws.channels.keys()) {
        const RawChannelData oldData = oldRaws.channels.value(channel);
        if (!channels.contains(channel)) {
            channels[channel] = oldData;
        } else if (channels.value(channel).fading) {
            const uint8_t newValue = channels[channel].value;
            channels[channel].value = fadeValue(oldData.value, newValue, fade);
        }
    }
}

QHash<int, uint8_t> RawData::getChannels() const {
    QHash<int, uint8_t> result;
    for (const int channel : channels.keys()) {
        result[channel] = channels.value(channel).value;
    }
    return result;
}
