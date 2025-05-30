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

int CueModel::rowCount(const QModelIndex&) const
{
    return kernel->groups->rowCount();
}

int CueModel::columnCount(const QModelIndex&) const
{
    return 6;
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
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->intensities.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->intensities.value(group)->name();
            }
            return QVariant();
        } else if (column == CueModelColumns::color) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->colors.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->colors.value(group)->name();
            }
            return QVariant();
        } else if (column == CueModelColumns::position) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->positions.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->positions.value(group)->name();
            }
            return QVariant();
        } else if (column == CueModelColumns::raws) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->raws.contains(group)) {
                QStringList raws;
                for (Raw* raw : kernel->cuelistView->currentCuelist->currentCue->raws.value(group)) {
                    raws.append(raw->name());
                }
                return raws.join(" + ");
            }
        } else if (column == CueModelColumns::effects) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->effects.contains(group)) {
                QStringList effects;
                for (Effect* effect : kernel->cuelistView->currentCuelist->currentCue->effects.value(group)) {
                    effects.append(effect->name());
                }
                return effects.join(" + ");
            }
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
        } else if (kernel->cuelistView->currentCuelist != nullptr) {
            if (column == CueModelColumns::intensity) {
                return kernel->cuelistView->currentCuelist->cues->INTENSITIESATTRIBUTEID + " Intensity";
            } else if (column == CueModelColumns::color) {
                return kernel->cuelistView->currentCuelist->cues->COLORSATTRIBUTEID + " Color";
            } else if (column == CueModelColumns::position) {
                return kernel->cuelistView->currentCuelist->cues->POSITIONSATTRIBUTEID + " Position";
            } else if (column == CueModelColumns::raws) {
                return kernel->cuelistView->currentCuelist->cues->RAWSATTRIBUTEID + " Raws";
            } else if (column == CueModelColumns::effects) {
                return kernel->cuelistView->currentCuelist->cues->EFFECTSATTRIBUTEID + " Effects";
            } else {
                return QVariant();
            }
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
