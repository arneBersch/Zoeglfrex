#include "itemlist.h"
#include "kernel/kernel.h"

template <class T> ItemList<T>::ItemList() {
    //
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

template <class T> QList<QString> ItemList<T>::getIds() const {
    QList<QString> ids;
    for (T *item : items) {
        ids.append(item->id);
    }
    return ids;
}

template <class T> bool ItemList<T>::copyItems(QList<QString> ids, QString targetId) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->error("Couldn't copy items because item with ID " + id + " doesn't exist.");
            return false;
        }
        itemRows.append(itemRow);
    }
    for (int itemRow : itemRows) {
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Couldn't copy item " + items[itemRow]->name() + " because target ID " + targetId + " is already used.");
        } else {
            T* targetItem = items[itemRow]->copy();
            targetItem->id = targetId;
            int position = 0;
            for (int index=0; index < items.size(); index++) {
                if (greaterId(items[index]->id, targetId)) {
                    position++;
                }
            }
            beginInsertRows(QModelIndex(), position, position);
            items.insert(position, targetItem);
            endInsertRows();
        }
    }
    kernel->terminal->success("Copied " + QString::number(ids.length()) + " items to \"" + targetId + ".");
    return true;
}

template <class T> bool ItemList<T>::deleteItems(QList<QString> ids) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->error("Couldn't delete items because item with ID " + id + " doesn't exist.");
            return false;
        }
        itemRows.append(itemRow);
    }
    for (int itemRow : itemRows) {
        T *item= items[itemRow];
        items.removeAt(itemRow);
        delete item;
    }
    kernel->terminal->success("Deleted " + QString::number(ids.length()) + " items.");
    return true;
}

template <class T> bool ItemList<T>::labelItems(QList<QString> ids, QString label) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->error("Couldn't label items because item with ID " + id + " doesn't exist.");
            return false;
        }
        itemRows.append(itemRow);
    }
    for (int itemRow : itemRows) {
        items[itemRow]->label = label;
        emit dataChanged(index(itemRow, 0), index(itemRow, 0), {Qt::DisplayRole, Qt::EditRole});
    }
    kernel->terminal->success("Labeled " + QString::number(ids.length()) + " items as \"" + label + ".");
    return true;
}

template <class T> bool ItemList<T>::moveItems(QList<QString> ids, QString targetId) {
    QList<int> itemRows;
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->error("Couldn't move items because item with ID " + id + " doesn't exist.");
            return false;
        }
        itemRows.append(itemRow);
    }
    for (int itemRow : itemRows) {
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Couldn't move item " + items[itemRow]->name() + " because target ID " + targetId + " is already used.");
        } else {
            T* item = items[itemRow];
            beginRemoveRows(QModelIndex(), itemRow, itemRow);
            items.removeAt(itemRow);
            endRemoveRows();
            item->id = targetId;
            int position = 0;
            for (int index=0; index < items.size(); index++) {
                if (greaterId(items[index]->id, targetId)) {
                    position++;
                }
            }
            beginInsertRows(QModelIndex(), position, position);
            items.insert(position, item);
            endInsertRows();
        }
    }
    kernel->terminal->success("Moved " + QString::number(ids.length()) + " items to " + targetId + ".");
    return true;
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

template <class T> bool ItemList<T>::greaterId(QString firstId, QString secondId) {
    QList<QString> firstIdParts = firstId.split(".");
    QList<QString> secondIdParts = secondId.split(".");
    if (firstIdParts[0].toInt() < secondIdParts[0].toInt()) {
        return true;
    } else if (firstIdParts[0].toInt() == secondIdParts[0].toInt()) {
        if (firstIdParts[1].toInt() < secondIdParts[1].toInt()) {
            return true;
        } else if (firstIdParts[1].toInt() == secondIdParts[1].toInt()) {
            if (firstIdParts[2].toInt() < secondIdParts[2].toInt()) {
                return true;
            } else if (firstIdParts[2].toInt() == secondIdParts[2].toInt()) {
                if (firstIdParts[3].toInt() < secondIdParts[3].toInt()) {
                    return true;
                } else if (firstIdParts[3].toInt() == secondIdParts[3].toInt()) {
                    if (firstIdParts[4].toInt() < secondIdParts[4].toInt()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
