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
#include "terminal/terminal.h"
#include "inspector/inspector.h"
#include "cuelistview/cuelistview.h"
#include "dmxengine/dmxengine.h"
#include "preview2d/preview2d.h"
#include "controlpanel/controlpanel.h"
#include "playbackmonitor/playbackmonitor.h"
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
    Terminal *terminal;
    Inspector *inspector;
    CuelistView *cuelistView;
    DmxEngine *dmxEngine;
    Preview2d *preview2d;
    ControlPanel *controlPanel;
    PlaybackMonitor *playbackMonitor;
    QMutex *mutex;
    const QString VERSION = "0.5.0";

    const QString IDATTRIBUTEID = "0";
    const QString LABELATTRIBUTEID = "1";

    const QString MODELCHANNELSATTRIBUTEID = "2";
    const QString MODELPANRANGEATTRIBUTEID = "3.1";
    const QString MODELTILTRANGEATTRIBUTEID = "3.2";
    const QString MODELMINZOOMATTRIBUTEID = "4.1";
    const QString MODELMAXZOOMATTRIBUTEID = "4.2";

    const QString FIXTUREMODELATTRIBUTEID = "2";
    const QString FIXTUREADDRESSATTRIBUTEID = "3.1";
    const QString FIXTUREUNIVERSEATTRIBUTEID = "3.2";
    const QString FIXTUREPOSITIONXATTRIBUTEID = "4.1";
    const QString FIXTUREPOSITIONYATTRIBUTEID = "4.2";
    const QString FIXTUREROTATIONATTRIBUTEID = "5.1";
    const QString FIXTUREINVERTPANATTRIBUTEID = "5.2";

    const QString GROUPFIXTURESATTRIBUTEID = "2";

    const QString INTENSITYDIMMERATTRIBUTEID = "2";
    const QString INTENSITYRAWSATTRIBUTEID = "3";

    const QString COLORHUEATTRIBUTEID = "2";
    const QString COLORSATURATIONATTRIBUTEID = "3";
    const QString COLORQUALITYATTRIBUTEID = "4";
    const QString COLORRAWSATTRIBUTEID ="5";

    const QString POSITIONPANATTRIBUTEID = "2";
    const QString POSITIONTILTATTRIBUTEID = "3";
    const QString POSITIONZOOMATTRIBUTEID = "4";
    const QString POSITIONRAWSATTRIBUTEID = "5";

    const QString EFFECTSTEPSATTRIBUTEID = "2";
    const QString EFFECTINTENSITYSTEPSATTRIBUTEID = "3";
    const QString EFFECTCOLORSTEPSATTRIBUTEID = "4";
    const QString EFFECTPOSITIONSTEPSATTRIBUTEID = "5";
    const QString EFFECTRAWSTEPSATTRIBUTEID = "6";
    const QString EFFECTSTEPHOLDATTRIBUTEID = "7";
    const QString EFFECTSTEPFADEATTRIBUTEID = "8";
    const QString EFFECTPHASEATTRIBUTEID = "9";

    const QString RAWCHANNELVALUEATTRIBUTEID = "2";
    const QString RAWMOVEWHILEDARKATTRIBUTEID = "3";
    const QString RAWFADEATTRIBUTEID = "4";

    const QString CUELISTCUEATTRIBUTEID = "2";
    const QString CUELISTPRIORITYATTRIBUTEID = "3";
    const QString CUELISTMOVEWHILEDARKATTRIBUTEID = "4";

    const QString CUEINTENSITIESATTRIBUTEID = "2";
    const QString CUECOLORSATTRIBUTEID = "3";
    const QString CUEPOSITIONSATTRIBUTEID = "4";
    const QString CUERAWSATTRIBUTEID = "5";
    const QString CUEEFFECTSATTRIBUTEID = "6";
    const QString CUEFADEATTRIBUTEID = "7";
    const QString CUEBLOCKATTRIBUTEID = "8";
private:
    QMap<int, QString> xmlToCommandAttributes(QXmlStreamAttributes streamAttributes);
};

#endif // KERNEL_H
