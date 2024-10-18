/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rowlist.h"

RowList::RowList(Kernel *core)
{
    kernel = core;
}

Row* RowList::getRow(QString id)
{
    int rowRow = getRowRow(id);
    if (rowRow < 0 || rowRow >= rows.size()) {
        return nullptr;
    }
    return rows[rowRow];
}

int RowList::getRowRow(QString id)
{
    for (int rowRow = 0; rowRow < rows.size(); rowRow++) {
        if (rows[rowRow]->id == id) {
            return rowRow;
        }
    }
    return -1;
}

QString RowList::copyRow(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Row* row = getRow(id);
        if (row == nullptr) {
            return "Row can't be copied because it doesn't exist.";
        }
        if (getRow(targetId) != nullptr) {
            return "Row can't be copied because Target ID is already used.";
        }
        Row *targetRow = recordRow(targetId, row->group);
        targetRow->label = row->label;
        targetRow->group = row->group;
    }
    return QString();
}

QString RowList::deleteRow(QList<QString> ids)
{
    for (QString id : ids) {
        int rowRow = getRowRow(id);
        if (rowRow < 0) {
            return "Row can't be deleted because it doesn't exist.";
        }
        Row *row = rows[rowRow];
        kernel->cues->deleteRow(row);
        rows.removeAt(rowRow);
        delete row;
    }
    return QString();
}

void RowList::deleteGroup(Group *group) {
    QList<QString> invalidRows;
    for (Row* row : rows) {
        if (row->group == group) {
            invalidRows.append(group->id);
        }
    }
    deleteRow(invalidRows);
}

QString RowList::labelRow(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Row* row = getRow(id);
        if (row == nullptr) {
            return "Row can't be labeled because it doesn't exist.";
        }
        row->label = label;
    }
    return QString();
}

QString RowList::moveRow(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int rowRow = getRowRow(id);
        if (rowRow < 0) {
            return "Row can't be moved because it doesn't exist.";
        }
        if (getRow(targetId) != nullptr) {
            return "Row can't be moved because Target ID is already used.";
        }
        Row* row = rows[rowRow];
        rows.removeAt(rowRow);
        row->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < rows.size(); index++) {
            QList<QString> indexIdParts = (rows[index]->id).split(".");
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
        rows.insert(position, row);
    }
    return QString();
}

Row* RowList::recordRow(QString id, Group *group)
{
    Row *row = new Row;
    row->id = id;
    row->label = QString();
    row->group = group;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < rows.size(); index++) {
        QList<QString> indexIdParts = (rows[index]->id).split(".");
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
    rows.insert(position, row);
    return row;
}

QString RowList::recordRowGroup(QList<QString> ids, QString groupId)
{
    Group* group = kernel->groups->getGroup(groupId);
    if (group == nullptr) {
        return "Row can't be recorded because Group doesn't exist.";
    }
    for (QString id : ids) {
        Row* row = getRow(id);
        if (row == nullptr) {
            row = recordRow(id, group);
        }
        row->group = group;
    }
    return QString();
}

QList<QString> RowList::getIds() {
    QList<QString> ids;
    for (Row *row : rows) {
        ids.append(row->id);
    }
    return ids;
}

int RowList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rows.size();
}

int RowList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant RowList::data(const QModelIndex &index, const int role) const
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
        if (column == RowListColumns::id) {
            return rows[row]->id;
        } else if (column == RowListColumns::label) {
            return rows[row]->label;
        } else if (column == RowListColumns::group) {
            return (rows[row]->group)->label;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant RowList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == RowListColumns::id) {
            return "ID";
        } else if (column == RowListColumns::label) {
            return "Label";
        } else if (column == RowListColumns::group) {
            return "Group";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
