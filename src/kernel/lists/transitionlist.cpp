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

Transition* TransitionList::getItem(QString id)
{
    int transitionRow = getItemRow(id);
    if (transitionRow < 0) {
        return nullptr;
    }
    return items[transitionRow];
}

int TransitionList::getItemRow(QString id)
{
    for (int transitionRow = 0; transitionRow < items.size(); transitionRow++) {
        if (items[transitionRow]->id == id) {
            return transitionRow;
        }
    }
    return -1;
}

Transition* TransitionList::getItemByRow(int row) {
    if (row >= items.size() || row < 0) {
        return nullptr;
    }
    return items[row];
}

bool TransitionList::copyItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Transition* transition = getItem(id);
        if (transition == nullptr) {
            kernel->terminal->error("Transition can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Transition can't be copied because Target ID is already used.");
            return false;
        }
        Transition *targetTransition = recordTransition(targetId);
        targetTransition->label = transition->label;
        targetTransition->fade = transition->fade;
    }
    return true;
}

bool TransitionList::deleteItems(QList<QString> ids)
{
    for (QString id : ids) {
        int transitionRow = getItemRow(id);
        if (transitionRow < 0) {
            kernel->terminal->error("Transition can't be deleted because it doesn't exist.");
            return false;
        }
        Transition *transition = items[transitionRow];
        kernel->cues->deleteTransition(transition);
        items.removeAt(transitionRow);
        delete transition;
    }
    return true;
}

bool TransitionList::moveItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int transitionRow = getItemRow(id);
        if (transitionRow < 0) {
            kernel->terminal->error("Transition can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Transition can't be moved because Target ID is already used.");
            return false;
        }
        Transition* transition = items[transitionRow];
        items.removeAt(transitionRow);
        transition->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, transition);
    }
    return true;
}

Transition* TransitionList::recordTransition(QString id)
{
    Transition *transition = new Transition;
    transition->id = id;
    transition->label = QString();
    transition->fade = 0;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, transition);
    return transition;
}

bool TransitionList::recordTransitionFade(QList<QString> ids, float fade)
{
    if (fade > 60 || fade < 0) {
        kernel->terminal->error("Record Transition Fade only allows from 0s to 60s.");
        return false;
    }
    for (QString id : ids) {
        Transition* transition = getItem(id);
        if (transition == nullptr) {
            transition = recordTransition(id);
        }
        transition->fade = fade;
    }
    return true;
}

QList<QString> TransitionList::getIds() {
    QList<QString> ids;
    for (Transition *transition : items) {
        ids.append(transition->id);
    }
    return ids;
}

int TransitionList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return items.size();
}
