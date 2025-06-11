/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemtable.h"
#include "kernel/kernel.h"

template <class T> ItemTable<T>::ItemTable(Kernel* core, int key, QString singular, QString plural) {
    kernel = core;
    itemKey = key;
    singularItemName = singular;
    pluralItemName = plural;
    stringAttributes[kernel->LABELATTRIBUTEID] = {"Label", QString(), QString()};
}

template <class T> ItemTable<T>::ItemTable(ItemTable<T>* table) {
    kernel = table->kernel;
    stringAttributes = table->stringAttributes;
    intAttributes = table->intAttributes;
    floatAttributes = table->floatAttributes;
    modelSpecificFloatAttributes = table->modelSpecificFloatAttributes;
    fixtureSpecificFloatAttributes = table->fixtureSpecificFloatAttributes;
    angleAttributes = table->angleAttributes;
    modelSpecificAngleAttributes = table->modelSpecificAngleAttributes;
    fixtureSpecificAngleAttributes = table->fixtureSpecificAngleAttributes;
    boolAttributes = table->boolAttributes;
    rawListAttributes = table->rawListAttributes;
    items = QList<T*>();
    for (T* tableItem : table->items) {
        T* item = new T(tableItem);
        item->id = tableItem->id;
        items.append(item);
    }
}

template <class T> ItemTable<T>::~ItemTable() {
    reset();
}

template <class T> T* ItemTable<T>::getItem(QString id) const {
    int itemRow = getItemRow(id);
    if (itemRow < 0 || itemRow >= items.size()) {
        return nullptr;
    }
    return items[itemRow];
}

template <class T> int ItemTable<T>::getItemRow(QString id) const {
    for (int itemRow = 0; itemRow < items.size(); itemRow++) {
        if (items[itemRow]->id == id) {
            return itemRow;
        }
    }
    return -1;
}

template <class T> QStringList ItemTable<T>::getIds() const {
    QStringList ids;
    for (T* item : items) {
        ids.append(item->id);
    }
    return ids;
}

