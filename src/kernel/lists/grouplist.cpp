/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "grouplist.h"

GroupList::GroupList(Kernel *core) : ItemList(Keys::Group, "Group", "Groups") {
    kernel = core;
}

void GroupList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attributeString = attributes.value(Keys::Attribute);
    if (attributeString == FIXTURESATTRIBUTEID) {
        QList<Fixture*> fixtureSelection;
        if (!value.isEmpty()) {
            if (value[0] != Keys::Fixture) {
                kernel->terminal->error("Seting Group Fixtures requires Fixtures.");
                return;
            }
            value.removeFirst();
            QList<QString> fixtureIds = kernel->keysToSelection(value, Keys::Fixture);
            if (fixtureIds.isEmpty()) {
                kernel->terminal->error("Can't set Group Fixtures because of an invalid Fixture selection.");
                return;
            }
            for (QString fixtureId : fixtureIds) {
                Fixture* fixture = kernel->fixtures->getItem(fixtureId);
                if (fixture == nullptr) {
                    kernel->terminal->warning("Can't add Fixture " + fixtureId + " to Group because it doesn't exist.");
                } else {
                    fixtureSelection.append(fixture);
                }
            }
        }
        for (QString id : ids) {
            Group* group = getItem(id);
            if (group == nullptr) {
                group = addItem(id);
            }
            group->fixtures = fixtureSelection;
            emit dataChanged(index(getItemRow(group->id), 0), index(getItemRow(group->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Fixtures of " + QString::number(ids.length()) + " Groups to " + QString::number(fixtureSelection.length()) + " Fixtures.");
    } else {
        kernel->terminal->error("Can't set Group Attribute " + attributeString + ".");
    }
}
