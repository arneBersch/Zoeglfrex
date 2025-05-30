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

int GroupModel::rowCount(const QModelIndex&) const
{
    if (kernel->cuelistView->currentCuelist != nullptr) {
        return kernel->cuelistView->currentCuelist->cues->rowCount();
    }
    return 0;
}

int GroupModel::columnCount(const QModelIndex&) const
{
    return 6;
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
        return QVariant();
    }
    if (kernel->cuelistView->currentCuelist == nullptr) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
        Cue *cue = kernel->cuelistView->currentCuelist->cues->items[row];
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
        } else if (column == GroupModelColumns::position) {
            if (cue->positions.contains(kernel->cuelistView->currentGroup)) {
                return cue->positions.value(kernel->cuelistView->currentGroup)->name();
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
        } else if (column == GroupModelColumns::effects) {
            if (cue->effects.contains(kernel->cuelistView->currentGroup)) {
                QStringList effects;
                for (Effect* effect : cue->effects.value(kernel->cuelistView->currentGroup)) {
                    effects.append(effect->name());
                }
                return effects.join(" + ");
            }
        } else {
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (kernel->cuelistView->currentCuelist->cues->items[row] == kernel->cuelistView->currentCuelist->currentCue) {
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
    if (kernel->cuelistView->currentCuelist == nullptr) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == GroupModelColumns::cue) {
            return "Cue";
        } else if (column == GroupModelColumns::intensity) {
            return kernel->cuelistView->currentCuelist->cues->INTENSITIESATTRIBUTEID + " Intensity";
        } else if (column == GroupModelColumns::color) {
            return kernel->cuelistView->currentCuelist->cues->COLORSATTRIBUTEID + " Color";
        } else if (column == GroupModelColumns::position) {
            return kernel->cuelistView->currentCuelist->cues->POSITIONSATTRIBUTEID + " Position";
        } else if (column == GroupModelColumns::raws) {
            return kernel->cuelistView->currentCuelist->cues->RAWSATTRIBUTEID + " Raws";
        } else if (column == GroupModelColumns::effects) {
            return kernel->cuelistView->currentCuelist->cues->EFFECTSATTRIBUTEID + " Effects";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
