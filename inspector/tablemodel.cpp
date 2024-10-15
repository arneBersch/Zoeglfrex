#include "tablemodel.h"

TableModel::TableModel() : QAbstractTableModel(nullptr)
{
    //
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rowList.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant TableModel::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == 0) {
            return rowList[row].id;
        } else if (column == 1) {
            return rowList[row].label;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant TableModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == 0) {
            return "ID";
        } else if (column == 1) {
            return "Label";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

void TableModel::setRows(QList<TableRow> rows)
{
    rowList.clear();
    rowList.append(rows);
}
