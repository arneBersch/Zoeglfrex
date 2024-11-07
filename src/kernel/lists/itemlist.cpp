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

template <class T> bool ItemList<T>::labelItems(QList<QString> ids, QString label) {
    QList<Item*> items;
    for (QString id : ids) {
        Item* item = getItem(id);
        if (item == nullptr) {
            kernel->terminal->error("Couldn't label items because item with ID " + id + " doesn't exist.");
            return false;
        }
        items.append(item);
    }
    for (Item* item : items) {
        item->label = label;
    }
    kernel->terminal->success("Labeled " + QString::number(ids.length()) + " items as \"" + label + ".");
    return true;
}

template <class T> bool ItemList<T>::moveItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int itemRow = getItemRow(id);
        if (itemRow < 0) {
            kernel->terminal->error("Couldn't move items because item with ID " + id + " doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Couldn't move items because target ID " + targetId + " is already used.");
            return false;
        }
        T* item = items[itemRow];
        items.removeAt(itemRow);
        item->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, item);
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
