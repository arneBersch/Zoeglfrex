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

#include "kernel/tables/modeltable.h"
#include "kernel/tables/fixturetable.h"
#include "kernel/tables/grouptable.h"
#include "kernel/tables/intensitytable.h"
#include "kernel/tables/colortable.h"
#include "kernel/tables/positiontable.h"
#include "kernel/tables/rawtable.h"
#include "kernel/tables/effecttable.h"
#include "kernel/tables/cuelisttable.h"
#include "kernel/tables/cuetable.h"
#include "terminal/terminal.h"
#include "inspector/inspector.h"
#include "cuelistview/cuelistview.h"
#include "dmxengine/dmxengine.h"
#include "preview2d/preview2d.h"
#include "controlpanel/controlpanel.h"
#include "mainwindow/mainwindow.h"

class Kernel {
public:
    Kernel(MainWindow* mainWindow);
    void reset();
    void saveFile(QString fileName);
    void openFile(QString fileName);
    MainWindow* mainWindow;
    ModelTable *models;
    FixtureTable *fixtures;
    GroupTable *groups;
    IntensityTable *intensities;
    ColorTable *colors;
    PositionTable *positions;
    RawTable *raws;
    EffectTable *effects;
    CuelistTable* cuelists;
    CueTable *cues;
    Terminal *terminal;
    Inspector *inspector;
    CuelistView *cuelistView;
    DmxEngine *dmxEngine;
    Preview2d *preview2d;
    ControlPanel *controlPanel;
    QMutex *mutex;
    const QString VERSION = "0.5.0";
};

#endif // KERNEL_H
