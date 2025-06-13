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

int GroupModel::rowCount(const QModelIndex&) const {
    return getCueRows().size();
}

int GroupModel::columnCount(const QModelIndex&) const {
    return 6;
}

QVariant GroupModel::data(const QModelIndex &index, const int role) const {
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
    Cue *cue = getCueRows()[row];
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
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
        bool columnValueChanged = false;
        if (getCueRows().indexOf(cue) > 0) {
            Cue* formerCue = getCueRows()[getCueRows().indexOf(cue) - 1];
            columnValueChanged = (((column == CueModelColumns::intensity) && (cue->intensities.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->intensities.value(kernel->cuelistView->currentGroup, nullptr)))
                || ((column == CueModelColumns::color) && (cue->colors.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->colors.value(kernel->cuelistView->currentGroup, nullptr)))
                || ((column == CueModelColumns::position) && (cue->positions.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->positions.value(kernel->cuelistView->currentGroup, nullptr)))
                || ((column == CueModelColumns::raws) && (cue->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>()) != formerCue->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>())))
                || ((column == CueModelColumns::effects) && (cue->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>()) != formerCue->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>()))));
        } else {
            columnValueChanged = (((column == CueModelColumns::intensity) && cue->intensities.contains(kernel->cuelistView->currentGroup))
                || ((column == CueModelColumns::color) && cue->colors.contains(kernel->cuelistView->currentGroup))
                || ((column == CueModelColumns::position) && cue->positions.contains(kernel->cuelistView->currentGroup))
                || ((column == CueModelColumns::raws) && cue->raws.contains(kernel->cuelistView->currentGroup))
                || ((column == CueModelColumns::effects) && cue->effects.contains(kernel->cuelistView->currentGroup)));
        }
        if (columnValueChanged) {
            return QColor(48, 0, 0);
        }
        if (cue == kernel->cuelistView->selectedCue()) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

QVariant GroupModel::headerData(int column, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == GroupModelColumns::cue) {
            return "Cue";
        } else if (column == GroupModelColumns::intensity) {
            return kernel->CUEINTENSITIESATTRIBUTEID + " Intensity";
        } else if (column == GroupModelColumns::color) {
            return kernel->CUECOLORSATTRIBUTEID + " Color";
        } else if (column == GroupModelColumns::position) {
            return kernel->CUEPOSITIONSATTRIBUTEID + " Position";
        } else if (column == GroupModelColumns::raws) {
            return kernel->CUERAWSATTRIBUTEID + " Raws";
        } else if (column == GroupModelColumns::effects) {
            return kernel->CUEEFFECTSATTRIBUTEID + " Effects";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QList<Cue*> GroupModel::getCueRows() const {
    if (kernel->cuelistView->currentCuelist == nullptr) {
        return QList<Cue*>();
    }
    QList<Cue*> cues;
    if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::noFilter) {
        cues = kernel->cuelistView->currentCuelist->cues->items;
    } else if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::activeRowsFilter) {
        for (Cue* cue : kernel->cuelistView->currentCuelist->cues->items) {
            if (cue->intensities.contains(kernel->cuelistView->currentGroup)
                || cue->colors.contains(kernel->cuelistView->currentGroup)
                || cue->positions.contains(kernel->cuelistView->currentGroup)
                || cue->raws.contains(kernel->cuelistView->currentGroup)
                || cue->effects.contains(kernel->cuelistView->currentGroup)) {
                cues.append(cue);
            }
        }
    } else if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::changedRowsFilter) {
        for (Cue* cue : kernel->cuelistView->currentCuelist->cues->items) {
            if (kernel->cuelistView->currentCuelist->cues->items.indexOf(cue) <= 0) {
                if (cue->intensities.contains(kernel->cuelistView->currentGroup)
                    || cue->colors.contains(kernel->cuelistView->currentGroup)
                    || cue->positions.contains(kernel->cuelistView->currentGroup)
                    || cue->raws.contains(kernel->cuelistView->currentGroup)
                    || cue->effects.contains(kernel->cuelistView->currentGroup)) {
                    cues.append(cue);
                }
            } else {
                Cue* formerCue = kernel->cuelistView->currentCuelist->cues->items[kernel->cuelistView->currentCuelist->cues->items.indexOf(cue) - 1];
                if ((cue->intensities.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->intensities.value(kernel->cuelistView->currentGroup, nullptr))
                    || (cue->colors.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->colors.value(kernel->cuelistView->currentGroup, nullptr))
                    || (cue->positions.value(kernel->cuelistView->currentGroup, nullptr) != formerCue->positions.value(kernel->cuelistView->currentGroup, nullptr))
                    || (cue->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>()) != formerCue->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>()))
                    || (cue->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>()) != formerCue->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>()))) {
                    cues.append(cue);
                }
            }
        }
    } else {
        Q_ASSERT(false);
    }
    return cues;
}
