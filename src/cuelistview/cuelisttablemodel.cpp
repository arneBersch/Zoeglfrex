/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelisttablemodel.h"

CuelistTableModel::CuelistTableModel() {}

void CuelistTableModel::refresh() {
    beginResetModel();
    int currentCueKey = -1;
    QSqlQuery currentCueQuery;
    if (currentCueQuery.exec("SELECT key FROM currentcue")) {
        if (currentCueQuery.next()) {
            currentCueKey = currentCueQuery.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
    }
    int currentGroupKey = -1;
    QSqlQuery currentGroupQuery;
    if (currentGroupQuery.exec("SELECT group_key FROM currentitems")) {
        if (currentGroupQuery.next()) {
            currentGroupKey = currentGroupQuery.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << currentGroupQuery.executedQuery() << currentGroupQuery.lastError().text();
    }
    currentKey = -1;
    if (mode == CueMode) {
        currentKey = currentGroupKey;
    } else if (mode == GroupMode) {
        currentKey = currentCueKey;
    }
    rows.clear();
    QSqlQuery rowQuery;
    if (mode == CueMode) {
        rowQuery.prepare("SELECT key, CONCAT(id, ' ', label) FROM groups ORDER BY sortkey");
    } else if (mode == GroupMode) {
        rowQuery.prepare("SELECT key, CONCAT(id, ' ', label) FROM currentcuelist_cues ORDER BY sortkey");
    }
    if (rowQuery.exec()) {
        while (rowQuery.next()) {
            RowData row;
            row.key = rowQuery.value(0).toInt();
            row.name = rowQuery.value(1).toString();
            int cueKey = currentCueKey;
            int groupKey = currentGroupKey;
            if (mode == CueMode) {
                groupKey = row.key;
            } else if (mode == GroupMode) {
                cueKey = row.key;
            }
            int lastCueKey = -1;
            QSqlQuery lastCueKeyQuery;
            lastCueKeyQuery.prepare("SELECT key FROM cues WHERE cuelist_key = (SELECT cuelist_key FROM cues WHERE key = :cue) AND sortkey < (SELECT sortkey FROM cues WHERE key = :cue) ORDER BY sortkey DESC LIMIT 1");
            lastCueKeyQuery.bindValue(":cue", cueKey);
            if (lastCueKeyQuery.exec()) {
                if (lastCueKeyQuery.next()) {
                    lastCueKey = lastCueKeyQuery.value(0).toInt();
                }
            } else {
                qWarning() << Q_FUNC_INFO << lastCueKeyQuery.executedQuery() << lastCueKeyQuery.lastError().text();
            }
            QStringList intensities = getCueValue("intensities", "cue_group_intensities", cueKey, groupKey);
            QStringList colors = getCueValue("colors", "cue_group_colors", cueKey, groupKey);
            QStringList positions = getCueValue("positions", "cue_group_positions", cueKey, groupKey);
            QStringList raws = getCueValue("raws", "cue_group_raws", cueKey, groupKey);
            QStringList effects = getCueValue("effects", "cue_group_effects", cueKey, groupKey);
            row.intensity = intensities.join(", ");
            row.color = colors.join(", ");
            row.position = positions.join(", ");
            row.raws = raws.join(", ");
            row.effects = effects.join(", ");
            row.intensityChanged = (intensities != getCueValue("intensities", "cue_group_intensities", lastCueKey, groupKey));
            row.colorChanged = (colors != getCueValue("colors", "cue_group_colors", lastCueKey, groupKey));
            row.positionChanged = (positions != getCueValue("positions", "cue_group_positions", lastCueKey, groupKey));
            row.rawsChanged = (raws != getCueValue("raws", "cue_group_raws", lastCueKey, groupKey));
            row.effectsChanged = (effects != getCueValue("effects", "cue_group_effects", lastCueKey, groupKey));
            if (filter == AllRows) {
                rows.append(row);
            } else if (filter == ActiveRows) {
                if (!row.intensity.isEmpty() || !row.color.isEmpty() || !row.position.isEmpty() || !row.raws.isEmpty() || !row.effects.isEmpty()) {
                    rows.append(row);
                }
            } else if (filter == ChangedRows) {
                if (row.intensityChanged || row.colorChanged || row.positionChanged || row.rawsChanged || row.effectsChanged) {
                    rows.append(row);
                }
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << rowQuery.executedQuery() << rowQuery.lastError().text();
    }
    endResetModel();
}

QStringList CuelistTableModel::getCueValue(const QString table, const QString valueTable, const int cueKey, const int groupKey) {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + table + ".id, ' ', " + table + ".label) FROM " + valueTable + ", " + table + " WHERE " + valueTable + ".item_key = :cue AND " + valueTable + ".foreignitem_key = :group AND " + valueTable + ".valueitem_key = " + table + ".key");
    query.bindValue(":cue", cueKey);
    query.bindValue(":group", groupKey);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QStringList();
    }
    QStringList values;
    while (query.next()) {
        values.append(query.value(0).toString());
    }
    return values;
}

void CuelistTableModel::setMode(Mode newMode) {
    mode = newMode;
    refresh();
}

void CuelistTableModel::setRowFilter(RowFilter newFilter) {
    filter = newFilter;
    refresh();
}

int CuelistTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return rows.length();
}

int CuelistTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant CuelistTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() > rows.length()) {
        return QVariant();
    }
    RowData row = rows.at(index.row());
    const int column = index.column();
    if (role == Qt::DisplayRole) {
        QString queryText;
        if (column == 0) {
            return row.name;
        } else if (column == 1) {
            return row.intensity;
        } else if (column == 2) {
            return row.color;
        } else if (column == 3) {
            return row.position;
        } else if (column == 4) {
            return row.raws;
        } else if (column == 5) {
            return row.effects;
        }
    } else if (role == Qt::BackgroundRole) {
        if ((column == 1) && row.intensityChanged) {
            return QColor(96, 96, 96);
        } else if ((column == 2) && row.colorChanged) {
            return QColor(96, 96, 96);
        } else if ((column == 3) && row.positionChanged) {
            return QColor(96, 96, 96);
        } else if ((column == 4) && row.rawsChanged) {
            return QColor(96, 96, 96);
        } else if ((column == 5) && row.effectsChanged) {
            return QColor(96, 96, 96);
        } else if (row.key == currentKey) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

QVariant CuelistTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (section == 0) {
            if (mode == CueMode) {
                return "Group";
            } else if (mode == GroupMode) {
                return "Cue";
            }
        } else if (section == 1) {
            return "Intensity";
        } else if (section == 2) {
            return "Color";
        } else if (section == 3) {
            return "Position";
        } else if (section == 4) {
            return "Raws";
        } else if (section == 5) {
            return "Effects";
        }
    }
    return QVariant();
}

QModelIndex CuelistTableModel::getCurrentRowIndex() {
    for (int row = 0; row < rows.length(); row++) {
        if (rows.at(row).key == currentKey) {
            return index(row, 0);
        }
    }
    return QModelIndex();
}
