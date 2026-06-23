/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemattribute.h"
#include "terminal/terminal.h"

ItemAttribute::ItemAttribute(const ItemType item, const QString id, const QString attributeName, const QString attribute, const ItemType attributeForeignItem) : Attribute(item, id, attributeName), tableAttribute(attribute), foreignItem(attributeForeignItem) {}

bool ItemAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemKey, attributes) && (attributes.size() == 1);
}

QStringList ItemAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QStringList output;

    const bool removeItem = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    QString foreignItemId;
    int foreignItemKey;
    QStringList successfulIds;
    if (!removeItem) {
        if (!valueKeys.startsWith(foreignItem.getKey())) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because no " + foreignItem.getSingular() + " was given."));
            return output;
        }
        QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.size() != 1) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given " + foreignItem.getSingular() + " ID is invalid."));
            return output;
        }
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemIds.first());
        if (!foreignItemQuery.exec()) {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed to execute check if " + foreignItem.getSingular() + " exists."));
            return output;
        }
        if (!foreignItemQuery.next()) {
            output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because " + foreignItem.getSingular() + " " + foreignItemIds.first() + " doesn't exist."));
            return output;
        }
        foreignItemKey = foreignItemQuery.value(0).toInt();
    }

    createItems(item, ids);

    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                if (removeItem) {
                    updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + name + " = NULL WHERE key = :key");
                } else {
                    updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + name + " = :item WHERE key = :key");
                    updateQuery.bindValue(":item", foreignItemKey);
                }
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed setting " + name + " of " + item.getSingular() + " " + id + "."));
                }
            } else {
                output.append(Terminal::formatWarningMessage("Failed to set " + name + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found."));
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed loading " + item.getSingular() + " " + id + "."));
        }
    }

    if (!successfulIds.isEmpty()) {
        if (removeItem) {
            output.append(Terminal::formatSuccessMessage("Removed " + name + " of " + item.format(successfulIds) + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " to " + foreignItem.getSingular() + " " + foreignItemId + "."));
        }
    }
    return output;
}
