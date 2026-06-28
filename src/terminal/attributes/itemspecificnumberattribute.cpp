/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemspecificnumberattribute.h"
#include "terminal/terminal.h"

ItemSpecificNumberAttribute::ItemSpecificNumberAttribute(
    const ItemType item,
    const QString id,
    const QString name,
    const ItemType attributeForeignItem,
    const QString attributeValueTable,
    const NumberType numberType
    ) : Attribute(item, id, name), valueTable(attributeValueTable), foreignItem(attributeForeignItem), number(numberType) {
}

bool ItemSpecificNumberAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && attributes.contains(foreignItem.getKey()) && (attributes.size() == 2);
}

QStringList ItemSpecificNumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));

    QStringList output;

    QList<int> foreignItemKeys;
    QStringList foreignItemIds;
    for (QString foreignItemId : attributes.value(foreignItem.getKey())) {
        const int foreignItemKey = foreignItem.getItemKey(foreignItemId, &output);
        if (foreignItemKey >= 0) {
            foreignItemKeys.append(foreignItemKey);
            foreignItemIds.append(foreignItemId);
        }
    }
    Q_ASSERT(foreignItemKeys.length() == foreignItemIds.length());
    if (foreignItemKeys.isEmpty()) {
        output.append(Terminal::formatErrorMessage("No valid " + foreignItem.getPlural() + " were found."));
        return output;
    }

    const bool removeValues = valueKeys == QList<Keys::Key>({ Keys::Minus });
    const bool difference = valueKeys.startsWith(Keys::Plus);
    if (difference) {
        valueKeys.removeFirst();
    }
    const QList<float> values = Keys::keysToNumbers(valueKeys, foreignItemKeys.length());
    if (!removeValues && values.isEmpty()) {
        output.append(Terminal::formatErrorMessage("Invalid value given."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        const int key = item.getItemKey(id, &output);
        if (key >= 0) {
            bool allQueriesSuccessful = true;
            for (int foreignItemIndex = 0; foreignItemIndex < foreignItemKeys.length(); foreignItemIndex++) {
                const int foreignItemKey = foreignItemKeys[foreignItemIndex];
                const QString foreignItemId = foreignItemIds[foreignItemIndex];
                if (removeValues) {
                    QSqlQuery query;
                    query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item");
                    query.bindValue(":item", key);
                    query.bindValue(":foreign_item", foreignItemKey);
                    if (!query.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + " and " + foreignItem.getSingular() + " " + foreignItemId + "."));
                    }
                } else {
                    float value = values[foreignItemIndex];
                    bool valueOk = true;
                    if (difference) {
                        QSqlQuery currentValueQuery;
                        currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item");
                        currentValueQuery.bindValue(":item", key);
                        currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                        if (!currentValueQuery.exec()) {
                            qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed loading the current " + name + " of " + item.getSingular() + " " + id + " and " + foreignItem.getSingular() + " " + foreignItemId + "."));
                            valueOk = false;
                        } else if (!currentValueQuery.next()) {
                            output.append(Terminal::formatErrorMessage("Failed loading the current " + name + " of " + item.getSingular() + " " + id + " and " + foreignItem.getSingular() + " " + foreignItemId + "."));
                            valueOk = false;
                        } else {
                            value += currentValueQuery.value(0).toFloat();
                        }
                    }

                    if (valueOk) {
                        const QVariant formattedValue = number.format(value);
                        if (formattedValue.isValid()) {
                            QVariant value;
                            QSqlQuery query;
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, value) VALUES (:item, :foreign_item, :value)");
                            query.bindValue(":item", key);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":value", formattedValue);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + " and " + foreignItem.getSingular() + " " + foreignItemId + "."));
                            }
                        } else {
                            output.append(Terminal::formatErrorMessage("Invalid value given for " + item.getSingular() + " " + id + " and " + foreignItem.getSingular() + " " + foreignItemId + "."));
                        }
                    } else {
                        allQueriesSuccessful = false;
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
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIds) + "."));
        } else if (difference) {
            output.append(Terminal::formatSuccessMessage("Changed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIds) + " by " + Keys::keysToString(valueKeys) + number.getUnit() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIds) + " to " + Keys::keysToString(valueKeys) + number.getUnit() + "."));
        }
    }

    return output;
}
