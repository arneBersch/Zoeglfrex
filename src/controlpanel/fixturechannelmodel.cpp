/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturechannelmodel.h"

FixtureChannelModel::FixtureChannelModel() {}

int FixtureChannelModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return channels.length();
}

int FixtureChannelModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 8;
}

QVariant FixtureChannelModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() > channels.length()) {
        return QVariant();
    }
    ChannelData channel = channels.at(index.row());
    const int column = index.column();
    if (role == Qt::DisplayRole) {
        QString queryText;
        if (column == 0) {
            return channel.title;
        } else if (column == 1) {
            return channel.value;
        } else if (column == 2) {
            return channel.modelValue;
        } else if (column == 3) {
            return channel.fixtureValue;
        }
    }
    return QVariant();
}

QVariant FixtureChannelModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (section == 0) {
            return "Channel";
        } else if (section == 1) {
            return "Value";
        } else if (section == 2) {
            return "Model Value";
        } else if (section == 3) {
            return "Fixture Value";
        }
    }
    return QVariant();
}

void FixtureChannelModel::setRawKey(const int raw) {
    beginResetModel();

    modelKey = -1;
    fixtureKey = -1;
    rawKey = raw;
    QString channelsString = QString();
    QSqlQuery query;
    if (query.exec("SELECT models.key, fixtures.key, models.channels FROM models, fixtures, currentitems WHERE currentitems.fixture_key = fixtures.key AND fixtures.model_key = models.key")) {
        if (query.next()) {
            modelKey = query.value(0).toInt();
            fixtureKey = query.value(1).toInt();
            channelsString = query.value(2).toString();
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }

    channels.clear();
    for (int i = 0; i < channelsString.length(); i++) {
        QChar channelChar = channelsString.at(i);
        ChannelData channel;
        channel.title = QString::number(i + 1) + ". ";
        const bool fine = (channelChar != channelChar.toUpper());
        channelChar = channelChar.toUpper();
        if (channelChar == QChar('D')) {
            channel.title = "Dimmer";
        } else if (channelChar == QChar('R')) {
            channel.title = "Red";
        } else if (channelChar == QChar('G')) {
            channel.title = "Green";
        } else if (channelChar == QChar('B')) {
            channel.title = "Blue";
        } else if (channelChar == QChar('W')) {
            channel.title = "White";
        } else if (channelChar == QChar('C')) {
            channel.title = "Cyan";
        } else if (channelChar == QChar('M')) {
            channel.title = "Magenta";
        } else if (channelChar == QChar('Y')) {
            channel.title = "Yellow";
        } else if (channelChar == QChar('P')) {
            channel.title = "Pan";
        } else if (channelChar == QChar('T')) {
            channel.title = "Tilt";
        } else if (channelChar == QChar('Z')) {
            channel.title = "Zoom";
        } else if (channelChar == QChar('F')) {
            channel.title = "Focus";
        } else if (channelChar == QChar('0')) {
            channel.title = "DMX 0";
        } else if (channelChar == QChar('1')) {
            channel.title = "DMX 255";
        } else {
            Q_ASSERT(false);
        }
        if (fine) {
            channel.title.append(" fine");
        }
        channels.append(channel);
    }

    if ((rawKey >= 0) && (modelKey >= 0)) {
        QSqlQuery valueQuery;
        valueQuery.prepare("SELECT key, value FROM raw_channel_values WHERE item_key = :raw");
        valueQuery.bindValue(":raw", rawKey);
        if (valueQuery.exec()) {
            while (valueQuery.next()) {
                const int channel = valueQuery.value(0).toInt();
                const uint8_t value = valueQuery.value(1).toInt();
                if (channel <= channels.length()) {
                    channels[channel - 1].valueGiven = true;
                    channels[channel - 1].value = value;
                    channels[channel - 1].modelValue = value;
                    channels[channel - 1].fixtureValue = value;
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << valueQuery.executedQuery() << valueQuery.lastError().text();
        }

        QSqlQuery modelQuery;
        modelQuery.prepare("SELECT key, value FROM raw_model_channel_values WHERE item_key = :raw AND foreignitem_key = :model");
        modelQuery.bindValue(":raw", rawKey);
        modelQuery.bindValue(":model", modelKey);
        if (modelQuery.exec()) {
            while (modelQuery.next()) {
                const int channel = modelQuery.value(0).toInt();
                const uint8_t value = modelQuery.value(1).toInt();
                if (channel <= channels.length()) {
                    channels[channel - 1].modelValueGiven = true;
                    channels[channel - 1].modelValue = value;
                    channels[channel - 1].fixtureValue = value;
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << modelQuery.executedQuery() << modelQuery.lastError().text();
        }

        QSqlQuery fixtureQuery;
        fixtureQuery.prepare("SELECT key, value FROM raw_fixture_channel_values WHERE item_key = :raw AND foreignitem_key = :fixture");
        fixtureQuery.bindValue(":raw", rawKey);
        fixtureQuery.bindValue(":fixture", fixtureKey);
        if (fixtureQuery.exec()) {
            while (fixtureQuery.next()) {
                const int channel = fixtureQuery.value(0).toInt();
                const uint8_t value = fixtureQuery.value(1).toInt();
                if (channel <= channels.length()) {
                    channels[channel - 1].fixtureValueGiven = true;
                    channels[channel - 1].fixtureValue = value;
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        }
    }

    endResetModel();
}

void FixtureChannelModel::setChannelDifference(const int channel, int difference) {
    const ChannelData channelData = channels.at(channel - 1);
    QSqlQuery query;
    if (channelData.fixtureValueGiven) {
        Q_ASSERT(fixtureKey >= 0);
        query.prepare("INSERT OR REPLACE INTO raw_fixture_channel_values (item_key, key, foreignitem_key, value) VALUES (:raw, :channel, :fixture, :value)");
        query.bindValue(":fixture", fixtureKey);
        difference += channelData.fixtureValue;
    } else if (channelData.modelValueGiven) {
        Q_ASSERT(modelKey >= 0);
        query.prepare("INSERT OR REPLACE INTO raw_model_channel_values (item_key, key, foreignitem_key, value) VALUES (:raw, :channel, :model, :value)");
        query.bindValue(":model", modelKey);
        difference += channelData.modelValue;
    } else if (channelData.valueGiven) {
        query.prepare("INSERT OR REPLACE INTO raw_channel_values (item_key, key, value) VALUES (:raw, :channel, :value)");
        difference += channelData.value;
    } else {
        return;
    }
    difference = std::min(difference, 255);
    difference = std::max(difference, 0);
    query.bindValue(":value", difference);
    query.bindValue(":raw", rawKey);
    query.bindValue(":channel", channel);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}

void FixtureChannelModel::updateButtons(const int channel, QPushButton* valueButton, QPushButton* modelButton, QPushButton* fixtureButton) {
    Q_ASSERT(channel <= channels.length());
    const ChannelData channelData = channels.at(channel - 1);
    valueButton->setText(QString::number(channelData.value));
    valueButton->setCheckable(true);
    valueButton->setChecked(channelData.valueGiven);
    connect(valueButton, &QPushButton::clicked, this, [this, channel, valueButton] {
        QSqlQuery query;
        if (valueButton->isChecked()) {
            query.prepare("INSERT OR REPLACE INTO raw_channel_values (item_key, key) VALUES (:raw, :channel)");
        } else {
            query.prepare("DELETE FROM raw_channel_values WHERE item_key = :raw AND key = :channel");
        }
        query.bindValue(":raw", rawKey);
        query.bindValue(":channel", channel);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
    modelButton->setText(QString::number(channelData.modelValue));
    modelButton->setCheckable(true);
    modelButton->setChecked(channelData.modelValueGiven);
    connect(modelButton, &QPushButton::clicked, this, [this, channel, modelButton] {
        Q_ASSERT(modelKey >= 0);
        QSqlQuery query;
        if (modelButton->isChecked()) {
            query.prepare("INSERT OR REPLACE INTO raw_model_channel_values (item_key, key, foreignitem_key) VALUES (:raw, :channel, :model)");
        } else {
            query.prepare("DELETE FROM raw_model_channel_values WHERE item_key = :raw AND foreignitem_key = :model AND key = :channel");
        }
        query.bindValue(":model", modelKey);
        query.bindValue(":raw", rawKey);
        query.bindValue(":channel", channel);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
    fixtureButton->setText(QString::number(channelData.fixtureValue));
    fixtureButton->setCheckable(true);
    fixtureButton->setChecked(channelData.fixtureValueGiven);
    connect(fixtureButton, &QPushButton::clicked, this, [this, channel, fixtureButton] {
        Q_ASSERT(fixtureKey >= 0);
        QSqlQuery query;
        if (fixtureButton->isChecked()) {
            query.prepare("INSERT OR REPLACE INTO raw_fixture_channel_values (item_key, key, foreignitem_key) VALUES (:raw, :channel, :fixture)");
        } else {
            query.prepare("DELETE FROM raw_fixture_channel_values WHERE item_key = :raw AND foreignitem_key = :fixture AND key = :channel");
        }
        query.bindValue(":fixture", fixtureKey);
        query.bindValue(":raw", rawKey);
        query.bindValue(":channel", channel);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
}
