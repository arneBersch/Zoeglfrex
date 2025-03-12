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
    intAttributes[ADDRESSATTRIBUTEID] = {"Address", 0, 0, 512};
}

void FixtureList::setAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == MODELATTRIBUTEID) {
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
                if (kernel->patchOkay()) {
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
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
    }
}
