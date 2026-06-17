/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "integerspecificnumberattribute.h"

template <typename T> IntegerSpecificNumberAttribute<T>::IntegerSpecificNumberAttribute(
    const ItemType item,
    const QString name,
    const QString attributeValueTable,
    const NumberType key,
    const NumberType value)
    : Attribute(item, name), valueTable(attributeValueTable), keyNumber(key), valueNumber(value) {
}

template <typename T> void IntegerSpecificNumberAttribute<T>::set(QStringList ids, QString integerId, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = integerId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid.");
        return;
    }
    key = Keys::keysToFloat({Keys::Plus, Keys::Zero}, &ok, key, keyNumber);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
    T value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToFloat(valueKeys, &ok, 0, valueNumber);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                if (removeValues) {
                    QSqlQuery query;
                    query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                    query.bindValue(":item", itemKey);
                    query.bindValue(":key", key);
                    if (query.exec()) {
                        successfulIds.append(id);
                    } else {
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        error("Failed removing the " + name + " of " + item.getSingular() + " " + id + ".");
                    }
                } else {
                    bool valueOk = true;
                    if (difference) {
                        QSqlQuery currentValueQuery;
                        currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                        currentValueQuery.bindValue(":item", itemKey);
                        currentValueQuery.bindValue(":key", key);
                        if (currentValueQuery.exec()) {
                            if (currentValueQuery.next()) {
                                value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), valueNumber);
                            } else {
                                value = keysToFloat(valueKeys, &valueOk, 0, valueNumber);
                            }
                            if (!valueOk) {
                                error("Invalid value given for " + item.getSingular() + " " + id + ".");
                            }
                        } else {
                            qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                            error("Failed loading the current " + name + " of " + item.getSingular() + " " + id + ".");
                            valueOk = false;
                        }
                    }
                    if (valueOk) {
                        QSqlQuery query;
                        query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, key, value) VALUES (:item, :key, :value)");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":key", key);
                        query.bindValue(":value", value);
                        if (query.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + name + " of " + item.getSingular() + " " + id + ".");
                        }
                    }
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + name + " of " + item.format(successfulIds) + " at " + QString::number(key) + ".");
        } else if (difference) {
            success("Changed " + name + " of " + item.format(successfulIds) + " at " + QString::number(key) + keyNumber.getUnit() + " by " + QString::number(value) + valueNumber.getUnit() + ".");
        } else {
            success("Set " + name + " of " + item.format(successfulIds) + " at " + QString::number(key) + keyNumber.getUnit() + " to " + QString::number(value) + valueNumber.getUnit() + ".");
        }
    }
}
