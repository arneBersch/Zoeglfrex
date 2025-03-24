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
#include "kernel/lists/rawlist.h"
#include "kernel/lists/cuelist.h"
#include "terminal/terminal.h"
#include "inspector/inspector.h"
#include "cuelistview/cuelistview.h"

class Kernel {
public:
    Kernel();
    void reset();
    void saveFile(QString fileName);
    void openFile(QString fileName);
    ModelList *models;
    FixtureList *fixtures;
    GroupList *groups;
    IntensityList *intensities;
    ColorList *colors;
    RawList *raws;
    CueList *cues;
    Terminal *terminal;
    Inspector *inspector;
    CuelistView *cuelistView;
    QMutex *mutex;
    const QString VERSION = "0.4.0";
};

#endif // KERNEL_H
