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
    Group *group = kernel->groups->items[row];
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
        if (column == CueModelColumns::group) {
            return group->name();
        } else if (column == CueModelColumns::intensity) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->intensities.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->intensities.value(group)->name();
            }
        } else if (column == CueModelColumns::color) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->colors.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->colors.value(group)->name();
            }
        } else if (column == CueModelColumns::position) {
            if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->positions.contains(group)) {
                return kernel->cuelistView->currentCuelist->currentCue->positions.value(group)->name();
            }
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
        Cue* formerCue = nullptr;
        if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && (kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->currentCuelist->currentCue->id) > 0)) {
            formerCue = kernel->cuelistView->currentCuelist->cues->items[kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->currentCuelist->currentCue->id) - 1];
        }
        if ((column == CueModelColumns::intensity) && (formerCue != nullptr) && (kernel->cuelistView->currentCuelist->currentCue->intensities.value(group, nullptr) != formerCue->intensities.value(group, nullptr))) {
            return QColor(48, 0, 0);
        } else if ((column == CueModelColumns::color) && (formerCue != nullptr) && (kernel->cuelistView->currentCuelist->currentCue->colors.value(group, nullptr) != formerCue->colors.value(group, nullptr))) {
            return QColor(48, 0, 0);
        } else if ((column == CueModelColumns::position) && (formerCue != nullptr) && (kernel->cuelistView->currentCuelist->currentCue->positions.value(group, nullptr) != formerCue->positions.value(group, nullptr))) {
            return QColor(48, 0, 0);
        } else if ((column == CueModelColumns::raws) && (formerCue != nullptr) && (kernel->cuelistView->currentCuelist->currentCue->raws.value(group, QList<Raw*>()) != formerCue->raws.value(group, QList<Raw*>()))) {
            return QColor(48, 0, 0);
        } else if ((column == CueModelColumns::effects) && (formerCue != nullptr) && (kernel->cuelistView->currentCuelist->currentCue->effects.value(group, QList<Effect*>()) != formerCue->effects.value(group, QList<Effect*>()))) {
            return QColor(48, 0, 0);
        }
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
            return kernel->CUEINTENSITIESATTRIBUTEID + " Intensity";
        } else if (column == CueModelColumns::color) {
            return kernel->CUECOLORSATTRIBUTEID + " Color";
        } else if (column == CueModelColumns::position) {
            return kernel->CUEPOSITIONSATTRIBUTEID + " Position";
        } else if (column == CueModelColumns::raws) {
            return kernel->CUERAWSATTRIBUTEID + " Raws";
        } else if (column == CueModelColumns::effects) {
            return kernel->CUEEFFECTSATTRIBUTEID + " Effects";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
