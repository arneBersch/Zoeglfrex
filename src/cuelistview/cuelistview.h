/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELISTVIEW_H
#define CUELISTVIEW_H

#include <QtWidgets>

#include "cuelistview/cuemodel.h"
#include "cuelistview/groupmodel.h"
class Kernel;
class Fixture;
class Group;
class Cue;
class DmxEngine;

class CuelistView : public QWidget {
    Q_OBJECT
public:
    CuelistView(Kernel *core, QWidget *parent = nullptr);
    void loadView();
    void loadCue(QString cueId);
    void loadGroup(QString groupId);
    void nextCue();
    void previousCue();
    void nextGroup();
    void previousGroup();
    void nextFixture();
    void previousFixture();
    Fixture* currentFixture = nullptr;
    Group* currentGroup = nullptr;
    Cue* currentCue = nullptr;
    DmxEngine *dmxEngine;
private:
    void updateCuelistView();
    Kernel *kernel;
    QComboBox *cueViewModeComboBox;
    const QString CUEVIEWCUEMODE = "Cue Mode";
    const QString CUEVIEWGROUPMODE = "Group Mode";
    QTableView *cuelistTableView;
    CueModel *cueModel;
    GroupModel *groupModel;
    QLabel *cueOrGroupLabel;
    QLabel *fixtureLabel;
};

#include "kernel/kernel.h"
#include "cuelistview/dmxengine.h"

#endif // CUELISTVIEW_H
