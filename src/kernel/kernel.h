/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KERNEL_H
#define KERNEL_H

#include <QtWidgets>

#include "kernel/lists/modellist.h"
#include "kernel/lists/fixturelist.h"
#include "kernel/lists/grouplist.h"
#include "kernel/lists/intensitylist.h"
#include "kernel/lists/colorlist.h"
#include "kernel/lists/transitionlist.h"
#include "kernel/lists/rowlist.h"
#include "kernel/lists/cuelist.h"
#include "terminal/terminal.h"
#include "inspector/inspector.h"
#include "cuelistview/cuelistview.h"

namespace keys {
enum {
    zero, // 0
    one, // 1
    two, // 2
    three, // 3
    four, // 4
    five, // 5
    six, // 6
    seven, // 7
    eight, // 8
    nine, // 9
    plus, // +
    minus, // -
    period, // .
    comma, // ,
    thru, // /
    model, // M
    fixture, // F
    group, // G
    intensity, // I
    color, // C
    transition, // T
    cue, // C
    row, // R
    copyItem, // SHIFT C
    deleteItem, // SHIFT D
    labelItem, // SHIFT L
    moveItem, // SHIFT M
    recordItem, // SHIFT R
};
}

class Kernel {
public:
    Kernel();
    QString execute(QList<int> command, QString text = QString());
    ModelList *models;
    FixtureList *fixtures;
    GroupList *groups;
    IntensityList *intensities;
    ColorList *colors;
    TransitionList *transitions;
    RowList *rows;
    CueList *cues;
    Terminal *terminal;
    Inspector *inspector;
    CuelistView *cuelistView;
    QMutex *mutex;
private:
    bool isItem(int key);
    bool isOperator(int key);
    bool isNumber(int key);
    int keyToNumber(int key);
    QString keysToId(QList<int> keys);
    QList<float> keysToValue(QList<int> keys);
    QList<QString> keysToSelection(QList<int> keys);
};

#endif // KERNEL_H
