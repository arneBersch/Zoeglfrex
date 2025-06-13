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

int CueModel::rowCount(const QModelIndex&) const {
    return getGroupRows().length();
}

int CueModel::columnCount(const QModelIndex&) const {
    return 6;
}

QVariant CueModel::data(const QModelIndex &index, const int role) const {
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
    Group *group = getGroupRows()[row];
    if (role == Qt::DisplayRole) {
        QMutexLocker locker(kernel->mutex);
        if (column == CueModelColumns::group) {
            return group->name();
        } else if (column == CueModelColumns::intensity) {
            if ((kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->intensities.contains(group)) {
                return kernel->cuelistView->selectedCue()->intensities.value(group)->name();
            }
        } else if (column == CueModelColumns::color) {
            if ((kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->colors.contains(group)) {
                return kernel->cuelistView->selectedCue()->colors.value(group)->name();
            }
        } else if (column == CueModelColumns::position) {
            if ((kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->positions.contains(group)) {
                return kernel->cuelistView->selectedCue()->positions.value(group)->name();
            }
        } else if (column == CueModelColumns::raws) {
            if ((kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->raws.contains(group)) {
                QStringList raws;
                for (Raw* raw : kernel->cuelistView->selectedCue()->raws.value(group)) {
                    raws.append(raw->name());
                }
                return raws.join(" + ");
            }
        } else if (column == CueModelColumns::effects) {
            if ((kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->effects.contains(group)) {
                QStringList effects;
                for (Effect* effect : kernel->cuelistView->selectedCue()->effects.value(group)) {
                    effects.append(effect->name());
                }
                return effects.join(" + ");
            }
        } else {
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        bool columnValueChanged = false;
        if (kernel->cuelistView->selectedCue() != nullptr) {
            if (kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) > 0) {
                Cue* formerCue = kernel->cuelistView->currentCuelist->cues->items[kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) - 1];
                columnValueChanged = (
                    ((column == CueModelColumns::intensity) && (kernel->cuelistView->selectedCue()->intensities.value(group, nullptr) != formerCue->intensities.value(group, nullptr)))
                    || ((column == CueModelColumns::color) && (kernel->cuelistView->selectedCue()->colors.value(group, nullptr) != formerCue->colors.value(group, nullptr)))
                    || ((column == CueModelColumns::position) && (kernel->cuelistView->selectedCue()->positions.value(group, nullptr) != formerCue->positions.value(group, nullptr)))
                    || ((column == CueModelColumns::raws) && (kernel->cuelistView->selectedCue()->raws.value(group, QList<Raw*>()) != formerCue->raws.value(group, QList<Raw*>())))
                    || ((column == CueModelColumns::effects) && (kernel->cuelistView->selectedCue()->effects.value(group, QList<Effect*>()) != formerCue->effects.value(group, QList<Effect*>())))
                    );
            } else {
                columnValueChanged = (
                    ((column == CueModelColumns::intensity) && kernel->cuelistView->selectedCue()->intensities.contains(group))
                    || ((column == CueModelColumns::color) && kernel->cuelistView->selectedCue()->colors.contains(group))
                    || ((column == CueModelColumns::position) && kernel->cuelistView->selectedCue()->positions.contains(group))
                    || ((column == CueModelColumns::raws) && kernel->cuelistView->selectedCue()->raws.contains(group))
                    || ((column == CueModelColumns::effects) && kernel->cuelistView->selectedCue()->effects.contains(group))
                );
            }
        }
        if (columnValueChanged) {
            return QColor(48, 0, 0);
        }
        if (group == kernel->cuelistView->currentGroup) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

QVariant CueModel::headerData(int column, Qt::Orientation orientation, int role) const {
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

QList<Group*> CueModel::getGroupRows() const {
    QList<Group*> groups;
    if (kernel->cuelistView->filterComboBox->currentIndex() == CuelistViewFilters::noFilter) {
        groups = kernel->groups->items;
    } else if (kernel->cuelistView->filterComboBox->currentIndex() == CuelistViewFilters::activeRowsFilter) {
        if (kernel->cuelistView->selectedCue() == nullptr) {
            return QList<Group*>();
        }
        for (Group* group : kernel->groups->items) {
            if (kernel->cuelistView->selectedCue()->intensities.contains(group)
                || kernel->cuelistView->selectedCue()->colors.contains(group)
                || kernel->cuelistView->selectedCue()->positions.contains(group)
                || kernel->cuelistView->selectedCue()->raws.contains(group)
                || kernel->cuelistView->selectedCue()->effects.contains(group)) {
                groups.append(group);
            }
        }
    } else if (kernel->cuelistView->filterComboBox->currentIndex() == CuelistViewFilters::changedRowsFilter) {
        if (kernel->cuelistView->selectedCue() == nullptr) {
            return QList<Group*>();
        }
        for (Group* group : kernel->groups->items) {
            if (kernel->cuelistView->currentCuelist->cues->items.indexOf(kernel->cuelistView->selectedCue()) <= 0) {
                if (kernel->cuelistView->selectedCue()->intensities.contains(group)
                    || kernel->cuelistView->selectedCue()->colors.contains(group)
                    || kernel->cuelistView->selectedCue()->positions.contains(group)
                    || kernel->cuelistView->selectedCue()->raws.contains(group)
                    || kernel->cuelistView->selectedCue()->effects.contains(group)) {
                    groups.append(group);
                }
            } else {
                Cue* formerCue = kernel->cuelistView->currentCuelist->cues->items[kernel->cuelistView->currentCuelist->cues->items.indexOf(kernel->cuelistView->selectedCue()) - 1];
                if ((kernel->cuelistView->selectedCue()->intensities.value(group, nullptr) != formerCue->intensities.value(group, nullptr))
                    || (kernel->cuelistView->selectedCue()->colors.value(group, nullptr) != formerCue->colors.value(group, nullptr))
                    || (kernel->cuelistView->selectedCue()->positions.value(group, nullptr) != formerCue->positions.value(group, nullptr))
                    || (kernel->cuelistView->selectedCue()->raws.value(group, QList<Raw*>()) != formerCue->raws.value(group, QList<Raw*>()))
                    || (kernel->cuelistView->selectedCue()->effects.value(group, QList<Effect*>()) != formerCue->effects.value(group, QList<Effect*>()))
                    ) {
                    groups.append(group);
                }
            }
        }
    } else {
        Q_ASSERT(false);
    }
    return groups;
}
