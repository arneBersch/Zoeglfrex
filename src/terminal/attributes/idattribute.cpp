/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "idattribute.h"
#include "terminal/terminal.h"

IDAttribute::IDAttribute(const ItemType item) : Attribute(item, "0", "ID") {}

bool IDAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList IDAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));
    QStringList output;

    valueKeys.prepend(item.getKey());
    QStringList newIds = keysToIds(valueKeys);
    if (newIds.size() != 1) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " ID because no valid ID was given."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        existsQuery.bindValue(":id", newIds.first());
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                output.append(Terminal::formatWarningMessage("Can't set ID of " + item.getSingular() + " to " + newIds.first() + " because this " + item.getSingular() + " ID is already used."));
            } else {
                QSqlQuery keyQuery;
                keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
                keyQuery.bindValue(":id", id);
                if (keyQuery.exec()) {
                    if (keyQuery.next()) {
                        QSqlQuery updateQuery;
                        updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET id = :newId WHERE key = :key");
                        updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                        updateQuery.bindValue(":newId", newIds.first());
                        if (updateQuery.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                            output.append(Terminal::formatErrorMessage("Failed to update ID of " + item.getSingular() + " " + id + " because the request failed."));
                        }
                    } else {
                        output.append(Terminal::formatWarningMessage("Can't set ID of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist."));
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed loading " + item.getSingular() + " " + id + "."));
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Error executing check if " + item.getSingular() + " " + newIds.first() + " exists."));
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Set ID of " + item.format(successfulIds) + " to " + newIds.first() + "."));
    }

    output.append(item.updateSortingKeys());
    return output;
}
