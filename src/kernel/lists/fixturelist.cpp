/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturelist.h"

FixtureList::FixtureList(Kernel *core) : ItemList(Keys::Fixture, "Fixture", "Fixtures") {
    kernel = core;
}

void FixtureList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attributeString = attributes.value(Keys::Attribute);
    if (attributeString == MODELATTRIBUTEID) {
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Fixture* fixture = getItem(id);
                if (fixture == nullptr) {
                    fixture = addItem(id);
                }
                fixture->model = nullptr;
            }
            kernel->terminal->success("Set Model of " + QString::number(ids.size()) + " Fixtures to None (Dimmer).");
        } else if ((value.size() >= 2) && (value.first() == Keys::Model)) {
            value.removeFirst();
            QString modelId = kernel->keysToId(value);
            Model *model = kernel->models->getItem(modelId);
            if (model == nullptr) {
                kernel->terminal->error("Can't set Model of Fixtures because Model " + modelId + " doesn't exist.");
                return;
            }
            int fixtureCounter = 0;
            for (QString id : ids) {
                Fixture* fixture = getItem(id);
                Model* oldModel = nullptr;
                if (fixture == nullptr) {
                    fixture = addItem(id);
                } else {
                    oldModel = fixture->model;
                }
                fixture->model = model;
                if (channelsOkay()) {
                    emit dataChanged(index(getItemRow(fixture->id), 0), index(getItemRow(fixture->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    fixtureCounter++;
                } else {
                    fixture->model = oldModel; // don't change Model if this would result in an address conflict
                    kernel->terminal->warning("Can't set Model of Fixture " + id + " because this would result in an address conflict.");
                }
            }
            kernel->terminal->success("Set Model of " + QString::number(fixtureCounter) + " Fixtures to Model " + model->name() + ".");
        } else {
            kernel->terminal->error("Can' set Fixture Model because an invalid value was given.");
            return;
        }
    } else if (attributeString == ADDRESSATTRIBUTEID) {
        int address = kernel->keysToValue(value);
        if ((address < 0) || (address > 512)) {
            kernel->terminal->error("Can't set Fixture Address because Address has to be a valid number between 0 and 512.");
            return;
        }
        QList<Fixture*> fixtures;
        for (QString id : ids) {
            Fixture *fixture = getItem(id);
            if (fixture == nullptr) {
                fixture = addItem(id);
            }
            fixtures.append(fixture);
        }
        QMap<Fixture*, int> oldAddresses = QMap<Fixture*, int>();
        for (Fixture* fixture : fixtures) {
            oldAddresses[fixture] = fixture->address;
            if (address > 0) {
                fixture->address = address;
                if (fixture->model == nullptr) {
                    address += 1;
                } else {
                    address += fixture->model->channels.length();
                }
            } else {
                fixture->address = 0;
            }
        }
        if (!channelsOkay()) {
            for (Fixture* fixture : fixtures) {
                fixture->address = oldAddresses[fixture];
            }
            kernel->terminal->error("Didn't set Fixture Adresses because this would have resulted in a DMX address conflict.");
            return;
        }
        for (Fixture* fixture : fixtures) {
            emit dataChanged(index(getItemRow(fixture->id), 0), index(getItemRow(fixture->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Address of " + QString::number(fixtures.length()) + " Fixtures.");
    } else {
        kernel->terminal->error("Can't set Fixture Attribute " + attributeString + ".");
    }
}

    bool FixtureList::channelsOkay() {
        QSet<int> channels;
        for (Fixture* fixture : items) {
            if (fixture->address > 0) {
                int fixtureChannels = 1;
                if (fixture->model != nullptr) {
                    fixtureChannels = fixture->model->channels.size();
                }
                for (int channel = fixture->address; channel < (fixture->address + fixtureChannels); channel++) {
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
