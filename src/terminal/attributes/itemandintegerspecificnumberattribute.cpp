/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemandintegerspecificnumberattribute.h"
#include "terminal/terminal.h"

ItemAndIntegerSpecificNumberAttribute::ItemAndIntegerSpecificNumberAttribute(
    const ItemType item,
    const QString id,
    const QString name,
    const ItemType attributeForeignItem,
    const QString attributeValueTable,
    const NumberType key,
    const NumberType value)
    : Attribute(item, id, name), valueTable(attributeValueTable), foreignItem(attributeForeignItem), keyNumber(key), valueNumber(value) {
}

bool ItemAndIntegerSpecificNumberAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && attributes.contains(foreignItem.getKey()) && (attributes.size() == 2);
}

QStringList ItemAndIntegerSpecificNumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));

    QStringList output;

    const QList<QString> numberIdParts = attributes.value(Keys::Attribute).first().split(".");
    if (numberIdParts.length() != 2) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid."));
        return output;
    }
    bool ok;
    QVariant key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid."));
        return output;
    }
    key = keyNumber.format(&ok, key.toInt());
    if (!ok) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid."));
        return output;
    }

    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
    QVariant value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToNumber(valueKeys, &ok, 0, valueNumber);
        if (!ok) {
            output.append(Terminal::formatErrorMessage("Invalid value given."));
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
    Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
    if (foreignItemKeys.isEmpty()) {
        output.append(Terminal::formatErrorMessage("No valid " + foreignItem.getPlural() + " were found."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        const int itemKey = item.getItemKey(id, &output);
        if (itemKey >= 0) {
            bool allQueriesSuccessful = true;
            for (const int foreignItemKey : foreignItemKeys) {
                if (removeValues) {
                    QSqlQuery query;
                    query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item AND key = :key");
                    query.bindValue(":item", itemKey);
                    query.bindValue(":foreign_item", foreignItemKey);
                    query.bindValue(":key", key);
                    if (!query.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + "."));
                    }
                } else {
                    bool valueOk = true;
                    if (difference) {
                        QSqlQuery currentValueQuery;
                        currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item AND key = :key");
                        currentValueQuery.bindValue(":item", itemKey);
                        currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                        currentValueQuery.bindValue(":key", key);
                        if (!currentValueQuery.exec()) {
                            qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed loading the current " + name + " of " + item.getSingular() + " " + id + "."));
                            valueOk = false;
                        } else if (!currentValueQuery.next()) {
                            output.append(Terminal::formatErrorMessage("Failed loading the current " + name + " of " + item.getSingular() + " " + id + "."));
                            valueOk = false;
                        } else {
                            value = keysToNumber(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), valueNumber);
                            if (!valueOk) {
                                output.append(Terminal::formatErrorMessage("Invalid value given for " + item.getSingular() + " " + id + "."));
                            }
                        }
                    }
                    if (valueOk) {
                        QSqlQuery query;
                        query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, key, value) VALUES (:item, :foreign_item, :key, :value)");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        query.bindValue(":key", key);
                        query.bindValue(":value", value);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + "."));
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
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + key.toString() + keyNumber.getUnit() + "."));
        } else if (difference) {
            output.append(Terminal::formatSuccessMessage("Changed " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + key.toString() + keyNumber.getUnit() + " by " + value.toString() + valueNumber.getUnit() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + key.toString() + keyNumber.getUnit() + " to " + value.toString() + valueNumber.getUnit() + "."));
        }
    }

    return output;
}