template <class T> void ItemTable<T>::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if ((!attributes.contains(Keys::Attribute)) && (value.size() == 1) && (value.first() == Keys::Minus)) { // Delete Item
        QStringList existingIds;
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
        if (existingIds.size() == 1) {
            kernel->terminal->success("Deleted " + singularItemName + " " + existingIds.first() + ".");
        } else {
            kernel->terminal->success("Deleted " + QString::number(existingIds.length()) + " " + pluralItemName + ".");
        }
    } else if (attributes.isEmpty() && (value.size() > 1) && (value.first() == itemKey)) { // Copy Item
        value.removeFirst();
        T* sourceItem = getItem(kernel->terminal->keysToId(value));
        if (sourceItem == nullptr) {
            kernel->terminal->error("Can't copy " + pluralItemName + " because " + singularItemName + " " + kernel->terminal->keysToId(value) + " doesn't exist.");
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
        kernel->terminal->success("Copied " + QString::number(itemCounter) + " " + pluralItemName + " from " + singularItemName + " " + sourceItem->name() + " .");
    } else if (attribute == kernel->IDATTRIBUTEID) { // Move Item
        QString targetId = kernel->terminal->keysToId(value);
        if (targetId.isEmpty()) {
            kernel->terminal->error("Couldn't set " + singularItemName + " ID because the given ID is not valid.");
            return;
        }
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
                kernel->terminal->warning("Couldn't set ID of " + singularItemName + " " + items[itemRow]->name() + " because " + singularItemName + " ID " + targetId + " is already used.");
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
        if (ids.size() == 1) {
            kernel->terminal->success("Set " + stringAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to \"" + text + "\".");
        } else {
            kernel->terminal->success("Set " + stringAttribute.name + " of " + QString::number(ids.size()) + " " + pluralItemName + " to \"" + text + "\".");
        }
    } else if (intAttributes.contains(attribute)) { // Integer Attribute
        IntAttribute intAttribute = intAttributes.value(attribute);
        bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
        if (difference) {
            value.removeFirst();
        }
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        int newValue = kernel->terminal->keysToValue(value);
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
        if (difference) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->intAttributes[attribute] += newValue;
                if (item->intAttributes.value(attribute) < intAttribute.min) {
                    item->intAttributes[attribute] = intAttribute.min;
                    kernel->terminal->warning("Can't decrease " +  intAttribute.name + " of " + singularItemName + " " + item->name() + " because " + intAttribute.name + " must be at least " + QString::number(intAttribute.min) + intAttribute.unit + ".");
                } else if (item->intAttributes.value(attribute) > intAttribute.max) {
                    item->intAttributes[attribute] = intAttribute.max;
                    kernel->terminal->warning("Can't increase " +  intAttribute.name + " of " + singularItemName + " " + item->name() + " because " + intAttribute.name + " must not exceed " + QString::number(intAttribute.max) + intAttribute.unit + ".");
                }
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + intAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(getItem(ids.first())->intAttributes.value(attribute)) + intAttribute.unit + ".");
            } else {
                kernel->terminal->success("Changed " + intAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
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
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + intAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(newValue) + intAttribute.unit + ".");
            } else {
                kernel->terminal->success("Set " + intAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + intAttribute.unit + ".");
            }
        }
    } else if (modelSpecificFloatAttributes.contains(attribute) && attributes.contains(Keys::Model)) { // Model Specific Float Attribute
        FloatAttribute floatAttribute = modelSpecificFloatAttributes.value(attribute);
        Model* model = kernel->models->getItem(attributes.value(Keys::Model));
        if (model == nullptr) {
            kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Model " + attributes.value(Keys::Model) + " because Model doesn't exist.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->modelSpecificFloatAttributes[attribute].remove(model);
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Model " + model->name() + " in " + singularItemName + " " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Model " + model->name() + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->terminal->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Model " + model->name() + " because no valid number was given.");
                    return;
                }
                if (negativeValue) {
                    newValue *= -1;
                }
            } else {
                bool ok = true;
                newValue = text.toFloat(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Model " + model->name() + " because no valid number was given.");
                    return;
                }
            }
            if (difference) {
                for (QString id : ids) {
                    T* item = getItem(id);
                    if (item == nullptr) {
                        item = addItem(id);
                    }
                    if (!item->modelSpecificFloatAttributes.value(attribute).contains(model)) {
                        item->modelSpecificFloatAttributes[attribute][model] = item->floatAttributes.value(attribute);
                    }
                    item->modelSpecificFloatAttributes[attribute][model] += newValue;
                    if (item->modelSpecificFloatAttributes.value(attribute).value(model) < floatAttribute.min) {
                        item->modelSpecificFloatAttributes[attribute][model] = floatAttribute.min;
                        kernel->terminal->warning("Can't decrease " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must be at least " + QString::number(floatAttribute.min) + floatAttribute.unit + ".");
                    } else if (item->modelSpecificFloatAttributes.value(attribute).value(model) > floatAttribute.max) {
                        item->modelSpecificFloatAttributes[attribute][model] = floatAttribute.max;
                        kernel->terminal->warning("Can't increase " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must not exceed " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                    }
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Model " + model->name() + + " to " + QString::number(getItem(ids.first())->modelSpecificFloatAttributes.value(attribute).value(model)) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Changed " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Model " + model->name() + ".");
                }
            } else {
                if ((newValue < floatAttribute.min) || (newValue > floatAttribute.max)) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Model " + model->name() + " because " + floatAttribute.name + " has to be a number between " + QString::number(floatAttribute.min) + floatAttribute.unit + " and " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                    return;
                }
                for (QString id : ids) {
                    T* item = getItem(id);
                    if (item == nullptr) {
                        item = addItem(id);
                    }
                    item->modelSpecificFloatAttributes[attribute][model] = newValue;
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Model " + model->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Model " + model->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
                }
            }
        }
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
            if (ids.size() == 1) {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Fixture " + fixture->name() + " in " + singularItemName + " " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed " + floatAttribute.name + " of Fixture " + fixture->name() + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->terminal->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->name() + " because no valid number was given.");
                    return;
                }
                if (negativeValue) {
                    newValue *= -1;
                }
            } else {
                bool ok = true;
                newValue = text.toFloat(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->name() + " because no valid number was given.");
                    return;
                }
            }
            if (difference) {
                for (QString id : ids) {
                    T* item = getItem(id);
                    if (item == nullptr) {
                        item = addItem(id);
                    }
                    if (!item->fixtureSpecificFloatAttributes.value(attribute).contains(fixture)) {
                        item->fixtureSpecificFloatAttributes[attribute][fixture] = item->floatAttributes.value(attribute);
                        if (item->modelSpecificFloatAttributes.value(attribute).contains(fixture->model)) {
                            item->fixtureSpecificFloatAttributes[attribute][fixture] = item->modelSpecificFloatAttributes.value(attribute).value(fixture->model);
                        }
                    }
                    item->fixtureSpecificFloatAttributes[attribute][fixture] += newValue;
                    if (item->fixtureSpecificFloatAttributes.value(attribute).value(fixture) < floatAttribute.min) {
                        item->fixtureSpecificFloatAttributes[attribute][fixture] = floatAttribute.min;
                        kernel->terminal->warning("Can't decrease " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must be at least " + QString::number(floatAttribute.min) + floatAttribute.unit + ".");
                    } else if (item->fixtureSpecificFloatAttributes.value(attribute).value(fixture) > floatAttribute.max) {
                        item->fixtureSpecificFloatAttributes[attribute][fixture] = floatAttribute.max;
                        kernel->terminal->warning("Can't increase " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must not exceed " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                    }
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Fixture " + fixture->name() + + " to " + QString::number(getItem(ids.first())->fixtureSpecificFloatAttributes.value(attribute).value(fixture)) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Changed " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixture " + fixture->name() + ".");
                }
            } else {
                if ((newValue < floatAttribute.min) || (newValue > floatAttribute.max)) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " of Fixture " + fixture->name() + " because " + floatAttribute.name + " has to be a number between " + QString::number(floatAttribute.min) + floatAttribute.unit + " and " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
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
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Fixture " + fixture->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
                } else {
                    kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixture " + fixture->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
                }
            }
        }
    } else if (floatAttributes.contains(attribute)) { // Float Attribute
        FloatAttribute floatAttribute = floatAttributes.value(attribute);
        bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
        if (difference) {
            value.removeFirst();
        }
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        float newValue = kernel->terminal->keysToValue(value);
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
            newValue = text.toFloat(&ok);
            if (!ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + floatAttribute.name + " because no valid number was given.");
                return;
            }
        }
        if (difference) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->floatAttributes[attribute] += newValue;
                if (item->floatAttributes.value(attribute) < floatAttribute.min) {
                    item->floatAttributes[attribute] = floatAttribute.min;
                    kernel->terminal->warning("Can't decrease " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must be at least " + QString::number(floatAttribute.min) + floatAttribute.unit + ".");
                } else if (item->floatAttributes.value(attribute) > floatAttribute.max) {
                    item->floatAttributes[attribute] = floatAttribute.max;
                    kernel->terminal->warning("Can't increase " +  floatAttribute.name + " of " + singularItemName + " " + item->name() + " because " + floatAttribute.name + " must not exceed " + QString::number(floatAttribute.max) + floatAttribute.unit + ".");
                }
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(getItem(ids.first())->floatAttributes.value(attribute)) + floatAttribute.unit + ".");
            } else {
                kernel->terminal->success("Changed " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
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
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + floatAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(newValue) + floatAttribute.unit + ".");
            } else {
                kernel->terminal->success("Set " + floatAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + floatAttribute.unit + ".");
            }
        }
    } else if (modelSpecificAngleAttributes.contains(attribute) && attributes.contains(Keys::Model)) { // Model Specific Angle Attribute
        AngleAttribute angleAttribute = modelSpecificAngleAttributes.value(attribute);
        Model* model = kernel->models->getItem(attributes.value(Keys::Model));
        if (model == nullptr) {
            kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " of Model " + attributes.value(Keys::Model) + " because Model doesn't exist.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->modelSpecificAngleAttributes[attribute].remove(model);
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed " + angleAttribute.name + " of Model " + model->name() + " in " + singularItemName + " " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed " + angleAttribute.name + " of Model " + model->name() + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->terminal->keysToValue(value);
            bool ok = true;
            if (!text.isEmpty()) {
                newValue = text.toFloat(&ok);
            }
            if ((newValue < 0) || !ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " because no valid number was given.");
                return;
            }
            if (difference) {
                for (QString id : ids) {
                    T* item = getItem(id);
                    if (item == nullptr) {
                        item = addItem(id);
                    }
                    if (!item->modelSpecificAngleAttributes.value(attribute).contains(model)) {
                        item->modelSpecificAngleAttributes[attribute][model] = item->angleAttributes.value(attribute);
                    }
                    if (!negativeValue) {
                        item->modelSpecificAngleAttributes[attribute][model] += newValue;
                    } else {
                        item->modelSpecificAngleAttributes[attribute][model] -= newValue;
                    }
                    while (item->modelSpecificAngleAttributes.value(attribute).value(model) >= 360.0) {
                        item->modelSpecificAngleAttributes[attribute][model] -= 360.0;
                    }
                    while (item->modelSpecificAngleAttributes.value(attribute).value(model) < 0.0) {
                        item->modelSpecificAngleAttributes[attribute][model] += 360.0;
                    }
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Model " + model->name() + + " to " + QString::number(getItem(ids.first())->modelSpecificAngleAttributes.value(attribute).value(model)) + "°.");
                } else {
                    kernel->terminal->success("Changed " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Model " + model->name() + ".");
                }
            } else {
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
                    item->modelSpecificAngleAttributes[attribute][model] = newValue;
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Model " + model->name() + " to " + QString::number(newValue) + "°.");
                } else {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Model " + model->name() + " to " + QString::number(newValue) + "°.");
                }
            }
        }
    } else if (fixtureSpecificAngleAttributes.contains(attribute) && attributes.contains(Keys::Fixture)) { // Fixture Specific Angle Attribute
        AngleAttribute angleAttribute = fixtureSpecificAngleAttributes.value(attribute);
        Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
        if (fixture == nullptr) {
            kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " of Fixture " + attributes.value(Keys::Fixture) + " because Fixture doesn't exist.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->fixtureSpecificAngleAttributes[attribute].remove(fixture);
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed " + angleAttribute.name + " of Fixture " + fixture->name() + " in " + singularItemName + " " + getItem(ids.first())->name() + ".");
            } else {
                kernel->terminal->success("Removed " + angleAttribute.name + " of Fixture " + fixture->name() + " in " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            float newValue = kernel->terminal->keysToValue(value);
            bool ok = true;
            if (!text.isEmpty()) {
                newValue = text.toFloat(&ok);
            }
            if ((newValue < 0) || !ok) {
                kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " because no valid number was given.");
                return;
            }
            if (difference) {
                for (QString id : ids) {
                    T* item = getItem(id);
                    if (item == nullptr) {
                        item = addItem(id);
                    }
                    if (!item->fixtureSpecificAngleAttributes.value(attribute).contains(fixture)) {
                        item->fixtureSpecificAngleAttributes[attribute][fixture] = item->angleAttributes.value(attribute);
                        if (item->modelSpecificAngleAttributes.value(attribute).contains(fixture->model)) {
                            item->fixtureSpecificAngleAttributes[attribute][fixture] = item->modelSpecificAngleAttributes.value(attribute).value(fixture->model);
                        }
                    }
                    if (!negativeValue) {
                        item->fixtureSpecificAngleAttributes[attribute][fixture] += newValue;
                    } else {
                        item->fixtureSpecificAngleAttributes[attribute][fixture] -= newValue;
                    }
                    while (item->fixtureSpecificAngleAttributes.value(attribute).value(fixture) >= 360.0) {
                        item->fixtureSpecificAngleAttributes[attribute][fixture] -= 360.0;
                    }
                    while (item->fixtureSpecificAngleAttributes.value(attribute).value(fixture) < 0.0) {
                        item->fixtureSpecificAngleAttributes[attribute][fixture] += 360.0;
                    }
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Fixture " + fixture->name() + + " to " + QString::number(getItem(ids.first())->fixtureSpecificAngleAttributes.value(attribute).value(fixture)) + "°.");
                } else {
                    kernel->terminal->success("Changed " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixture " + fixture->name() + ".");
                }
            } else {
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
                    item->fixtureSpecificAngleAttributes[attribute][fixture] = newValue;
                    emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " at Fixture " + fixture->name() + " to " + QString::number(newValue) + "°.");
                } else {
                    kernel->terminal->success("Set " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " at Fixture " + fixture->name() + " to " + QString::number(newValue) + "°.");
                }
            }
        }
    } else if (angleAttributes.contains(attribute)) { // Angle Attribute
        AngleAttribute angleAttribute = angleAttributes.value(attribute);
        bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
        if (difference) {
            value.removeFirst();
        }
        bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
        if (negativeValue) {
            value.removeFirst();
        }
        float newValue = kernel->terminal->keysToValue(value);
        bool ok = true;
        if (!text.isEmpty()) {
            newValue = text.toFloat(&ok);
        }
        if ((newValue < 0) || !ok) {
            kernel->terminal->error("Can't set " + singularItemName + " " + angleAttribute.name + " because no valid number was given.");
            return;
        }
        if (difference) {
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                if (!negativeValue) {
                    item->angleAttributes[attribute] += newValue;
                } else {
                    item->angleAttributes[attribute] -= newValue;
                }
                while (item->angleAttributes.value(attribute) >= 360.0) {
                    item->angleAttributes[attribute] -= 360.0;
                }
                while (item->angleAttributes.value(attribute) < 0.0) {
                    item->angleAttributes[attribute] += 360.0;
                }
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(getItem(ids.first())->angleAttributes.value(attribute)) + "°.");
            } else {
                kernel->terminal->success("Changed " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + ".");
            }
        } else {
            while (newValue >= 360.0) {
                newValue -= 360.0;
            }
            if (negativeValue && (newValue > 0)) {
                newValue = 360.0 - newValue;
            }
            for (QString id : ids) {
                T* item = getItem(id);
                if (item == nullptr) {
                    item = addItem(id);
                }
                item->angleAttributes[attribute] = newValue;
                emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Set " + angleAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(newValue) + "°.");
            } else {
                kernel->terminal->success("Set " + angleAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + "°.");
            }
        }
    } else if (boolAttributes.contains(attribute)) { // Bool Attribute
        BoolAttribute boolAttribute = boolAttributes.value(attribute);
        bool newValue;
        QString valueText = QString::number(kernel->terminal->keysToValue(value));
        if (!text.isEmpty()) {
            valueText = text;
        }
        if (valueText == "0") {
            newValue = false;
        } else if (valueText == "1") {
            newValue = true;
        } else {
            kernel->terminal->error("Can't set " + singularItemName + " " + boolAttribute.name + " because this Atribute only accepts 0 or 1 as values.");
            return;
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            item->boolAttributes[attribute] = newValue;
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        if (ids.size() == 1) {
            kernel->terminal->success("Set " + boolAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(newValue) + ".");
        } else {
            kernel->terminal->success("Set " + boolAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(newValue) + ".");
        }
    } else if (rawListAttributes.contains(attribute)) { // Raw List Attribute
        RawListAttribute rawListAttribute = rawListAttributes.value(attribute);
        bool addRaws = value.startsWith(Keys::Plus);
        if (addRaws) {
            value.removeFirst();
        }
        QList<Raw*> rawSelection = QList<Raw*>();
        if (!value.isEmpty() || !text.isEmpty()) {
            if (!value.isEmpty()) {
                if (value.first() != Keys::Raw) {
                    kernel->terminal->error("Can't set " + singularItemName + " " + rawListAttribute.name + " because this requires Raws.");
                    return;
                }
                value.removeFirst();
            }
            QStringList rawIds = kernel->terminal->keysToSelection(value, Keys::Raw);
            if (!text.isEmpty()) {
                rawIds = text.split("+");
            }
            if (rawIds.isEmpty()) {
                kernel->terminal->error("Can't set " + singularItemName + " " + rawListAttribute.name + " because of an invalid Raw selection.");
                return;
            }
            for (QString rawId : rawIds) {
                Raw* raw = kernel->raws->getItem(rawId);
                if (raw == nullptr) {
                    kernel->terminal->warning("Can't add Raw " + rawId + " to " + singularItemName + " " + rawListAttribute.name + " because it doesn't exist.");
                } else {
                    if (!rawSelection.contains(raw)) {
                        rawSelection.append(raw);
                    }
                }
            }
        }
        for (QString id : ids) {
            T* item = getItem(id);
            if (item == nullptr) {
                item = addItem(id);
            }
            if (addRaws) {
                for (Raw* raw : rawSelection) {
                    if (!item->rawListAttributes[attribute].contains(raw)) {
                        item->rawListAttributes[attribute].append(raw);
                    }
                }
            } else {
                item->rawListAttributes[attribute] = rawSelection;
            }
            emit dataChanged(index(getItemRow(item->id), 0), index(getItemRow(item->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        if ((kernel->cuelistView->currentGroup != nullptr) && !kernel->cuelistView->currentGroup->fixtures.contains(kernel->cuelistView->currentFixture)) {
            kernel->cuelistView->currentFixture = nullptr;
        }
        if (ids.size() == 1) {
            kernel->terminal->success("Set " + rawListAttribute.name + " of " + singularItemName + " " + getItem(ids.first())->name() + " to " + QString::number(getItem(ids.first())->rawListAttributes.value(attribute).length()) + " Raws.");
        } else {
            if (addRaws) {
                kernel->terminal->success("Added " + QString::number(rawSelection.length()) + " Raws to the " + rawListAttribute.name + " of " + QString::number(ids.length()) + " " + pluralItemName + ".");
            } else {
                kernel->terminal->success("Set Raws of " + QString::number(ids.length()) + " " + pluralItemName + " to " + QString::number(rawSelection.length()) + " Raws.");
            }
        }
    } else {
        kernel->terminal->error("Can't set " + singularItemName + " Attribute " + attributes.value(Keys::Attribute) + ".");
    }
}

template <class T> void ItemTable<T>::saveItemsToFile(QXmlStreamWriter* fileStream) {
    fileStream->writeStartElement(pluralItemName);
    for (T* item : items) {
        fileStream->writeStartElement(singularItemName);
        fileStream->writeAttribute("ID", item->id);
        item->writeAttributesToFile(fileStream);
        fileStream->writeEndElement();
    }
    fileStream->writeEndElement();
}

template <class T> void ItemTable<T>::reset() {
    for (int itemIndex = (items.length() - 1); itemIndex >= 0; itemIndex--) {
        T* item = items[itemIndex];
        items.removeAt(itemIndex);
        delete item;
    }
}

template <class T> int ItemTable<T>::rowCount(const QModelIndex&) const {
    return items.size();
}

template <class T> int ItemTable<T>::columnCount(const QModelIndex&) const {
    return 1;
}

template <class T> QVariant ItemTable<T>::data(const QModelIndex &index, const int role) const {
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        return items[row]->name();
    } else if (role == Qt::BackgroundRole) {
        if (kernel->inspector->ids.contains(items[row]->id)) {
            return QColor(48, 48, 48);
        }
    }
    return QVariant();
}

template <class T> int ItemTable<T>::findRow(QString id) {
    int position = 0;
    QStringList idParts = id.split(".");
    for (T* item : items) {
        QStringList indexIdParts = item->id.split(".");
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

template <class T> T* ItemTable<T>::addItem(QString id) {
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
    for (QString attribute : modelSpecificFloatAttributes.keys()) {
        item->modelSpecificFloatAttributes[attribute] = QMap<Model*, float>();
    }
    for (QString attribute : fixtureSpecificFloatAttributes.keys()) {
        item->fixtureSpecificFloatAttributes[attribute] = QMap<Fixture*, float>();
    }
    for (QString attribute : angleAttributes.keys()) {
        item->angleAttributes[attribute] = angleAttributes.value(attribute).value;
    }
    for (QString attribute : modelSpecificAngleAttributes.keys()) {
        item->modelSpecificAngleAttributes[attribute] = QMap<Model*, float>();
    }
    for (QString attribute : angleAttributes.keys()) {
        item->fixtureSpecificAngleAttributes[attribute] = QMap<Fixture*, float>();
    }
    for (QString attribute : boolAttributes.keys()) {
        item->boolAttributes[attribute] = boolAttributes.value(attribute).value;
    }
    for (QString attribute : rawListAttributes.keys()) {
        item->rawListAttributes[attribute] = rawListAttributes.value(attribute).value;
    }
    int row = findRow(id);
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    endInsertRows();
    kernel->terminal->success("Added " + singularItemName + " " + id + ".");
    return item;
}
