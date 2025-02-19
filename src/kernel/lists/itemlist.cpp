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

template <class T> QList<QString> ItemList<T>::getIds() const {
    QList<QString> ids;
    for (T* item : items) {
        ids.append(item->id);
    }
    return ids;
}

template <class T> void ItemList<T>::setAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    if ((!attributes.contains(Keys::Attribute)) && (value.size() == 1) && (value.first() == Keys::Minus)) { // Delete Item
        QList<QString> existingIds;
        for (QString id : ids) {
            if (getItem(id) == nullptr) {
                kernel->terminal->warning("Couldn't delete " + singularItemName + " " + id + " because it doesn't exist.");
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
    } else if (attributes.isEmpty() && (value.size() > 1)) { // Copy Item
        value.removeFirst();
        T* sourceItem = getItem(kernel->keysToId(value));
        if (sourceItem == nullptr) {
            kernel->terminal->error("Can't copy " + pluralItemName + " because " + singularItemName + " " + sourceItem->id + " doesn't exist.");
            return;
        }
        int itemCounter = 0;
        for (QString id : ids) {
            if (getItem(id) != nullptr) {
                kernel->terminal->warning("Couldn't copy " + singularItemName + " " + id + " because target ID " + id + " is already used.");
            } else {
                T* item = new T(sourceItem);
                item->id = id;
                int position = findRow(id);
                beginInsertRows(QModelIndex(), position, position);
                items.insert(position, item);
                endInsertRows();
                itemCounter++;
            }
        }
        kernel->terminal->success("Copied " + QString::number(itemCounter) + " " + pluralItemName + " from " + singularItemName + " " + sourceItem->id + " .");
    } else if (attributes.value(Keys::Attribute) == IDATTRIBUTEID) {
        QString targetId = kernel->keysToId(value);
        QList<int> itemRows;
        for (QString id : ids) {
            int itemRow = getItemRow(id);
            if (itemRow < 0) {
                kernel->terminal->warning("Couldn't set " + singularItemName + " ID because " + singularItemName + " " + id + " doesn't exist.");
            } else {
                itemRows.append(itemRow);
            }
        }
        for (int itemRow : itemRows) {
            if (getItem(targetId) != nullptr) {
                kernel->terminal->warning("Couldn't set ID of " + singularItemName + " " + items[itemRow]->id + " because " + singularItemName + " ID " + targetId + " is already used.");
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
        kernel->terminal->success("Set ID of " + QString::number(itemRows.length()) + " " + pluralItemName + " to " + targetId + ".");
    } else if (attributes.value(Keys::Attribute) == LABELATTRIBUTEID) {
        if (!value.isEmpty()) {
            kernel->terminal->error("Label doesn't take any parameters.");
            return;
        }
        QList<int> itemRows;
        for (QString id : ids) {
            int itemRow = getItemRow(id);
            if (itemRow < 0) {
                kernel->terminal->warning("Couldn't label " + singularItemName + " " + id + " because it doesn't exist.");
            } else {
                itemRows.append(itemRow);
            }
        }
        for (int itemRow : itemRows) {
            items[itemRow]->label = text;
            emit dataChanged(index(itemRow, 0), index(itemRow, 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Labeled " + QString::number(itemRows.length()) + " " + pluralItemName + " as \"" + text + "\".");
    } else {
        setOtherAttribute(ids, attributes, value, text);
    }
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
    for (T* item : items) {
        QList<QString> indexIdParts = item->id.split(".");
        bool greaterId = true;
        bool sameBeginning = true;
        int minPartAmount = idParts.length();
        if (indexIdParts.length() < minPartAmount) {
            minPartAmount = indexIdParts.length();
        }
        for (int part = 0; part < minPartAmount; part++) {
            if ((idParts[part].toInt() < indexIdParts[part].toInt()) && sameBeginning) {
                greaterId = false;
            }
            if (idParts[part].toInt() != indexIdParts[part].toInt()) {
                sameBeginning = false;
            }
        }
        if (greaterId && (!sameBeginning || (idParts.length() > indexIdParts.length()))) {
            position++;
        }
    }
    return position;
}

template <class T> T* ItemList<T>::addItem(QString id) {
    T* item = new T(kernel);
    item->id = id;
    int row = findRow(id);
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    endInsertRows();
    return item;
}
