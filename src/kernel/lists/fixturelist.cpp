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

bool FixtureList::copyFixture(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            kernel->terminal->error("Fixture can't be copied because it doesn't exist.");
            return false;
        }
        if (getFixture(targetId) != nullptr) {
            kernel->terminal->error("Fixture can't be copied because Target ID is already used.");
            return false;
        }
        Fixture *targetFixture = recordFixture(targetId, fixture->model);
        if (targetFixture == nullptr) {
            kernel->terminal->error("Fixture can't be copied because no free address was found.");
            return false;
        }
        targetFixture->label = fixture->label;
        targetFixture->model = fixture->model;
    }
    return true;
}

bool FixtureList::deleteFixture(QList<QString> ids)
{
    for (QString id : ids) {
        int fixtureRow = getFixtureRow(id);
        if (fixtureRow < 0) {
            kernel->terminal->error("Fixture can't be deleted because it doesn't exist.");
            return false;
        }
        Fixture *fixture = fixtures[fixtureRow];
        kernel->groups->deleteFixture(fixture);
        fixtures.removeAt(fixtureRow);
        delete fixture;
    }
    return true;
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

bool FixtureList::labelFixture(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            kernel->terminal->error("Fixture " + id + " can't be labeled because it doesn't exist.");
            return false;
        }
        fixture->label = label;
    }
    return true;
}

bool FixtureList::moveFixture(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int fixtureRow = getFixtureRow(id);
        if (fixtureRow < 0) {
            kernel->terminal->error("Fixture " + id + " can't be moved because it doesn't exist.");
            return false;
        }
        if (getFixture(targetId) != nullptr) {
            kernel->terminal->error("Fixture " + id + " can't be moved because Target ID is already used.");
            return false;
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
    return true;
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

bool FixtureList::recordFixtureAddress(QList<QString> ids, int address)
{
    QSet<int> fixtureChannels;
    for (QString id : ids) {
        Fixture *fixture = getFixture(id);
        if (fixture == nullptr) {
            kernel->terminal->error("Fixture can't record address because Fixture " + id + " doesn't exist.");
            return false;
        }
        for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.length()); channel++) {
            fixtureChannels.insert(channel);
        }
    }
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        for (int channel = address; channel < (address + fixture->model->channels.length()); channel++) {
            if (channel < 1 || channel > 512) {
                kernel->terminal->error("Can't record Fixture Address because address " + QString::number(channel) + " is out of range.");
                return false;
            }
            if (usedChannels().contains(channel) && !fixtureChannels.contains(channel)) {
                kernel->terminal->error("Can't record Fixture Address because DMX channel " + QString::number(channel) + " is already used.");
                return false;
            }
        }
        fixture->address = address;
        address += fixture->model->channels.length();
    }
    return true;
}

bool FixtureList::recordFixtureModel(QList<QString> ids, QString modelId, int address)
{
    Model *model = kernel->models->getModel(modelId);
    if (model == nullptr) {
        kernel->terminal->error("Can't record Fixture Model because Model " + modelId + " doesn't exist.");
        return false;
    }
    for (QString id : ids) {
        Fixture* fixture = getFixture(id);
        if (fixture == nullptr) {
            fixture = recordFixture(id, model);
            if (fixture == nullptr) {
                kernel->terminal->error("Can't record Fixture because no free DMX address was found.");
                return false;
            }
        }
        fixture->model = model;
        if (address > 0) {
            QList<QString> fixtureId;
            fixtureId.append(id);
            if (!recordFixtureAddress(fixtureId, address)) {
                return false;
            }
            address += model->channels.length();
        }
    }
    return true;
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
