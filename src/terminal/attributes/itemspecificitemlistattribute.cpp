/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemspecificitemlistattribute.h"
#include "terminal/terminal.h"

ItemSpecificItemListAttribute::ItemSpecificItemListAttribute(
    const ItemType item,
    const QString id,
    const QString attributeName,
    const ItemType attributeForeignItem,
    const ItemType attributeValueItem,
    const QString attributeValueTable,
    const bool allowMultiple
) : Attribute(item, id, attributeName),
    valueTable(attributeValueTable),
    foreignItem(attributeForeignItem),
    valueItem(attributeValueItem),
    allowMultiple(allowMultiple) {
}

bool ItemSpecificItemListAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && attributes.contains(foreignItem.getKey()) && (attributes.size() == 2);
}

QStringList ItemSpecificItemListAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));

    QStringList output;

    const bool removeValues = valueKeys == QList<Keys::Key>({ Keys::Minus });
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.getKey())) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no " + valueItem.getPlural() + " were given."));
            return output;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given " + valueItem.getSingular() + " IDs are invalid."));
            return output;
        }
        if (!allowMultiple && (valueItemIds.size() != 1)) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because this Attribute only accepts one " + valueItem.getSingular() + " as a value."));
            return output;
        }
        for (QString valueItemId : valueItemIds) {
            const int valueItemKey = valueItem.getItemKey(valueItemId, &output);
            if (valueItemKey >= 0) {
                valueItemKeys.append(valueItemKey);
                valueItemIdStrings.append(valueItemId);
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no valid " + valueItem.getPlural() + " were given."));
            return output;
        }
    }

    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : attributes.value(foreignItem.getKey())) {
        const int foreignItemKey = foreignItem.getItemKey(foreignItemId, &output);
        if (foreignItemKey >= 0) {
            foreignItemKeys.append(foreignItemKey);
            foreignItemIdStrings.append(foreignItemId);
        }
    }
    Q_ASSERT(foreignItemKeys.size() == foreignItemIdStrings.size());
    if (foreignItemKeys.isEmpty()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no valid " + foreignItem.getPlural() + " were found."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        int key = item.getItemKey(id, &output);
        if (key >= 0) {
            bool allQueriesSuccessful = true;
            for (const int foreignItemKey : foreignItemKeys) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND foreignitem_key = :foreign_item");
                deleteQuery.bindValue(":item", key);
                deleteQuery.bindValue(":foreign_item", foreignItemKey);
                if (!deleteQuery.exec()) {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed deleting old " + name + " of " + item.getSingular() + " " + id + "."));
                } else {
                    for (const int valueItemKey : valueItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, foreignitem_key, valueitem_key) VALUES (:item, :foreign_item, :value_item)");
                        insertQuery.bindValue(":item", key);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        insertQuery.bindValue(":value_item", valueItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + "."));
                        }
                    }
                }
            }
            if (allQueriesSuccessful) {
                successfulIds.append(id);
            }
        }
    }

    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " to " + valueItem.format(valueItemIdStrings) + "."));
        }
    }

    return output;
}
