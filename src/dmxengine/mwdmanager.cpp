/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mwdmanager.h"

MwDManager::MwDManager() {}

void MwDManager::nextFrame() {
    sortkeyDifference++;

    for (const int cuelistKey : cuelistSortkeys.keys()) {
        QSqlQuery cueQuery;
        cueQuery.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey = :sort");
        cueQuery.bindValue(":cuelist", cuelistKey);
        cueQuery.bindValue(":sort", cuelistSortkeys.value(cuelistKey) + sortkeyDifference);
        if (cueQuery.exec()) {
            if (cueQuery.next()) {
                renderCue(cueQuery.value(0).toInt());
            } else {
                cuelistSortkeys.remove(cuelistKey);
            }
        } else {
            qWarning() << cueQuery.executedQuery() << cueQuery.lastError().text();
        }
    }

    if (cuelistSortkeys.isEmpty()) {
        oldGroupData = groupData;
        groupData.clear();

        sortkeyDifference = 0;

        QSqlQuery cuelistQuery;
        if (cuelistQuery.exec("SELECT cuelists.key, cues.sortkey FROM cuelists, cues WHERE cuelists.movewhiledark = 1 AND cuelists.currentcue_key = cues.key")) {
            while (cuelistQuery.next()) {
                cuelistSortkeys[cuelistQuery.value(0).toInt()] = cuelistQuery.value(1).toInt();
            }
        } else {
            qWarning() << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        }
    }
}

void MwDManager::renderCue(const int cueKey) {
    for (QString table : mwdValueTables) {
        QSqlQuery cueValueQuery;
        cueValueQuery.prepare("SELECT DISTINCT foreignitem_key FROM " + table + " WHERE item_key = :cue");
        cueValueQuery.bindValue(":cue", cueKey);
        if (cueValueQuery.exec()) {
            while (cueValueQuery.next()) {
                const int groupKey = cueValueQuery.value(0).toInt();
                if (!groupData.contains(groupKey) || (groupData.value(groupKey).difference > sortkeyDifference)) {
                    groupData[groupKey] = { sortkeyDifference, { cueKey }};
                } else if (groupData.value(groupKey).difference == sortkeyDifference) {
                    groupData[groupKey].cueKeys.insert(cueKey);
                }
            }
        } else {
            qWarning() << cueValueQuery.executedQuery() << cueValueQuery.lastError().text();
        }
    }
}

QSet<int> MwDManager::getMwDCues(const QList<int> groupKeys) {
    int difference = -1;
    QSet<int> cueKeys;
    for (const int groupKey : groupKeys) {
        if (oldGroupData.contains(groupKey)) {
            const GroupData group = oldGroupData.value(groupKey);
            if ((group.difference < difference) || cueKeys.isEmpty()) {
                difference = group.difference;
                cueKeys.clear();
            }
            if (group.difference == difference) {
                cueKeys.unite(group.cueKeys);
            }
        }
    }
    return cueKeys;
}
