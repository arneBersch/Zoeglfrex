/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "positionlist.h"

PositionList::PositionList() {
    //
}

void PositionList::setLists(CueList *cueList)
{
    cues = cueList;
}

Position* PositionList::getPosition(QString id)
{
    int positionRow = getPositionRow(id);
    if (positionRow < 0 || positionRow >= positions.size()) {
        return nullptr;
    }
    return positions[positionRow];
}

int PositionList::getPositionRow(QString id)
{
    for (int positionRow = 0; positionRow < positions.size(); positionRow++) {
        if (positions[positionRow]->id == id) {
            return positionRow;
        }
    }
    return -1;
}

QString PositionList::copyPosition(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Position* position = getPosition(id);
        if (position == nullptr) {
            return "Position can't be copied because it doesn't exist.";
        }
        if (getPosition(targetId) != nullptr) {
            return "Position can't be copied because Target ID is already used.";
        }
        Position *targetPosition = recordPosition(targetId);
        targetPosition->label = position->label;
        targetPosition->pan = position->pan;
        targetPosition->tilt = position->tilt;
    }
    return QString();
}

QString PositionList::deletePosition(QList<QString> ids)
{
    for (QString id : ids) {
        int positionRow = getPositionRow(id);
        if (positionRow < 0) {
            return "Position can't be deleted because it doesn't exist.";
        }
        Position *position = positions[positionRow];
        cues->deletePosition(position);
        positions.removeAt(positionRow);
        delete position;
    }
    return QString();
}

QString PositionList::labelPosition(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Position* position = getPosition(id);
        if (position == nullptr) {
            return "Position can't be labeled because it doesn't exist.";
        }
        position->label = label;
    }
    return QString();
}

QString PositionList::movePosition(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Position* position = getPosition(id);
        if (position == nullptr) {
            return "Position can't be moved because it doesn't exist.";
        }
        if (getPosition(targetId) != nullptr) {
            return "Position can't be moved because Target ID is already used.";
        }
        position->id = targetId;
    }
    return QString();
}

Position* PositionList::recordPosition(QString id)
{
    Position *position = new Position;
    position->id = id;
    position->label = QString();
    position->pan = 0;
    position->tilt = 0;
    positions.append(position);
    return position;
}

QString PositionList::recordPositionPan(QList<QString> ids, float pan)
{
    if (pan > 180 || pan < -180) {
        return "Record Position Pan only allows from -180° to 180°.";
    }
    for (QString id : ids) {
        Position* position = getPosition(id);
        if (position == nullptr) {
            position = recordPosition(id);
        }
        position->pan = pan;
    }
    return QString();
}

QString PositionList::recordPositionTilt(QList<QString> ids, float tilt)
{
    if (tilt > 90 || tilt < -90) {
        return "Record Position Tilt only allows from -90° to 90°.";
    }
    for (QString id : ids) {
        Position* position = getPosition(id);
        if (position == nullptr) {
            position = recordPosition(id);
        }
        position->tilt = tilt;
    }
    return QString();
}

int PositionList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return positions.size();
}

int PositionList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant PositionList::data(const QModelIndex &index, const int role) const
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
        if (column == PositionListColumns::id) {
            return positions[row]->id;
        } else if (column == PositionListColumns::label) {
            return positions[row]->label;
        } else if (column == PositionListColumns::pan) {
            return positions[row]->pan;
        } else if (column == PositionListColumns::tilt) {
            return positions[row]->tilt;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant PositionList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == PositionListColumns::id) {
            return "ID";
        } else if (column == PositionListColumns::label) {
            return "Label";
        } else if (column == PositionListColumns::pan) {
            return "Pan";
        } else if (column == PositionListColumns::tilt) {
            return "Tilt";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
