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
    QHash<int, int> oldCuelistSortkeys = cuelistSortkeys;
    cuelistSortkeys.clear();

    QSqlQuery cuelistQuery;
    if (cuelistQuery.exec("SELECT cuelists.key, cues.sortkey FROM cuelists, cues WHERE cuelists.movewhiledark = 1 AND cuelists.currentcue_key = cues.key ORDER BY cuelists.priority, cuelists.sortkey")) {
        while (cuelistQuery.next()) {
            const int cuelistKey = cuelistQuery.value(0).toInt();
            const int currentCueSortkey = cuelistQuery.value(1).toInt();

            if (oldCuelistSortkeys.contains(cuelistKey)) {
                cuelistSortkeys[cuelistKey] = oldCuelistSortkeys.value(cuelistKey);
            } else {
                cuelistSortkeys[cuelistKey] = currentCueSortkey + 1;
            }

            int cueKey = getCueKey(cuelistKey);
            if ((cueKey < 0) && oldCuelistSortkeys.contains(cuelistKey)) {
                cuelistSortkeys[cuelistKey] = currentCueSortkey + 1;
                cueKey = getCueKey(cuelistKey);
            }
            if (cueKey > 0) {
                const int sortkeyDifference = cuelistSortkeys.value(cuelistKey) - currentCueSortkey;
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
        }
    } else {
        qWarning() << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
    }
}

QSet<int> MwDManager::getMwDCues(const QList<int> groupKeys) {
    int difference = -1;
    QSet<int> cueKeys;
    for (const int groupKey : groupKeys) {
        if (groupData.contains(groupKey)) {
            const GroupData group = groupData.value(groupKey);
            if (group.difference < difference) {
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

int MwDManager::getCueKey(const int cuelistKey) {
    QSqlQuery query;
    query.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey = :sort");
    query.bindValue(":cuelist", cuelistKey);
    query.bindValue(":sort", cuelistSortkeys.value(cuelistKey));
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qWarning() << query.executedQuery() << query.lastError().text();
    }
    return -1;
}

    /*QHash<int, int> mwdGroupCues;
    QHash<int, int> mwdGroupCueDifference;
    const int cuelistKey = mwdCuelistQuery.value(0).toInt();
    const int currentCueKey = mwdCuelistQuery.value(1).toInt();

    QSqlQuery cuesQuery;
    cuesQuery.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey > (SELECT sortkey FROM cues WHERE key = :currentcue) LIMIT 10");
    cuesQuery.bindValue(":cuelist", cuelistKey);
    cuesQuery.bindValue(":currentcue", currentCueKey);
    if (cuesQuery.exec()) {
        while (cuesQuery.next()) {
            const int cueKey = cuesQuery.value(0).toInt();
            const int cueDifference = cuesQuery.at() + 1;

            for (const int groupKey : groupKeys) {
                if (!mwdGroupCueDifference.contains(groupKey) || (mwdGroupCueDifference.value(groupKey) >= cueDifference)) {
                    for (QString table : tables) {
                        QSqlQuery cueValueQuery;
                        cueValueQuery.prepare("SELECT valueitem_key FROM " + table + " WHERE item_key = :cue AND foreignitem_key = :group LIMIT 1");
                        cueValueQuery.bindValue(":cue", cueKey);
                        cueValueQuery.bindValue(":group", groupKey);
                        if (cueValueQuery.exec()) {
                            if (cueValueQuery.next()) {
                                mwdGroupCues[groupKey] = cueKey;
                                mwdGroupCueDifference[groupKey] = cueDifference;
                                break;
                            }
                        } else {
                            qWarning() << cueValueQuery.executedQuery() << cueValueQuery.lastError().text();
                        }
                    }
                }
            }
        }
    } else {
        qWarning() << cuesQuery.executedQuery() << cuesQuery.lastError().text();
    }*/