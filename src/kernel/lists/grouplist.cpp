/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "grouplist.h"

GroupList::GroupList(Kernel *core) : ItemList(core, Keys::Group, "Group", "Groups") {}

void GroupList::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute == FIXTURESATTRIBUTEID) {
        bool addFixtures = value.startsWith(Keys::Plus);
        if (addFixtures) {
            value.removeFirst();
        }
        QList<Fixture*> fixtureSelection;
        if (!value.isEmpty() || !text.isEmpty()) {
            if (!value.isEmpty()) {
                if (value.first() != Keys::Fixture) {
                    kernel->terminal->error("Seting Group Fixtures requires Fixtures.");
                    return;
                }
                value.removeFirst();
            }
            QStringList fixtureIds = kernel->terminal->keysToSelection(value, Keys::Fixture);
            if (!text.isEmpty()) {
                fixtureIds = text.split("+");
            }
            if (fixtureIds.isEmpty()) {
                kernel->terminal->error("Can't set Group Fixtures because of an invalid Fixture selection.");
                return;
            }
            for (QString fixtureId : fixtureIds) {
                Fixture* fixture = kernel->fixtures->getItem(fixtureId);
                if (fixture == nullptr) {
                    kernel->terminal->warning("Can't add Fixture " + fixtureId + " to Groups because it doesn't exist.");
                } else {
                    if (!fixtureSelection.contains(fixture)) {
                        fixtureSelection.append(fixture);
                    }
                }
            }
        }
        for (QString id : ids) {
            Group* group = getItem(id);
            if (group == nullptr) {
                group = addItem(id);
            }
            if (addFixtures) {
                for (Fixture* fixture : fixtureSelection) {
                    if (!group->fixtures.contains(fixture)) {
                        group->fixtures.append(fixture);
                    }
                }
            } else {
                group->fixtures = fixtureSelection;
            }
            emit dataChanged(index(getItemRow(group->id), 0), index(getItemRow(group->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        if ((kernel->cuelistView->currentGroup != nullptr) && !kernel->cuelistView->currentGroup->fixtures.contains(kernel->cuelistView->currentFixture)) {
            kernel->cuelistView->currentFixture = nullptr;
        }
        if (ids.size() == 1) {
            kernel->terminal->success("Set Fixtures of Group " + getItem(ids.first())->name() + " to " + QString::number(getItem(ids.first())->fixtures.length()) + " Fixtures.");
        } else {
            if (addFixtures) {
                kernel->terminal->success("Added " + QString::number(fixtureSelection.length()) + " Fixtures to " + QString::number(ids.length()) + " Groups.");
            } else {
                kernel->terminal->success("Set Fixtures of " + QString::number(ids.length()) + " Groups to " + QString::number(fixtureSelection.length()) + " Fixtures.");
            }
        }
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
    }
}
