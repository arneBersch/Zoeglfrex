/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelistmodel.h"
#include "kernel/kernel.h"

CuelistModel::CuelistModel(Kernel* core) {
    kernel = core;
}

int CuelistModel::rowCount(const QModelIndex&) const {
    return kernel->cuelists->items.size();
}

int CuelistModel::columnCount(const QModelIndex&) const {
    return 5;
}

QVariant CuelistModel::data(const QModelIndex &index, const int role) const {
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    }
    Cuelist *cuelist = kernel->cuelists->items[row];
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
        if (column == CuelistModelColumns::cuelistName) {
            return cuelist->name();
        } else if (column == CuelistModelColumns::currentCue) {
            if (cuelist->currentCue != nullptr) {
                return cuelist->currentCue->name();
            }
            return "No Cue selected";
            return QVariant();
        } else {
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (cuelist == kernel->cuelistView->currentCuelist) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

QVariant CuelistModel::headerData(int column, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == CuelistModelColumns::cuelistName) {
            return "Cuelist";
        } else if (column == CuelistModelColumns::currentCue) {
            return "Cue";
        } else if (column == CuelistModelColumns::dimmer) {
            return "Dimmer";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

void CuelistModel::reset() {
    beginResetModel();
    endResetModel();
}
