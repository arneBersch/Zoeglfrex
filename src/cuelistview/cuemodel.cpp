/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuemodel.h"
#include "kernel/kernel.h"

CueModel::CueModel(Kernel *core) {
    kernel = core;
}

void CueModel::loadCue(Cue *cue) {
    beginRemoveRows(QModelIndex(), 0, (rows.length() - 1));
    rows.clear();
    endRemoveRows();
    if (cue == nullptr) {
        return;
    }
    currentCue = cue;
    beginInsertRows(QModelIndex(), 0, (kernel->rows->getIds().length() - 1));
    rows = kernel->rows->getIds();
    endInsertRows();
}

int CueModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rows.count();
}

int CueModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant CueModel::data(const QModelIndex &index, const int role) const
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
        QMutexLocker locker(kernel->mutex);
        if (column == CueModelColumns::row) {
            return rows[row];
        } else if (column == CueModelColumns::intensity) {
            Row *currentRow = kernel->rows->getRow(rows[row]);
            if (currentCue->intensities.contains(currentRow)) {
                QString id = (currentCue->intensities[currentRow])->id;
                return id;
            }
            return QVariant();
        } else if (column == CueModelColumns::color) {
            Row *currentRow = kernel->rows->getRow(rows[row]);
            if (currentCue->colors.contains(currentRow)) {
                QString id = (currentCue->colors[currentRow])->id;
                return id;
            }
            return QVariant();
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant CueModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == CueModelColumns::row) {
            return "Row";
        } else if (column == CueModelColumns::intensity) {
            return "Intensity";
        } else if (column == CueModelColumns::color) {
            return "Color";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
