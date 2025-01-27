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

void CueModel::loadCue() {
    beginResetModel();
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
    if (!index.isValid()) {
        return QString();
    }
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
        Group *group = kernel->groups->items[row];
        if (column == CueModelColumns::group) {
            return group->name();
        } else if (column == CueModelColumns::intensity) {
            if (kernel->cuelistView->currentCue != nullptr && kernel->cuelistView->currentCue->intensities.contains(group)) {
                return kernel->cuelistView->currentCue->intensities.value(group)->name();
            }
            return QVariant();
        } else if (column == CueModelColumns::color) {
            if (kernel->cuelistView->currentCue != nullptr && kernel->cuelistView->currentCue->colors.contains(group)) {
                return kernel->cuelistView->currentCue->colors.value(group)->name();
            }
            return QVariant();
        } else {
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (kernel->groups->items[row] == kernel->cuelistView->currentGroup) {
            return QColor(48, 48, 48);
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
