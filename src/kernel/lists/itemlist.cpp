#include "itemlist.h"
#include "kernel/kernel.h"

template <class T> ItemList<T>::ItemList(QString singular, QString plural) {
    singularItemName = singular;
    pluralItemName = plural;
}

template <class T> T* ItemList<T>::getItem(QString id) const {
    int itemRow = getItemRow(id);
    if (itemRow < 0 || itemRow >= items.size()) {
        return nullptr;
    }
    return items[itemRow];
}

template <class T> int ItemList<T>::getItemRow(QString id) const {
    for (int itemRow = 0; itemRow < items.size(); itemRow++) {
        if (items[itemRow]->id == id) {
            return itemRow;
        }
    }
    return -1;
}

template <class T> void ItemList<T>::copyItems(QList<QString> ids, QString targetId) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->warning("Couldn't copy " + pluralItemName + " because " + singularItemName + " with ID " + id + " doesn't exist.");
        } else {
            itemRows.append(itemRow);
        }
    }
    for (int itemRow : itemRows) {
        if (getItem(targetId) != nullptr) {
            kernel->terminal->warning("Couldn't copy " + singularItemName + " " + items[itemRow]->name() + " because target ID " + targetId + " is already used.");
        } else {
            T* targetItem = new T(items[itemRow]);
            targetItem->id = targetId;
            int position = findRow(targetId);
            beginInsertRows(QModelIndex(), position, position);
            items.insert(position, targetItem);
            endInsertRows();
        }
    }
    kernel->terminal->success("Copied " + QString::number(itemRows.length()) + " " + pluralItemName + " to \"" + targetId + ".");
}

template <class T> void ItemList<T>::deleteItems(QList<QString> ids) {
    QList<QString> existingIds;
    for (QString id : ids) {
        if (getItem(id) == nullptr) {
            kernel->terminal->warning("Couldn't delete " + pluralItemName + " because " + singularItemName + " with ID " + id + " doesn't exist.");
        } else {
            existingIds.append(id);
        }
    }
    for (QString id : existingIds) {
        int itemRow = getItemRow(id);
        T *item = items[itemRow];
        beginRemoveRows(QModelIndex(), itemRow, itemRow);
        items.removeAt(itemRow);
        endRemoveRows();
        delete item;
    }
    kernel->terminal->success("Deleted " + QString::number(existingIds.length()) + " " + pluralItemName + ".");
}

template <class T> void ItemList<T>::labelItems(QList<QString> ids, QString label) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->warning("Couldn't label " + pluralItemName + " because " + singularItemName + " with ID " + id + " doesn't exist.");
        } else {
            itemRows.append(itemRow);
        }
    }
    for (int itemRow : itemRows) {
        items[itemRow]->label = label;
        emit dataChanged(index(itemRow, 0), index(itemRow, 0), {Qt::DisplayRole, Qt::EditRole});
    }
    kernel->terminal->success("Labeled " + QString::number(itemRows.length()) + " " + pluralItemName + " as \"" + label + "\".");
}

template <class T> void ItemList<T>::moveItems(QList<QString> ids, QString targetId) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->warning("Couldn't move " + pluralItemName + " because " + singularItemName + " with ID " + id + " doesn't exist.");
        } else {
            itemRows.append(itemRow);
        }
    }
    for (int itemRow : itemRows) {
        if (getItem(targetId) != nullptr) {
            kernel->terminal->warning("Couldn't move " + singularItemName + " " + items[itemRow]->name() + " because target ID " + targetId + " is already used.");
        } else {
            T* item = items[itemRow];
            beginRemoveRows(QModelIndex(), itemRow, itemRow);
            items.removeAt(itemRow);
            endRemoveRows();
            item->id = targetId;
            int row = findRow(targetId);
            beginInsertRows(QModelIndex(), row, row);
            items.insert(row, item);
            endInsertRows();
        }
    }
    kernel->terminal->success("Moved " + QString::number(itemRows.length()) + " " + pluralItemName + " to " + targetId + ".");
}

template <class T> int ItemList<T>::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return items.size();
}

template <class T> int ItemList<T>::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

template <class T> QVariant ItemList<T>::data(const QModelIndex &index, const int role) const {
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        return items[row]->name();
    }
    return QVariant();
}

template <class T> int ItemList<T>::findRow(QString id) {
    int position = 0;
    QList<QString> idParts = id.split(".");
    for (int index=0; index < items.size(); index++) {
        QList<QString> indexIdParts = items[index]->id.split(".");
        if (indexIdParts[0].toInt() < idParts[0].toInt()) {
            position++;
        } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
            if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                position++;
            } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                    position++;
                } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                    if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                        position++;
                    } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                        if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                            position++;
                        }
                    }
                }
            }
        }
    }
    return position;
}

template <class T> T* ItemList<T>::recordItem(QString id) {
    T* item = new T(kernel);
    item->id = id;
    int row = findRow(id);
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    endInsertRows();
    return item;
}
