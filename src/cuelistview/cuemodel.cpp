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
    if (cue == nullptr) {
        return;
    }
    beginResetModel();
    currentCue = cue;
    endResetModel();
}

int CueModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return kernel->groups->rowCount();
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
        if (column == CueModelColumns::group) {
            return kernel->groups->items[row]->name();
        } else if (column == CueModelColumns::intensity) {
            Group *currentGroup = kernel->groups->items[row];
            if (currentCue != nullptr && currentCue->intensities.contains(currentGroup)) {
                return (currentCue->intensities[currentGroup])->name();
            }
            return QVariant();
        } else if (column == CueModelColumns::color) {
            Group *currentGroup = kernel->groups->items[row];
            if (currentCue != nullptr && currentCue->colors.contains(currentGroup)) {
                return (currentCue->colors[currentGroup])->name();
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
        if (column == CueModelColumns::group) {
            return "Group";
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
