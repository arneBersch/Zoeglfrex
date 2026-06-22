/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemlistattribute.h"

ItemListAttribute::ItemListAttribute(
    const ItemType item,
    const QString id,
    const QString attributeName,
    const ItemType attributeForeignItem,
    const QString attributeValueTable
    ) : Attribute(item, id, attributeName), valueTable(attributeValueTable), foreignItem(attributeForeignItem) {}

bool ItemListAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) {
    return Attribute::matches(itemKey, attributes) && (attributes.size() == 1);
}

void ItemListAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    if ((valueKeys.size() != 1) || !valueKeys.startsWith(Keys::Minus)) {
        if (!valueKeys.startsWith(foreignItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + name + " because no " + foreignItem.getPlural() + " were given.");
            return;
        }
        const QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + name + " because the given " + foreignItem.getSingular() + " IDs are invalid.");
            return;
        }
        for (QString foreignItemId : foreignItemIds) {
            QSqlQuery foreignItemQuery;
            foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                    foreignItemIdStrings.append(foreignItemId);
                } else {
                    warning("Can't add " + foreignItem.getSingular() + " " + foreignItemId + " to " + item.getSingular() + " " + name + " because this " + foreignItem.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
                error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
            }
        }
        Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
        if (foreignItemKeys.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + name + " because no valid " + foreignItem.getPlural() + " were given.");
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
                bool allQueriesSuccessful = true;
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :key");
                deleteQuery.bindValue(":key", itemKey);
                if (deleteQuery.exec()) {
                    for (const int foreignItemKey : foreignItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, valueItem_key) VALUES (:item, :foreign_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + ".");
                        }
                    }
                } else {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Failed deleting old " + name + " of " + item.getSingular() + " " + id + ".");
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
        success("Set " + name + " of " + item.format(successfulIds) + " to " + foreignItem.format(foreignItemIdStrings) + ".");
    }
}
