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

int GroupList::getItemRow(QString id) const {
    for (int groupRow = 0; groupRow < items.size(); groupRow++) {
        if (items[groupRow]->id == id) {
            return groupRow;
        }
    }
    return -1;
}

bool GroupList::copyItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Group *group = getItem(id);
        if (group == nullptr) {
            kernel->terminal->error("Group can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Group can't be copied because Target ID is already used.");
            return false;
        }
        Group *targetGroup = recordGroup(targetId);
        targetGroup->label = group->label;
        targetGroup->fixtures = group->fixtures;
    }
    return true;
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
        group->fixtures.remove(fixture);
    }
}

bool GroupList::moveItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int groupRow = getItemRow(id);
        if (groupRow < 0) {
            kernel->terminal->error("Group can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Group can't be moved because Target ID is already used.");
            return false;
        }
        Group* group = items[groupRow];
        items.removeAt(groupRow);
        group->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, group);
    }
    return true;
}

Group* GroupList::recordGroup(QString id) {
    Group *group = new Group;
    group->id = id;
    group->label = QString();
    group->fixtures = QSet<Fixture*>();
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
    QSet<Fixture*> fixtureSelection;
    for (QString fixtureId : fixtureIds) {
        Fixture* fixture = kernel->fixtures->getItem(fixtureId);
        if (fixture == nullptr) {
            kernel->terminal->error("Can't record Group Fixtures because Fixture doesn't exist.");
            return false;
        }
        fixtureSelection.insert(fixture);
    }
    for (QString id : ids) {
        Group* group = getItem(id);
        if (group == nullptr) {
            group = recordGroup(id);
        }
        group->fixtures = fixtureSelection;
    }
    return false;
}

QList<QString> GroupList::getIds() const {
    QList<QString> ids;
    for (Group *group : items) {
        ids.append(group->id);
    }
    return ids;
}
