/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturelist.h"

FixtureList::FixtureList(Kernel *core)
{
    kernel = core;
}

Fixture* FixtureList::getFixture(QString id)
{
    int fixtureRow = getFixtureRow(id);
    if (fixtureRow < 0 || fixtureRow >= fixtures.size()) {
        return nullptr;
    }
    return fixtures[fixtureRow];
}

int FixtureList::getFixtureRow(QString id)
{
    for (int fixtureRow = 0; fixtureRow < fixtures.size(); fixtureRow++) {
        if (fixtures[fixtureRow]->id == id) {
            return fixtureRow;
        }
    }
    return -1;
}

QString FixtureList::copyFixture(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            return "Fixture can't be copied because it doesn't exist.";
        }
        if (getFixture(targetId) != nullptr) {
            return "Fixture can't be copied because Target ID is already used.";
        }
        Fixture *targetFixture = recordFixture(targetId, fixture->model);
        if (targetFixture == nullptr) {
            return "Fixture can't be copied because no free address was found.";
        }
        targetFixture->label = fixture->label;
        targetFixture->model = fixture->model;
    }
    return QString();
}

QString FixtureList::deleteFixture(QList<QString> ids)
{
    for (QString id : ids) {
        int fixtureRow = getFixtureRow(id);
        if (fixtureRow < 0) {
            return "Fixture can't be deleted because it doesn't exist.";
        }
        Fixture *fixture = fixtures[fixtureRow];
        kernel->groups->deleteFixture(fixture);
        fixtures.removeAt(fixtureRow);
        delete fixture;
    }
    return QString();
}

void FixtureList::deleteModel(Model *model)
{
    QList<QString> invalidFixtures;
    for (Fixture* fixture : fixtures) {
        if (fixture->model == model) {
            invalidFixtures.append(fixture->id);
        }
    }
    deleteFixture(invalidFixtures);
}

QString FixtureList::labelFixture(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            return tr("Fixture %1 can't be labeled because it doesn't exist.").arg(id);
        }
        fixture->label = label;
    }
    return QString();
}

QString FixtureList::moveFixture(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int fixtureRow = getFixtureRow(id);
        if (fixtureRow < 0) {
            return tr("Fixture %1 can't be moved because it doesn't exist.").arg(id);
        }
        if (getFixture(targetId) != nullptr) {
            return tr("Fixture %1 can't be moved because Target ID is already used.").arg(id);
        }
        Fixture* fixture = fixtures[fixtureRow];
        fixtures.removeAt(fixtureRow);
        fixture->id = targetId;
        int position = 0;
        for (int index=0; index < fixtures.size(); index++) {
            if (greaterId(fixtures[index]->id, targetId)) {
                position++;
            }
        }
        fixtures.insert(position, fixture);
    }
    return QString();
}

Fixture* FixtureList::recordFixture(QString id, Model* model)
{
    Fixture *fixture = new Fixture;
    fixture->id = id;
    fixture->label = QString();
    int address = findFreeAddress(model->channels.size());
    if (address <= 0) {
        return nullptr;
    }
    fixture->address = address;
    fixture->model = model;
    int position = 0;
    for (int index=0; index < fixtures.size(); index++) {
        if (greaterId(fixtures[index]->id, id)) {
            position++;
        }
    }
    fixtures.insert(position, fixture);
    return fixture;
}

QString FixtureList::recordFixtureAddress(QList<QString> ids, int address)
{
    QSet<int> fixtureChannels;
    for (QString id : ids) {
        Fixture *fixture = getFixture(id);
        if (fixture == nullptr) {
            return tr("Fixture can't record address because Fixture %1 doesn't exist.").arg(id);
        }
        for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.length()); channel++) {
            fixtureChannels.insert(channel);
        }
    }
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        for (int channel = address; channel < (address + fixture->model->channels.length()); channel++) {
            if (channel < 1 || channel > 512) {
                return tr("Can't record Fixture Address because address %1 is out of range.").arg(channel);
            }
            if (usedChannels().contains(channel) && !fixtureChannels.contains(channel)) {
                return tr("Can't record Fixture Address because channel %1 is already used.").arg(channel);
            }
        }
        fixture->address = address;
        address += fixture->model->channels.length();
    }
    return QString();
}

QString FixtureList::recordFixtureModel(QList<QString> ids, QString modelId, int address)
{
    Model *model = kernel->models->getModel(modelId);
    if (model == nullptr) {
        return tr("Can't record Fixture Model because Model %1 doesn't exist.").arg(modelId);
    }
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            fixture = recordFixture(id, model);
            if (fixture == nullptr) {
                return "Can't record Fixture because no free DMX address was found.";
            }
        }
        fixture->model = model;
        if (address > 0) {
            QList<QString> fixtureId;
            fixtureId.append(id);
            QString result = recordFixtureAddress(fixtureId, address);
            if (!result.isEmpty()) {
                return result;
            }
            address += model->channels.length();
        }
    }
    return QString();
}

QList<QString> FixtureList::getIds() {
    QList<QString> ids;
    for (Fixture *fixture : fixtures) {
        ids.append(fixture->id);
    }
    return ids;
}

int FixtureList::findFreeAddress(int channelCount) {
    int lastUsedChannel = 0;
    for (int channel=1; channel<=512; channel++) {
        if (usedChannels().contains(channel)) {
            lastUsedChannel = channel;
        } else {
            if ((channel - lastUsedChannel) >= channelCount) {
                return (lastUsedChannel + 1);
            }
        }
    }
    return 0;
}

QSet<int> FixtureList::usedChannels() {
    QSet<int> channels;
    for (Fixture* fixture : fixtures) {
        for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.size()); channel++) {
            channels.insert(channel);
        }
    }
    return channels;
}

int FixtureList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fixtures.size();
}

QVariant FixtureList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        return fixtures[row]->id;
    }
    return QVariant();
}
