/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemlist.h"
#include "kernel/kernel.h"

template <class T> ItemList<T>::ItemList(Kernel* core, int key, QString singular, QString plural) {
    kernel = core;
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

template <class T> QStringList ItemList<T>::getIds() const {
    QStringList ids;
    for (T* item : items) {
        ids.append(item->id);
    }
    return ids;
}

template <class T> void ItemList<T>::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
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
    } else if (attribute == IDATTRIBUTEID) { // Move Item
        QString targetId = kernel->terminal->keysToId(value);
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
    } else {
        kernel->terminal->error("Can't set " + singularItemName + " Attribute " + attributes.value(Keys::Attribute) + ".");
    }
}

template <class T> void ItemList<T>::saveItemsToFile(QXmlStreamWriter* fileStream) {
    fileStream->writeStartElement(pluralItemName);
    for (int itemRow = (items.size() - 1); itemRow >= 0; itemRow--) {
        T* item = items[itemRow];
        fileStream->writeStartElement(singularItemName);
        fileStream->writeAttribute("ID", item->id);
        item->writeAttributesToFile(fileStream);
        fileStream->writeEndElement();
    }
    fileStream->writeEndElement();
}

template <class T> void ItemList<T>::reset() {
    for (int itemIndex = (items.length() - 1); itemIndex >= 0; itemIndex--) {
        T* item = items[itemIndex];
        items.removeAt(itemIndex);
        delete item;
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
    int row = findRow(id);
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    endInsertRows();
    kernel->terminal->success("Added " + singularItemName + " " + id + ".");
    return item;
}
