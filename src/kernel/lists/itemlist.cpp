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
    stringAttributes[LABELATTRIBUTEID] = {"Label", QString(), QString()};
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
    QString attribute = attributes.value(Keys::Attribute);
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
    } else if (attribute == IDATTRIBUTEID) { // Move Item
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
    } else if (stringAttributes.contains(attribute)){ // String Attribute
        StringAttribute stringAttribute = stringAttributes.value(attribute);
        if (!value.isEmpty()) {
            kernel->terminal->error(singularItemName + " " + stringAttribute.name + " doesn't take a value.");
            return;
        }
        if (!stringAttribute.regex.isEmpty() && !text.contains(QRegularExpression(stringAttribute.regex))) {
            kernel->terminal->error("Didn't set " + singularItemName + " " + stringAttribute.name + " because " + stringAttribute.name + " \"" + text + "\" are not valid.");
            return;
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->stringAttributes[attribute] = text;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + stringAttribute.name + " of " + QString::number(ids.size()) + " " + pluralItemName + " to \"" + text + "\".");
    } else if (fixtureSpecificFloatAttributes.contains(attribute) && attributes.contains(Keys::Fixture)) { // Fixture Specific Float Attribute
        FloatAttribute floatAttribute = fixtureSpecificFloatAttributes.value(attribute);
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
                item->fixtureSpecificFloatAttributes[attribute].remove(fixture);
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            kernel->terminal->success("Removed " + floatAttribute.name + " of Fixture " + fixture->id + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
        } else {
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->id + " because no valid number was given.");
                    return;
                }
                if (negativeValue) {
                    newValue *= -1;
                }
            } else {
                bool ok = true;
                newValue = text.toFloat(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->id + " because no valid number was given.");
                    return;
                }
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
                item->fixtureSpecificFloatAttributes[attribute][fixture] = newValue;
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixutre " + fixture->id + " to " + QString::number(newValue) + floatAttribute.unit + ".");
        }
    } else if (intAttributes.contains(attribute)) { // Integer Attribute
        IntAttribute intAttribute = intAttributes.value(attribute);
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        int newValue = kernel->keysToValue(value);
        if (text.isEmpty()) {
            if (newValue < 0) {
                kernel->terminal->error("Can't set " + singularItemName + " " + intAttribute.name + " because no valid number was given.");
                return;
            }
            if (negativeValue) {
                newValue *= -1;
            }
        } else {
            bool ok = true;
            newValue = text.toInt(&ok);
            if (!ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + intAttribute.name + " because no valid number was given.");
                return;
            }
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
            item->intAttributes[attribute] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + intAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + intAttribute.unit + ".");
    } else if (floatAttributes.contains(attribute)) { // Float Attribute
        FloatAttribute floatAttribute = floatAttributes.value(attribute);
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        int newValue = kernel->keysToValue(value);
        if (text.isEmpty()) {
            if (newValue < 0) {
                kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " because no valid number was given.");
                return;
            }
            if (negativeValue) {
                newValue *= -1;
            }
        } else {
            bool ok = true;
            newValue = text.toInt(&ok);
            if (!ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " because no valid number was given.");
                return;
            }
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
            item->floatAttributes[attribute] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + floatAttribute.unit + ".");
    } else if (angleAttributes.contains(attribute)) { // Angle Attribute
        AngleAttribute angleAttribute = angleAttributes.value(attribute);
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        float newValue = kernel->keysToValue(value);
        if (!text.isEmpty()) {
            bool ok = true;
            newValue = text.toFloat(&ok);
            if (!ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " because no valid number was given.");
                return;
            }
        }
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
            item->angleAttributes[attribute] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + "°.");
    } else {
        kernel->terminal->error("Can't set " + singularItemName + " Attribute " + attributes.value(Keys::Attribute) + ".");
    }
}

template <class T> void ItemList<T>::saveItemsToFile(QXmlStreamWriter* fileStream) {
    fileStream->writeStartElement(pluralItemName);
    for (T* item : items) {
        fileStream->writeStartElement(singularItemName);
        fileStream->writeAttribute("ID", item->id);
        item->writeAttributesToFile(fileStream);
        fileStream->writeEndElement();
    }
    fileStream->writeEndElement();
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
    for (QString attribute : stringAttributes.keys()) {
        item->stringAttributes[attribute] = stringAttributes.value(attribute).value;
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
