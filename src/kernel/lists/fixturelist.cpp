/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturelist.h"

FixtureList::FixtureList(Kernel *core) : ItemList("Fixture", "Fixtures") {
    kernel = core;
}

void FixtureList::deleteModel(Model *model)
{
    QList<QString> invalidFixtures;
    for (Fixture* fixture : items) {
        if (fixture->model == model) {
            invalidFixtures.append(fixture->id);
        }
    }
    deleteItems(invalidFixtures);
}

Fixture* FixtureList::recordFixture(QString id, Model* model) {
    Fixture *fixture = new Fixture(kernel);
    fixture->id = id;
    int address = findFreeAddress(model->channels.size());
    if (address <= 0) {
        return nullptr;
    }
    fixture->address = address;
    fixture->model = model;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    beginInsertRows(QModelIndex(), position, position);
    items.insert(position, fixture);
    endInsertRows();
    return fixture;
}

void FixtureList::recordFixtureAddress(QList<QString> ids, int address) {
    QSet<int> fixtureChannels;
    QList<Fixture*> fixtures;
    for (QString id : ids) {
        Fixture *fixture = getItem(id);
        if (fixture == nullptr) {
            kernel->terminal->warning("Can't record Fixture " + id + " because it doesn't exist.");
        } else {
            fixtures.append(fixture);
            for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.length()); channel++) {
                fixtureChannels.insert(channel);
            }
        }
    }
    int fixtureCounter = 0;
    for (Fixture* fixture : fixtures) {
        bool addressAvailable = true;
        for (int channel = address; channel < (address + fixture->model->channels.length()); channel++) {
            if (channel < 1 || channel > 512 || (usedChannels().contains(channel) && !fixtureChannels.contains(channel))) {
                kernel->terminal->warning("Can't record Fixture Address of Fixture " + fixture->id + " because DMX address " + QString::number(channel) + " is not available.");
                addressAvailable = false;
            }
        }
        if (addressAvailable) {
            fixture->address = address;
            address += fixture->model->channels.length();
            fixtureCounter++;
        }
    }
    kernel->terminal->success("Recorded " + QString::number(fixtureCounter) + " Fixtures.");
}

void FixtureList::recordFixtureModel(QList<QString> ids, QString modelId, int address) {
    Model *model = kernel->models->getItem(modelId);
    if (model == nullptr) {
        kernel->terminal->error("Can't record Fixture Model because Model " + modelId + " doesn't exist.");
        return;
    }
    int fixtureCounter = 0;
    for (QString id : ids) {
        Fixture* fixture = getItem(id);
        if (fixture == nullptr) {
            fixture = recordFixture(id, model);
        }
        if (fixture == nullptr) {
            kernel->terminal->warning("Can't record Fixture " + id + " because no free DMX address was found.");
        } else {
            bool addressConflict = false;
            for (int channel=(address + fixture->model->channels.size()); channel < (address + model->channels.size()); channel++) {
                if (channel > 512) {
                    addressConflict = true;
                }
                if (kernel->fixtures->usedChannels().contains(channel)) {
                    addressConflict = true;
                }
            }
            if (addressConflict) {
                kernel->terminal->warning("Can't record Fixture Model because this would result in an address conflict.");
            } else {
                fixture->model = model;
                if (address > 0) {
                    QList<QString> fixtureId;
                    fixtureId.append(id);
                    recordFixtureAddress(fixtureId, address);
                    if (fixture->address == address) {
                        fixtureCounter++;
                    } else {
                        kernel->terminal->error("Couldn't set the DMX address of Fixture " + id + " to " + QString::number(address));
                    }
                    address += model->channels.length();
                }
            }
        }
    }
    kernel->terminal->success("Recorded " + QString::number(fixtureCounter) + " Fixtures.");
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
    for (Fixture* fixture : items) {
        for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.size()); channel++) {
            channels.insert(channel);
        }
    }
    return channels;
}
