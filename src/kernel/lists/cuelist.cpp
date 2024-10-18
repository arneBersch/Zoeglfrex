/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelist.h"

CueList::CueList(Kernel *core) {
    kernel = core;
}

Cue* CueList::getCue(QString id) {
    int cueRow = getCueRow(id);
    if (cueRow < 0 || cueRow >= cues.size()) {
        return nullptr;
    }
    return cues[cueRow];
}

int CueList::getCueRow(QString id) {
    for (int cueRow = 0; cueRow < cues.size(); cueRow++) {
        if (cues[cueRow]->id == id) {
            return cueRow;
        }
    }
    return -1;
}

QString CueList::copyCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Cue can't be copied because it doesn't exist.";
        }
        if (getCue(targetId) != nullptr) {
            return "Cue can't be copied because Target ID is already used.";
        }
        Cue *targetCue = recordCue(targetId, cue->transition);
        targetCue->label = cue->label;
        targetCue->intensities = cue->intensities;
        targetCue->colors = cue->colors;
        targetCue->transition = cue->transition;
    }
    return QString();
}

QString CueList::deleteCue(QList<QString> ids) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            return "Cue can't be deleted because it doesn't exist.";
        }
        Cue *cue = cues[cueRow];
        cues.removeAt(cueRow);
        delete cue;
        if (kernel->cuelistView->currentCue == cue) {
            kernel->cuelistView->currentCue = nullptr;
        }
    }
    return QString();
}

void CueList::deleteIntensity(Intensity* intensity) {
    for (Cue *cue : cues) {
        for (Row *row : cue->intensities.keys()) {
            if (cue->intensities.value(row) == intensity) {
                cue->intensities.remove(row);
            }
        }
    }
}

void CueList::deleteColor(Color* color) {
    for (Cue *cue : cues) {
        for (Row *row : cue->colors.keys()) {
            if (cue->colors.value(row) == color) {
                cue->colors.remove(row);
            }
        }
    }
}

void CueList::deleteTransition(Transition* transition) {
    QList<QString> invalidCues;
    for (Cue *cue : cues) {
        if (cue->transition == transition) {
            invalidCues.append(cue->id);
        }
    }
    deleteCue(invalidCues);
}

void CueList::deleteRow(Row *row) {
    for (Cue *cue : cues) {
        cue->intensities.remove(row);
        cue->colors.remove(row);
    }
}

QString CueList::deleteCueRowIntensity(QList<QString> ids, QString rowId) {
    Row* row = kernel->rows->getRow(rowId);
    if (row == nullptr) {
        return tr("Can't delete Cue Row Intensity: Row %1 doesn't exist.").arg(rowId);
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            return tr("Can't delete Cue Row Intensity because group %1 doesn't exist").arg(id);
        }
        cue->intensities.remove(row);
    }
    return QString();
}

QString CueList::deleteCueRowColor(QList<QString> ids, QString rowId) {
    Row* row = kernel->rows->getRow(rowId);
    if (row == nullptr) {
        return tr("Can't delete Cue Row Color: Row %1 doesn't exist.").arg(rowId);
    }
    for (QString id : ids) {
        Cue *cue = getCue(id);
        if (cue == nullptr) {
            return tr("Can't delete Cue Row Color because group %1 doesn't exist").arg(id);
        }
        cue->colors.remove(row);
    }
    return QString();
}

QString CueList::labelCue(QList<QString> ids, QString label) {
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Cue can't be labeled because it doesn't exist.";
        }
        cue->label = label;
    }
    return QString();
}

QString CueList::moveCue(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int cueRow = getCueRow(id);
        if (cueRow < 0) {
            return "Cue can't be moved because it doesn't exist.";
        }
        if (getCue(targetId) != nullptr) {
            return "Cue can't be moved because Target ID is already used.";
        }
        Cue* cue = cues[cueRow];
        cues.removeAt(cueRow);
        cue->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < cues.size(); index++) {
            QList<QString> indexIdParts = (cues[index]->id).split(".");
            if (indexIdParts[0].toInt() < idParts[0].toInt()) {
                position++;
            } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
                if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                    position++;
                } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                    if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                        position++;
                    } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                        if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                            position++;
                        } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                            if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                                position++;
                            }
                        }
                    }
                }
            }
        }
        cues.insert(position, cue);
    }
    return QString();
}

Cue* CueList::recordCue(QString id, Transition *transition) {
    Cue *cue = new Cue;
    cue->id = id;
    cue->label = QString();
    cue->transition = transition;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < cues.size(); index++) {
        QList<QString> indexIdParts = (cues[index]->id).split(".");
        if (indexIdParts[0].toInt() < idParts[0].toInt()) {
            position++;
        } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
            if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                position++;
            } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                    position++;
                } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                    if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                        position++;
                    } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                        if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                            position++;
                        }
                    }
                }
            }
        }
    }
    cues.insert(position, cue);
    return cue;
}

QString CueList::recordCueTransition(QList<QString> ids, QString transitionId) {
    Transition* transition = kernel->transitions->getTransition(transitionId);
    if (transition == nullptr) {
        return "Can't record Cue because Transition doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            cue = recordCue(id, transition);
        }
        cue->transition = transition;
    }
    return QString();
}

QString CueList::recordCueIntensity(QList<QString> ids, QString rowId, QString intensityId) {
    Row* row = kernel->rows->getRow(rowId);
    if (row == nullptr) {
        return "Can't record Cue because Row doesn't exist.";
    }
    Intensity* intensity = kernel->intensities->getIntensity(intensityId);
    if (intensity == nullptr) {
        return "Can't record Cue because Intensity doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Can't record Cue because no transition was specified.";
        }
        cue->intensities[row] = intensity;
    }
    return QString();
}

QString CueList::recordCueColor(QList<QString> ids, QString rowId, QString colorId) {
    Row* row = kernel->rows->getRow(rowId);
    if (row == nullptr) {
        return "Can't record Cue because Row doesn't exist.";
    }
    Color* color = kernel->colors->getColor(colorId);
    if (color == nullptr) {
        return "Can't record Cue because Color doesn't exist.";
    }
    for (QString id : ids) {
        Cue* cue = getCue(id);
        if (cue == nullptr) {
            return "Can't record Cue because no transition was specified.";
        }
        cue->colors[row] = color;
    }
    return QString();
}

QList<QString> CueList::getIds() {
    QList<QString> ids;
    for (Cue *cue : cues) {
        ids.append(cue->id);
    }
    return ids;
}

int CueList::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return cues.size();
}

int CueList::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 3;
}

QVariant CueList::data(const QModelIndex &index, const int role) const {
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == CueListColumns::id) {
            return cues[row]->id;
        } else if (column == CueListColumns::label) {
            return cues[row]->label;
        } else if (column == CueListColumns::transition) {
            return (cues[row]->transition)->id;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant CueList::headerData(int column, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == CueListColumns::id) {
            return "ID";
        } else if (column == CueListColumns::label) {
            return "Label";
        } else if (column == CueListColumns::transition) {
            return "Transition";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
