/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemtablemodel.h"

ItemTableModel::ItemTableModel() {}

void ItemTableModel::setTable(const QString newTable) {
    table = newTable;
    setQuery("SELECT id, label FROM " + table + " ORDER BY sortkey");
    setHeaderData(0, Qt::Horizontal, "ID");
    setHeaderData(1, Qt::Horizontal, "Label");
}

void ItemTableModel::setIds(const QStringList newIds) {
    ids = newIds;
}

QVariant ItemTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
        if (ids.contains(data(index.siblingAtColumn(0), Qt::DisplayRole).toString())) {
            return QColor(48, 48, 48);
        }
    }
    return QSqlQueryModel::data(index, role);
}

QModelIndex ItemTableModel::getLastSelectedRowIndex() {
    if (ids.isEmpty()) {
        return QModelIndex();
    }
    QSqlQuery sortkeyQuery;
    sortkeyQuery.prepare("SELECT sortkey FROM " + table + " WHERE id = :id");
    sortkeyQuery.bindValue(":id", ids.last());
    if (!sortkeyQuery.exec()) {
        return QModelIndex();
    }
    if (!sortkeyQuery.next()) {
        return QModelIndex();
    }
    const int row = sortkeyQuery.value(0).toInt() - 1;
    return index(row, 0);
}
