/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemspecificitemlistattribute.h"

ItemSpecificItemListAttribute::ItemSpecificItemListAttribute(
    const ItemType item,
    const QString attributeName,
    const ItemType attributeForeignItem,
    const ItemType attributeValueItem,
    const QString attributeValueTable,
    const bool allowMultiple
) : Attribute(item, attributeName),
    valueTable(attributeValueTable),
    foreignItem(attributeForeignItem),
    valueItem(attributeValueItem),
    allowMultiple(allowMultiple) {
}

void ItemSpecificItemListAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(attributes.contains(foreignItem.getKey()) && !attributes.value(foreignItem.getKey()).isEmpty());
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + name + " because no " + valueItem.getPlural() + " were given.");
            return;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + name + " because the given " + valueItem.getSingular() + " IDs are invalid.");
            return;
        }
        if (!allowMultiple && (valueItemIds.size() != 1)) {
            error("Can't set " + item.getSingular() + " " + name + " because this Attribute only accepts one " + valueItem.getSingular() + " as a value.");
            return;
        }
        for (QString valueItemId : valueItemIds) {
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.getSelectTable() + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    warning("Can't add " + valueItem.getSingular() + " " + valueItemId + " to " + item.getSingular() + " " + name + " because this " + valueItem.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                error("Failed to execute check if " + valueItem.getSingular() + " " + valueItemId + " exists.");
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + name + " because no valid " + valueItem.getPlural() + " were given.");
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
    Q_ASSERT(foreignItemKeys.size() == foreignItemIdStrings.size());
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
                    QSqlQuery deleteQuery;
                    deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND foreignItem_key = :foreign_item");
                    deleteQuery.bindValue(":item", itemKey);
                    deleteQuery.bindValue(":foreign_item", foreignItemKey);
                    if (!deleteQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                        error("Failed deleting old " + name + " of " + item.getSingular() + " " + id + ".");
                    }
                    for (const int valueItemKey : valueItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, foreignItem_key, valueItem_key) VALUES (:item, :foreign_item, :value_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        insertQuery.bindValue(":value_item", valueItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + ".");
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
        } else {
            success("Set " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " to " + valueItem.format(valueItemIdStrings) + ".");
        }
    }
}
