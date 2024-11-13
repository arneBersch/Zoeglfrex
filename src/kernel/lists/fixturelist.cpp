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

void FixtureList::recordFixtureAddress(QList<QString> ids, int address) {
    if (address > 512) {
        kernel->terminal->error("Can't record Fixtures because address is greate than 512.");
        return;
    }
    QList<Fixture*> fixtures;
    for (QString id : ids) {
        Fixture *fixture = getItem(id);
        if (fixture == nullptr) {
            kernel->terminal->warning("Can't record Fixture " + id + " because it doesn't exist.");
        } else {
            fixtures.append(fixture);
        }
    }
    QMap<Fixture*, int> oldAddresses = QMap<Fixture*, int>();
    for (Fixture* fixture : fixtures) {
        oldAddresses[fixture] = fixture->address;
        if (address > 0) {
            fixture->address = address;
            address += fixture->model->channels.length();
        } else {
            fixture->address = 0;
        }
    }
    if (!channelsOkay()) {
        for (Fixture* fixture : fixtures) {
            fixture->address = oldAddresses[fixture];
        }
        kernel->terminal->error("Didn't record Fixture Adresses because this would have resulted in an DMX address conflict.");
        return;
    }
    for (Fixture* fixture : fixtures) {
        emit dataChanged(index(getItemRow(fixture->id), 0), index(getItemRow(fixture->id), 0), {Qt::DisplayRole, Qt::EditRole});
    }
    kernel->terminal->success("Recorded address of " + QString::number(fixtures.length()) + " Fixtures.");
}

void FixtureList::recordFixtureModel(QList<QString> ids, QString modelId) {
    Model *model = kernel->models->getItem(modelId);
    if (model == nullptr) {
        kernel->terminal->error("Can't record Fixture Model because Model " + modelId + " doesn't exist.");
        return;
    }
    int fixtureCounter = 0;
    for (QString id : ids) {
        Fixture* fixture = getItem(id);
        Model* oldModel = nullptr;
        if (fixture == nullptr) {
            fixture = recordItem(id);
        } else {
            oldModel = fixture->model;
        }
        fixture->model = model;
        if (channelsOkay()) {
            emit dataChanged(index(getItemRow(fixture->id), 0), index(getItemRow(fixture->id), 0), {Qt::DisplayRole, Qt::EditRole});
            fixtureCounter++;
        } else {
            fixture->model = oldModel; // don't change model if this would result in an address conflict
        }
    }
    kernel->terminal->success("Recorded " + QString::number(fixtureCounter) + " Fixtures to Model " + model->name() + ".");
}

bool FixtureList::channelsOkay() {
    QSet<int> channels;
    for (Fixture* fixture : items) {
        if (fixture->address > 0) {
            for (int channel = fixture->address; channel < (fixture->address + fixture->model->channels.size()); channel++) {
                if (channel > 512) {
                    kernel->terminal->warning("Fixture " + fixture->name() + " would have channels greater than 512.");
                    return false;
                }
                if (channels.contains(channel)) {
                    kernel->terminal->warning("Channel " + QString::number(channel) + " would be used twice.");
                    return false;
                }
                channels.insert(channel);
            }
        }
    }
    return true;
}
