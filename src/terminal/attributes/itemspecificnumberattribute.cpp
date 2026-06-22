/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemspecificnumberattribute.h"

ItemSpecificNumberAttribute::ItemSpecificNumberAttribute(
    const ItemType item,
    const QString id,
    const QString name,
    const ItemType attributeForeignItem,
    const QString attributeValueTable,
    const NumberType numberType
    ) : Attribute(item, id, name), valueTable(attributeValueTable), foreignItem(attributeForeignItem), number(numberType) {
}

bool ItemSpecificNumberAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) {
    return Attribute::matches(itemKey, attributes) && attributes.contains(foreignItem.getKey()) && (attributes.size() == 2);
}

void ItemSpecificNumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(attributes.contains(foreignItem.getKey()) && !attributes.value(foreignItem.getKey()).isEmpty());
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
    QVariant value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToNumber(valueKeys, &ok, 0, number);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : attributes.value(foreignItem.getKey())) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + name + " for " + foreignItem.getSingular() + " " + foreignItemId + " because this " + foreignItem.getSingular() + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
        }
    }
    Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.getPlural() + " were found.");
        return;
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
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    if (removeValues) {
                        QSqlQuery query;
                        query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + name + " of " + item.getSingular() + " " + id + ".");
                        }
                    } else {
                        bool valueOk = true;
                        if (difference) {
                            QSqlQuery currentValueQuery;
                            currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item");
                            currentValueQuery.bindValue(":item", itemKey);
                            currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                            if (currentValueQuery.exec()) {
                                if (currentValueQuery.next()) {
                                    value = keysToNumber(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                                } else {
                                    value = keysToNumber(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
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
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignItem_key, value) VALUES (:item, :foreign_item, :value)");
                            query.bindValue(":item", itemKey);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":value", value);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                error("Failed removing the " + name + " of " + item.getSingular() + " " + id + ".");
                            }
                        } else {
                            allQueriesSuccessful = false;
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
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
            success("Removed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + ".");
        } else if (difference) {
            success("Changed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " by " + value.toString() + number.getUnit() + ".");
        } else {
            success("Set " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " to " + value.toString() + number.getUnit() + ".");
        }
    }
}
