/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "fixturetable.h"

FixtureTable::FixtureTable(Kernel *core) : ItemTable(core, Keys::Fixture, "Fixture", "Fixtures") {
    intAttributes[kernel->FIXTUREADDRESSATTRIBUTEID] = {"Address", 0, 0, 512};
    intAttributes[kernel->FIXTUREUNIVERSEATTRIBUTEID] = {"Universe", 1, 1, 63999};
    floatAttributes[kernel->FIXTUREPOSITIONXATTRIBUTEID] = {"X Position", 0, -100, 100};
    floatAttributes[kernel->FIXTUREPOSITIONYATTRIBUTEID] = {"Y Position", 0, -100, 100};
    angleAttributes[kernel->FIXTUREROTATIONATTRIBUTEID] = {"Rotation", 0};
    boolAttributes[kernel->FIXTUREINVERTPANATTRIBUTE] = {"Invert Pan", false};
}

void FixtureTable::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == kernel->FIXTUREMODELATTRIBUTEID) {
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                Fixture* fixture = getItem(id);
                if (fixture == nullptr) {
                    fixture = addItem(id);
                }
                fixture->model = nullptr;
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Model of Fixture " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed Model of " + QString::number(ids.size()) + " Fixtures.");
            }
        } else if (((value.size() >= 2) && (value.first() == Keys::Model)) || !text.isEmpty()) {
            if (!value.isEmpty()) {
                value.removeFirst();
            }
            QString modelId = kernel->terminal->keysToId(value);
            if (!text.isEmpty()) {
                modelId = text;
            }
            Model *model = kernel->models->getItem(modelId);
            if (model == nullptr) {
                kernel->terminal->error("Can't set Model of Fixtures because Model " + modelId + " doesn't exist.");
                return;
            }
            for (QString id : ids) {
                Fixture* fixture = getItem(id);
                if (fixture == nullptr) {
                    fixture = addItem(id);
                }
                fixture->model = model;
                emit dataChanged(index(getItemRow(fixture->id), 0), index(getItemRow(fixture->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set Model of Fixture " + getItem(ids.first())->name() + " to Model " + model->name() + ".");
            } else {
                kernel->terminal->success("Set Model of " + QString::number(ids.size()) + " Fixtures to Model " + model->name() + ".");
            }
        } else {
            kernel->terminal->error("Can' set Fixture Model because an invalid value was given.");
            return;
        }
    } else {
        ItemTable::setAttribute(ids, attributes, value, text);
    }
}
