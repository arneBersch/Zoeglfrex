#include "itemtablemodel.h"

ItemTableModel::ItemTableModel() {}

void ItemTableModel::setTable(QString table, QStringList currentIds) {
    ids = currentIds;
    setQuery("SELECT id, label FROM " + table + " ORDER BY sortkey");
    setHeaderData(0, Qt::Horizontal, "ID");
    setHeaderData(1, Qt::Horizontal, "Label");
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
