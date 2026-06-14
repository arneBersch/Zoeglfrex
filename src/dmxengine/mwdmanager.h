/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets>
#include <QtSql>

#ifndef MWDMANAGER_H
#define MWDMANAGER_H

class MwDManager {
public:
    MwDManager();
    void nextFrame();
    QSet<int> getMwDCues(QList<int> groupKeys);
private:
    int getCueKey(int cuelistKey);
    struct GroupData {
        int difference;
        QSet<int> cueKeys;
    };
    QHash<int, GroupData> groupData;
    QHash<int, int> cuelistSortkeys;
    const QStringList mwdValueTables = {
        "cue_group_intensities",
        "cue_group_colors",
        "cue_group_positions",
        "cue_group_raws",
        "cue_group_effects"
    };
};

#endif // MWDMANAGER_H
