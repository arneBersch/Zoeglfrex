/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "grouplist.h"

GroupList::GroupList(Kernel *core)
{
    kernel = core;
}

bool GroupList::deleteItems(QList<QString> ids) {
    for (QString id : ids) {
        int groupRow = getItemRow(id);
        if (groupRow < 0) {
            kernel->terminal->error("Group can't be deleted because it doesn't exist.");
            return false;
        }
        Group *group = items[groupRow];
        kernel->cues->deleteGroup(group);
        items.removeAt(groupRow);
        delete group;
    }
    return true;
}

void GroupList::deleteFixture(Fixture *fixture) {
    for (Group* group : items) {
        group->fixtures.removeAll(fixture);
    }
}

Group* GroupList::recordGroup(QString id) {
    Group *group = new Group;
    group->id = id;
    group->label = QString();
    group->fixtures = QList<Fixture*>();
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, group);
    return group;
}

bool GroupList::recordGroupFixtures(QList<QString> ids, QList<QString> fixtureIds) {
    QList<Fixture*> fixtureSelection;
    for (QString fixtureId : fixtureIds) {
        Fixture* fixture = kernel->fixtures->getItem(fixtureId);
        if (fixture == nullptr) {
            kernel->terminal->error("Can't record Group Fixtures because Fixture doesn't exist.");
            return false;
        }
        fixtureSelection.append(fixture);
    }
    for (QString id : ids) {
        Group* group = getItem(id);
        if (group == nullptr) {
            group = recordGroup(id);
        }
        group->fixtures = fixtureSelection;
    }
    return true;
}
