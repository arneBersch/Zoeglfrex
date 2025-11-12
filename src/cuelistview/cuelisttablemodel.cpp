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
    currentKey = -1;
    QSqlQuery currentKeyQuery;
    if (mode == CueMode) {
        currentKeyQuery.prepare("SELECT group_key FROM currentitems");
    } else if (mode == GroupMode) {
        currentKeyQuery.prepare("SELECT key FROM currentcue");
    }
    if (currentKeyQuery.exec()) {
        if (currentKeyQuery.next()) {
            currentKey = currentKeyQuery.value(0).toInt();
        }
    } else {
        qWarning() << Q_FUNC_INFO << currentKeyQuery.executedQuery() << currentKeyQuery.lastError().text();
    }
    rows.clear();
    QSqlQuery rowQuery;
    if (mode == CueMode) {
        rowQuery.prepare("SELECT key, CONCAT(id, ' ', label) FROM groups");
    } else if (mode == GroupMode) {
        rowQuery.prepare("SELECT key, CONCAT(id, ' ', label) FROM currentcuelist_cues");
    }
    if (rowQuery.exec()) {
        while (rowQuery.next()) {
            RowData row;
            row.key = rowQuery.value(0).toInt();
            row.name = rowQuery.value(1).toString();
            int cueKey = currentKey;
            int groupKey = currentKey;
            if (mode == CueMode) {
                cueKey = row.key;
            } else if (mode == GroupMode) {
                groupKey = row.key;
            }
            auto getValue = [cueKey, groupKey] (const QString table)->QString {
                QSqlQuery query;
                query.prepare("SELECT CONCAT(" + table + ".id, ' ', " + table + ".label) FROM cue_group_" + table + ", " + table + " WHERE cue_group_" + table + ".item_key = :cue AND cue_group_" + table + ".foreignitem_key = :group AND cue_group_" + table + ".valueitem_key = " + table + ".key");
                query.bindValue(":cue", cueKey);
                query.bindValue(":group", groupKey);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                    return QString();
                }
                QStringList values;
                while (query.next()) {
                    values.append(query.value(0).toString());
                }
                return values.join(", ");
            };
            row.intensity = getValue("intensities");
            row.color = getValue("colors");
            row.position = getValue("positions");
            row.raws = getValue("raws");
            row.effects = getValue("effects");
            rows.append(row);
        }
    } else {
        qWarning() << Q_FUNC_INFO << rowQuery.executedQuery() << rowQuery.lastError().text();
    }
    endResetModel();
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
    if (role == Qt::DisplayRole) {
        const int column = index.column();
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
        if (row.key == currentKey) {
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
