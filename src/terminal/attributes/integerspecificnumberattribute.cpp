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

QStringList IntegerSpecificNumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));

    QStringList output;

    QVariant key = processKeyAttribute(attributes.value(Keys::Attribute).first(), keyNumber);
    if (!key.isValid()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid."));
        return output;
    }

    const bool removeValues = valueKeys == QList<Keys::Key>({ Keys::Minus });
    const bool difference = valueKeys.startsWith(Keys::Plus);
    if (difference) {
        valueKeys.removeFirst();
    }
    const QList<float> values = Keys::keysToNumbers(valueKeys, ids.length());
    if (!removeValues && values.isEmpty()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because an invalid value was given."));
        return output;
    }

    QStringList successfulIds;
    for (int itemIndex = 0; itemIndex < ids.length(); itemIndex++) {
        const QString id = ids[itemIndex];
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
                float value = values[itemIndex];
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
                        value += currentValueQuery.value(0).toFloat();
                    }
                }

                if (valueOk) {
                    const QVariant formattedValue = valueNumber.format(value);
                    if (formattedValue.isValid()) {
                        QSqlQuery query;
                        query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, key, value) VALUES (:item, :key, :value)");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":key", key);
                        query.bindValue(":value", formattedValue);
                        if (query.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed removing the " + name + " of " + item.getSingular() + " " + id + "."));
                        }
                    } else {
                        output.append(Terminal::formatWarningMessage("Invalid value given for " + item.getSingular() + " " + id + "."));
                    }
                }
            }
        }
    }

    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + key.toString() + "."));
        } else if (difference) {
            output.append(Terminal::formatSuccessMessage("Changed " + name + " of " + item.format(successfulIds) + " at " + key.toString() + keyNumber.getUnit() + " by " + Keys::keysToString(valueKeys) + valueNumber.getUnit() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + key.toString() + keyNumber.getUnit() + " to " + Keys::keysToString(valueKeys) + valueNumber.getUnit() + "."));
        }
    }

    return output;
}
