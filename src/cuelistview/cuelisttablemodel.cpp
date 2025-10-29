/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelisttablemodel.h"

CuelistTableModel::CuelistTableModel() {
    setQuery("SELECT id, label FROM currentcuelist_cues ORDER BY sortkey");
    setHeaderData(0, Qt::Horizontal, "ID");
    setHeaderData(1, Qt::Horizontal, "Label");
}

void CuelistTableModel::reload() {
    currentCueId = QString();
    QSqlQuery query;
    if (query.exec("SELECT cues.id FROM cues, cuelists, currentitems WHERE currentitems.cuelist_key = cuelists.key AND cuelists.currentcue_key = cues.key")) {
        if (query.next()) {
            currentCueId = query.value(0).toString();
        }
    } else {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    refresh();
}

QVariant CuelistTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
        if (data(index.siblingAtColumn(0), Qt::DisplayRole).toString() == currentCueId) {
            return QColor(48, 48, 48);
        }
    }
    return QSqlQueryModel::data(index, role);
}
