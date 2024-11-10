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

Transition* TransitionList::recordTransition(QString id) {
    Transition *transition = new Transition(kernel);
    transition->id = id;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, transition);
    return transition;
}

bool TransitionList::recordTransitionFade(QList<QString> ids, float fade) {
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
