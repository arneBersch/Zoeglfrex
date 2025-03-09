/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemlist.h"
#include "kernel/kernel.h"

template <class T> ItemList<T>::ItemList(int key, QString singular, QString plural) {
    itemKey = key;
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
    } else if (attributes.isEmpty() && (value.size() > 1) && (value.first() == itemKey)) { // Copy Item
        value.removeFirst();
        T* sourceItem = getItem(kernel->keysToId(value));
        if (sourceItem == nullptr) {
            kernel->terminal->error("Can't copy " + pluralItemName + " because " + singularItemName + " " + kernel->keysToId(value) + " doesn't exist.");
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
    } else if (fixtureListAttributes.contains(attributes.value(Keys::Attribute))) {
        QList<Fixture*> fixtureSelection;
        if (!value.isEmpty()) {
            if (value.first() != Keys::Fixture) {
                kernel->terminal->error("Seting " + singularItemName + " Fixtures requires Fixtures.");
                return;
            }
            value.removeFirst();
            QList<QString> fixtureIds = kernel->keysToSelection(value, Keys::Fixture);
            if (fixtureIds.isEmpty()) {
                kernel->terminal->error("Can't set " + singularItemName + " Fixtures because of an invalid Fixture selection.");
                return;
            }
            for (QString fixtureId : fixtureIds) {
                Fixture* fixture = kernel->fixtures->getItem(fixtureId);
                if (fixture == nullptr) {
                    kernel->terminal->warning("Can't add Fixture " + fixtureId + " to " + singularItemName + " because it doesn't exist.");
                } else {
                    fixtureSelection.append(fixture);
                }
            }
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->fixtureListAttributes[attributes.value(Keys::Attribute)] = fixtureSelection;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Fixtures of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(fixtureSelection.length()) + " Fixtures.");
    } else if (fixtureSpecificFloatAttributes.contains(attributes.value(Keys::Attribute)) && attributes.contains(Keys::Fixture)) {
        FloatAttribute floatAttribute = fixtureSpecificFloatAttributes.value(attributes.value(Keys::Attribute));
        Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
        if (fixture == nullptr) {
            kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + attributes.value(Keys::Fixture) + " because Fixture doesn't exist.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->fixtureSpecificFloatAttributes[attributes.value(Keys::Attribute)].remove(fixture);
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            kernel->terminal->success("Removed " + floatAttribute.name + " of Fixture " + fixture->id + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
        } else {
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            int newValue = kernel->keysToValue(value);
            if (newValue < 0) {
                kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->id+ " because no valid number was given.");
                return;
            }
            if (negativeValue) {
                newValue *= -1;
            }
            if ((newValue < floatAttribute.min) || (newValue > floatAttribute.max)) {
                kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->id + " because " + floatAttribute.name + " has to be a number between " + QString::number(floatAttribute.min) + floatAttribute.unit + " and " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                return;
            }
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->fixtureSpecificFloatAttributes[attributes.value(Keys::Attribute)][fixture] = newValue;
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixutre " + fixture->id + " to " + QString::number(newValue) + floatAttribute.unit + ".");
        }
    } else if (intAttributes.contains(attributes.value(Keys::Attribute))) {
        IntAttribute intAttribute = intAttributes.value(attributes.value(Keys::Attribute));
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        int newValue = kernel->keysToValue(value);
        if (newValue < 0) {
            kernel->terminal->error("Can't set " + singularItemName + " " + intAttribute.name + " because no valid number was given.");
            return;
        }
        if (negativeValue) {
            newValue *= -1;
        }
        if ((newValue < intAttribute.min) || (newValue > intAttribute.max)) {
            kernel->terminal->error("Can't set " + singularItemName + " " + intAttribute.name + " because " + intAttribute.name + " has to be a number between " + QString::number(intAttribute.min) + intAttribute.unit + " and " + QString::number(intAttribute.max) + intAttribute.unit + ".");
            return;
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->intAttributes[attributes.value(Keys::Attribute)] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + intAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + intAttribute.unit + ".");
    } else if (floatAttributes.contains(attributes.value(Keys::Attribute))) {
        FloatAttribute floatAttribute = floatAttributes.value(attributes.value(Keys::Attribute));
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        int newValue = kernel->keysToValue(value);
        if (newValue < 0) {
            kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " because no valid number was given.");
            return;
        }
        if (negativeValue) {
            newValue *= -1;
        }
        if ((newValue < floatAttribute.min) || (newValue > floatAttribute.max)) {
            kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " because " + floatAttribute.name + " has to be a number between " + QString::number(floatAttribute.min) + floatAttribute.unit + " and " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
            return;
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->floatAttributes[attributes.value(Keys::Attribute)] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + floatAttribute.unit + ".");
    } else if (angleAttributes.contains(attributes.value(Keys::Attribute))) {
        AngleAttribute angleAttribute = angleAttributes.value(attributes.value(Keys::Attribute));
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        float newValue = kernel->keysToValue(value);
        if (newValue < 0) {
            kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " because no valid number was given.");
            return;
        }
        while (newValue >= 360) {
            newValue -= 360;
        }
        if (negativeValue && (newValue > 0)) {
            newValue = 360 - newValue;
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->angleAttributes[attributes.value(Keys::Attribute)] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + "°.");
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
    for (QString attribute : fixtureListAttributes.keys()) {
        item->fixtureListAttributes[attribute] = fixtureListAttributes.value(attribute).value;
    }
    for (QString attribute : intAttributes.keys()) {
        item->intAttributes[attribute] = intAttributes.value(attribute).value;
    }
    for (QString attribute : floatAttributes.keys()) {
        item->floatAttributes[attribute] = floatAttributes.value(attribute).value;
    }
    for (QString attribute : fixtureSpecificFloatAttributes.keys()) {
        item->fixtureSpecificFloatAttributes[attribute] = QMap<Fixture*, float>();
    }
    for (QString attribute : angleAttributes.keys()) {
        item->angleAttributes[attribute] = angleAttributes.value(attribute).value;
    }
    int row = findRow(id);
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    endInsertRows();
    return item;
}
