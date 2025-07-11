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
class Cuelist;
class Fixture;
class Group;
class Cue;

namespace CuelistViewModes {
enum {
    cueMode,
    groupMode,
};
}

namespace CuelistViewRowFilters {
enum {
    noFilter,
    activeRowsFilter,
    changedRowsFilter,
};
}

class CuelistView : public QWidget {
    Q_OBJECT
public:
    CuelistView(Kernel *core, QWidget *parent = nullptr);
    void reload();
    void loadCuelist(QString cuelistId);
    void loadCue(QString cueId);
    void nextCue();
    void previousCue();
    void loadGroup(QString groupId);
    void nextGroup();
    void previousGroup();
    void loadFixture(QString fixtureId);
    void nextFixture();
    void previousFixture();
    void noFixture();
    bool isSelected(Fixture* fixture);
    Cuelist* currentCuelist = nullptr;
    Cue* selectedCue() const;
    Cue* currentCue = nullptr;
    Fixture* currentFixture = nullptr;
    Group* currentGroup = nullptr;
    QPushButton* trackingButton;
    void updateCuelistView();
    QComboBox *cueViewModeComboBox;
    QComboBox *cueViewRowFilterComboBox;
private:
    Kernel *kernel;
    QTableView *cuelistTableView;
    CueModel *cueModel;
    GroupModel *groupModel;
    QLabel *cuelistLabel;
    QLabel *cueLabel;
    QLabel *groupLabel;
    QLabel *fixtureLabel;
};

#include "kernel/kernel.h"

#endif // CUELISTVIEW_H
