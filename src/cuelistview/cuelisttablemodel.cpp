/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelisttablemodel.h"

CuelistTableModel::CuelistTableModel() {
    setCueMode(false);
}

void CuelistTableModel::refresh() {
    beginResetModel();
    currentRow = -1;
    QSqlQuery query;
    if (cueMode) {
        query.prepare("SELECT groups.sortkey FROM groups, currentitems WHERE currentitems.group_key = groups.key");
    } else {
        query.prepare("SELECT cues.sortkey FROM cues, cuelists, currentitems WHERE currentitems.cuelist_key = cuelists.key AND cuelists.currentcue_key = cues.key");
    }
    if (query.exec()) {
        if (query.next()) {
            currentRow = (query.value(0).toInt() - 1);
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    endResetModel();
}

void CuelistTableModel::setCueMode(bool newCueMode) {
    cueMode = newCueMode;
    refresh();
}

int CuelistTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    QSqlQuery query;
    if (cueMode) {
        query.prepare("SELECT COUNT(*) FROM groups");
    } else {
        query.prepare("SELECT COUNT(*) FROM currentcuelist_cues");
    }
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    return 0;
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
    if (role == Qt::DisplayRole) {
        const int column = index.column();
        QString queryText;
        if (cueMode) {
            auto createQuery = [] (const QString itemTable, const QString valueTable) {
                return "SELECT CONCAT(" + itemTable + ".id, ' ', " + itemTable + ".label) FROM " + itemTable + ", groups, " + valueTable + ", cuelists, currentitems WHERE groups.sortkey = :sortkey AND " + valueTable + ".foreignitem_key = groups.key AND " + valueTable + ".valueitem_key = " + itemTable + ".key AND " + valueTable + ".item_key = cuelists.currentcue_key AND cuelists.key = currentitems.cuelist_key";
            };
            if (column == 0) {
                queryText = "SELECT CONCAT(id, ' ', label) FROM groups WHERE sortkey = :sortkey";
            } else if (column == 1) {
                queryText = createQuery("intensities", "cue_group_intensities");
            } else if (column == 2) {
                queryText = createQuery("colors", "cue_group_colors");
            } else if (column == 3) {
                queryText = createQuery("positions", "cue_group_positions");
            } else if (column == 4) {
                queryText = createQuery("raws", "cue_group_raws");
            } else if (column == 5) {
                queryText = createQuery("effects", "cue_group_effects");
            }
        } else {
            auto createQuery = [] (const QString itemTable, const QString valueTable) {
                return "SELECT CONCAT(" + itemTable + ".id, ' ', " + itemTable + ".label) FROM " + itemTable + ", " + valueTable + ", currentitems, currentcuelist_cues WHERE currentcuelist_cues.sortkey = :sortkey AND " + valueTable + ".foreignitem_key = currentitems.group_key AND " + valueTable + ".valueitem_key = " + itemTable + ".key AND currentcuelist_cues.key = " + valueTable + ".item_key";
            };
            if (column == 0) {
                queryText = "SELECT CONCAT(id, ' ', label) FROM currentcuelist_cues WHERE sortkey = :sortkey";
            } else if (column == 1) {
                queryText = createQuery("intensities", "cue_group_intensities");
            } else if (column == 2) {
                queryText = createQuery("colors", "cue_group_colors");
            } else if (column == 3) {
                queryText = createQuery("positions", "cue_group_positions");
            } else if (column == 4) {
                queryText = createQuery("raws", "cue_group_raws");
            } else if (column == 5) {
                queryText = createQuery("effects", "cue_group_effects");
            }
        }
        if (!queryText.isEmpty()) {
            QSqlQuery query;
            query.prepare(queryText);
            query.bindValue(":sortkey", (index.row() + 1));
            if (query.exec()) {
                QStringList values;
                while (query.next()) {
                    values.append(query.value(0).toString());
                }
                return values.join(", ");
            } else {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            }
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.row() == currentRow) {
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
            if (cueMode) {
                return "Group";
            } else {
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
