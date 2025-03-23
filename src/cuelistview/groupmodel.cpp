/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "groupmodel.h"
#include "kernel/kernel.h"

GroupModel::GroupModel(Kernel *core) {
    kernel = core;
}

void GroupModel::loadGroup() {
    beginResetModel();
    endResetModel();
}

int GroupModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return kernel->cues->rowCount();
}

int GroupModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant GroupModel::data(const QModelIndex &index, const int role) const
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
        Cue *cue = kernel->cues->items[row];
        if (column == GroupModelColumns::cue) {
            return cue->name();
        } else if (column == GroupModelColumns::intensity) {
            if (cue->intensities.contains(kernel->cuelistView->currentGroup)) {
                return cue->intensities.value(kernel->cuelistView->currentGroup)->name();
            }
            return QVariant();
        } else if (column == GroupModelColumns::color) {
            if (cue->colors.contains(kernel->cuelistView->currentGroup)) {
                return cue->colors.value(kernel->cuelistView->currentGroup)->name();
            }
            return QVariant();
        } else if (column == GroupModelColumns::raws) {
            if (cue->raws.contains(kernel->cuelistView->currentGroup)) {
                QStringList raws;
                for (Raw* raw : cue->raws.value(kernel->cuelistView->currentGroup)) {
                    raws.append(raw->name());
                }
                return raws.join(" + ");
            }
            }
        } else {
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (kernel->cues->items[row] == kernel->cuelistView->currentCue) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

QVariant GroupModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == GroupModelColumns::cue) {
            return "Cue";
        } else if (column == GroupModelColumns::intensity) {
            return kernel->cues->INTENSITIESATTRIBUTEID + " Intensity";
        } else if (column == GroupModelColumns::color) {
            return kernel->cues->COLORSATTRIBUTEID + " Color";
        } else if (column == GroupModelColumns::raws) {
            return kernel->cues->RAWSATTRIBUTEID + " Raws";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
