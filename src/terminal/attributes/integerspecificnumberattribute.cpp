/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "integerspecificnumberattribute.h"
#include "terminal/terminal.h"

IntegerSpecificNumberAttribute::IntegerSpecificNumberAttribute(
    const ItemType item,
    const QString id,
    const QString name,
    const QString attributeValueTable,
    const NumberType key,
    const NumberType value
    ) : Attribute(item, id + ".*", name), valueTable(attributeValueTable), keyNumber(key), valueNumber(value) {
}

bool IntegerSpecificNumberAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList IntegerSpecificNumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
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

    QStringList successfulIds;
    for (QString id : ids) {
        const int itemKey = item.getItemKey(id, &output);
        if (itemKey >= 0) {
            if (removeValues) {
                QSqlQuery query;
                query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                query.bindValue(":item", itemKey);
                query.bindValue(":key", key);
                if (query.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + "."));
                }
            } else {
                bool valueOk = true;
                if (difference) {
                    QSqlQuery currentValueQuery;
                    currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                    currentValueQuery.bindValue(":item", itemKey);
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
                    query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, key, value) VALUES (:item, :key, :value)");
                    query.bindValue(":item", itemKey);
                    query.bindValue(":key", key);
                    query.bindValue(":value", value);
                    if (query.exec()) {
                        successfulIds.append(id);
                    } else {
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + "."));
                    }
                }
            }
        }
    }

    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + key.toString() + "."));
        } else if (difference) {
            output.append(Terminal::formatSuccessMessage("Changed " + name + " of " + item.format(successfulIds) + " at " + key.toString() + keyNumber.getUnit() + " by " + value.toString() + valueNumber.getUnit() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + key.toString() + keyNumber.getUnit() + " to " + value.toString() + valueNumber.getUnit() + "."));
        }
    }

    return output;
}
