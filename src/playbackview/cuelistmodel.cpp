#include "cuelistmodel.h"
#include "kernel/kernel.h"

CuelistModel::CuelistModel(Kernel* core) {
    kernel = core;
}

int CuelistModel::rowCount(const QModelIndex&) const {
    return kernel->cuelists->items.size();
}

int CuelistModel::columnCount(const QModelIndex&) const {
    return 2;
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
