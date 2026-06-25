/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemlistattribute.h"
#include "terminal/terminal.h"

ItemListAttribute::ItemListAttribute(
    const ItemType item,
    const QString id,
    const QString attributeName,
    const ItemType attributeForeignItem,
    const QString attributeValueTable
    ) : Attribute(item, id, attributeName), valueTable(attributeValueTable), foreignItem(attributeForeignItem) {}

bool ItemListAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList ItemListAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));

    QStringList output;

    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    if (valueKeys != QList<Keys::Key>({ Keys::Minus })) {
        if (!valueKeys.startsWith(foreignItem.getKey())) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no " + foreignItem.getPlural() + " were given."));
            return output;
        }
        const QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given " + foreignItem.getSingular() + " IDs are invalid."));
            return output;
        }
        for (QString foreignItemId : foreignItemIds) {
            const int key = foreignItem.getItemKey(foreignItemId, &output);
            if (key >= 0) {
                foreignItemKeys.append(key);
                foreignItemIdStrings.append(foreignItemId);
            }
        }
        Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
        if (foreignItemKeys.isEmpty()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no valid " + foreignItem.getPlural() + " were given."));
            return output;
        }
    }

    QStringList successfulIds;
    for (QString id : ids) {
        const int key = item.getItemKey(id, &output);
        if (key >= 0) {
            bool allQueriesSuccessful = true;
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :key");
            deleteQuery.bindValue(":key", key);
            if (deleteQuery.exec()) {
                for (const int foreignItemKey : foreignItemKeys) {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, valueitem_key) VALUES (:item, :foreign_item)");
                    insertQuery.bindValue(":item", key);
                    insertQuery.bindValue(":foreign_item", foreignItemKey);
                    if (!insertQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + "."));
                    }
                }
            } else {
                allQueriesSuccessful = false;
                qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed deleting old " + name + " of " + item.getSingular() + " " + id + "."));
            }
            if (allQueriesSuccessful) {
                successfulIds.append(id);
            }
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " to " + foreignItem.format(foreignItemIdStrings) + "."));
    }

    return output;
}
