/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "transitionlist.h"

TransitionList::TransitionList(Kernel *core) {
    kernel = core;
}

Transition* TransitionList::getTransition(QString id)
{
    int transitionRow = getTransitionRow(id);
    if (transitionRow < 0) {
        return nullptr;
    }
    return transitions[transitionRow];
}

int TransitionList::getTransitionRow(QString id)
{
    for (int transitionRow = 0; transitionRow < transitions.size(); transitionRow++) {
        if (transitions[transitionRow]->id == id) {
            return transitionRow;
        }
    }
    return -1;
}

QString TransitionList::copyTransition(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Transition* transition = getTransition(id);
        if (transition == nullptr) {
            return "Transition can't be copied because it doesn't exist.";
        }
        if (getTransition(targetId) != nullptr) {
            return "Transition can't be copied because Target ID is already used.";
        }
        Transition *targetTransition = recordTransition(targetId);
        targetTransition->label = transition->label;
        targetTransition->fade = transition->fade;
    }
    return QString();
}

QString TransitionList::deleteTransition(QList<QString> ids)
{
    for (QString id : ids) {
        int transitionRow = getTransitionRow(id);
        if (transitionRow < 0) {
            return "Transition can't be deleted because it doesn't exist.";
        }
        Transition *transition = transitions[transitionRow];
        kernel->cues->deleteTransition(transition);
        transitions.removeAt(transitionRow);
        delete transition;
    }
    return QString();
}

QString TransitionList::labelTransition(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Transition* transition = getTransition(id);
        if (transition == nullptr) {
            return "Transition can't be labeled because it doesn't exist.";
        }
        transition->label = label;
    }
    return QString();
}

QString TransitionList::moveTransition(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int transitionRow = getTransitionRow(id);
        if (transitionRow < 0) {
            return "Transition can't be moved because it doesn't exist.";
        }
        if (getTransition(targetId) != nullptr) {
            return "Transition can't be moved because Target ID is already used.";
        }
        Transition* transition = transitions[transitionRow];
        transitions.removeAt(transitionRow);
        transition->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < transitions.size(); index++) {
            QList<QString> indexIdParts = (transitions[index]->id).split(".");
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
        transitions.insert(position, transition);
    }
    return QString();
}

Transition* TransitionList::recordTransition(QString id)
{
    Transition *transition = new Transition;
    transition->id = id;
    transition->label = QString();
    transition->fade = 0;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < transitions.size(); index++) {
        QList<QString> indexIdParts = (transitions[index]->id).split(".");
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
    transitions.insert(position, transition);
    return transition;
}

QString TransitionList::recordTransitionFade(QList<QString> ids, float fade)
{
    if (fade > 60 || fade < 0) {
        return "Record Transition Fade only allows from 0s to 60s.";
    }
    for (QString id : ids) {
        Transition* transition = getTransition(id);
        if (transition == nullptr) {
            transition = recordTransition(id);
        }
        transition->fade = fade;
    }
    return QString();
}

QList<QString> TransitionList::getIds() {
    QList<QString> ids;
    for (Transition *transition : transitions) {
        ids.append(transition->id);
    }
    return ids;
}

int TransitionList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return transitions.size();
}

int TransitionList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant TransitionList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == TransitionListColumns::id) {
            return transitions[row]->id;
        } else if (column == TransitionListColumns::label) {
            return transitions[row]->label;
        } else if (column == TransitionListColumns::fade) {
            return transitions[row]->fade;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant TransitionList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == TransitionListColumns::id) {
            return "ID";
        } else if (column == TransitionListColumns::label) {
            return "Label";
        } else if (column == TransitionListColumns::fade) {
            return "Fade (s)";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

