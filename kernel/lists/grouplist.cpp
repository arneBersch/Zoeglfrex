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

Group* GroupList::getGroup(QString id)
{
    int groupRow = getGroupRow(id);
    if (groupRow < 0 || groupRow >= groups.size()) {
        return nullptr;
    }
    return groups[groupRow];
}

int GroupList::getGroupRow(QString id)
{
    for (int groupRow = 0; groupRow < groups.size(); groupRow++) {
        if (groups[groupRow]->id == id) {
            return groupRow;
        }
    }
    return -1;
}

QString GroupList::copyGroup(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Group *group = getGroup(id);
        if (group == nullptr) {
            return "Group can't be copied because it doesn't exist.";
        }
        if (getGroup(targetId) != nullptr) {
            return "Group can't be copied because Target ID is already used.";
        }
        Group *targetGroup = recordGroup(targetId);
        targetGroup->label = group->label;
        targetGroup->fixtures = group->fixtures;
    }
    return QString();
}

QString GroupList::deleteGroup(QList<QString> ids)
{
    for (QString id : ids) {
        int groupRow = getGroupRow(id);
        if (groupRow < 0) {
            return "Group can't be deleted because it doesn't exist.";
        }
        Group *group = groups[groupRow];
        kernel->rows->deleteGroup(group);
        groups.removeAt(groupRow);
        delete group;
    }
    return QString();
}

void GroupList::deleteFixture(Fixture *fixture)
{
    for (Group* group : groups) {
        group->fixtures.remove(fixture);
    }
}

QString GroupList::labelGroup(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Group* group = getGroup(id);
        if (group == nullptr) {
            return "Group can't be labeled because it doesn't exist.";
        }
        group->label = label;
    }
    return QString();
}

QString GroupList::moveGroup(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int groupRow = getGroupRow(id);
        if (groupRow < 0) {
            return "Group can't be moved because it doesn't exist.";
        }
        if (getGroup(targetId) != nullptr) {
            return "Group can't be moved because Target ID is already used.";
        }
        Group* group = groups[groupRow];
        groups.removeAt(groupRow);
        group->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < groups.size(); index++) {
            QList<QString> indexIdParts = (groups[index]->id).split(".");
            if (indexIdParts[0].toInt() < idParts[0].toInt()) {
                position++;
            } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
                if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                    position++;
                } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                    if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                        position++;
                    } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                        if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                            position++;
                        } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                            if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                                position++;
                            }
                        }
                    }
                }
            }
        }
        groups.insert(position, group);
    }
    return QString();
}

Group* GroupList::recordGroup(QString id)
{
    Group *group = new Group;
    group->id = id;
    group->label = QString();
    group->fixtures = QSet<Fixture*>();
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < groups.size(); index++) {
        QList<QString> indexIdParts = (groups[index]->id).split(".");
        if (indexIdParts[0].toInt() < idParts[0].toInt()) {
            position++;
        } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
            if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                position++;
            } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                    position++;
                } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                    if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                        position++;
                    } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                        if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                            position++;
                        }
                    }
                }
            }
        }
    }
    groups.insert(position, group);
    return group;
}

QString GroupList::recordGroupFixtures(QList<QString> ids, QList<QString> fixtureIds)
{
    QSet<Fixture*> fixtureSelection;
    for (QString fixtureId : fixtureIds) {
        Fixture* fixture = kernel->fixtures->getFixture(fixtureId);
        if (fixture == nullptr) {
            return "Can't record Group Fixtures because Fixture doesn't exist.";
        }
        fixtureSelection.insert(fixture);
    }
    for (QString id : ids) {
        Group* group = getGroup(id);
        if (group == nullptr) {
            group = recordGroup(id);
        }
        group->fixtures = fixtureSelection;
    }
    return QString();
}

QList<QString> GroupList::getIds() {
    QList<QString> ids;
    for (Group *group : groups) {
        ids.append(group->id);
    }
    return ids;
}

int GroupList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return groups.size();
}

int GroupList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant GroupList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == GroupListColumns::id) {
            return groups[row]->id;
        } else if (column == GroupListColumns::label) {
            return groups[row]->label;
        } else if (column == GroupListColumns::fixtures) {
            QString text;
            QSet<Fixture*> fixtures = groups[row]->fixtures;
            for (Fixture* fixture : fixtures) {
                text += fixture->id;
                text += ", ";
            }
            return text;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant GroupList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == GroupListColumns::id) {
            return "ID";
        } else if (column == GroupListColumns::label) {
            return "Label";
        } else if (column == GroupListColumns::fixtures) {
            return "Fixtures";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
