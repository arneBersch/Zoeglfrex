/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "integerspecificitemlistattribute.h"
#include "terminal/terminal.h"

IntegerSpecificItemListAttribute::IntegerSpecificItemListAttribute(
    const ItemType item,
    const QString id,
    const QString name,
    const QString attributeValueTable,
    const ItemType value,
    const NumberType key,
    const bool multiple)
    : Attribute(item, id, name), valueTable(attributeValueTable), valueItem(value), keyNumber(key), allowMultiple(multiple) {}

bool IntegerSpecificItemListAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList IntegerSpecificItemListAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
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
    key = Keys::keysToNumber({Keys::Plus, Keys::Zero}, &ok, key.toInt(), keyNumber);
    if (!ok) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given Attribute is not valid."));
        return output;
    }
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
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
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.getSelectTable() + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    output.append(Terminal::formatWarningMessage("Can't add " + valueItem.getSingular() + " " + valueItemId + " to " + item.getSingular() + " " + name + " because this " + valueItem.getSingular() + " doesn't exist."));
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed to execute check if " + valueItem.getSingular() + " " + valueItemId + " exists."));
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no valid " + valueItem.getPlural() + " were given."));
            return output;
        }
        if (valueItemKeys.isEmpty()) {
            output.append(Terminal::formatErrorMessage("No valid " + valueItem.getPlural() + " were found."));
            return output;
        }
    }

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
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND key = :key");
                deleteQuery.bindValue(":item", itemKey);
                deleteQuery.bindValue(":key", key);
                if (!deleteQuery.exec()) {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed deleting old " + name + " of " + item.getSingular() + " " + id + "."));
                }
                for (const int valueItemKey : valueItemKeys) {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, key, valueitem_key) VALUES (:item, :key, :value_item)");
                    insertQuery.bindValue(":item", itemKey);
                    insertQuery.bindValue(":key", key);
                    insertQuery.bindValue(":value_item", valueItemKey);
                    if (!insertQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed to insert a " + valueItem.getSingular() + " into " + item.getSingular() + " " + id + "."));
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                output.append(Terminal::formatErrorMessage("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found."));
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed loading " + item.getSingular() + " " + id + "."));
        }
    }

    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + " at " + key.toString() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " at " + key.toString() + " to " + valueItem.format(valueItemIdStrings) + "."));
        }
    }

    return output;
}
