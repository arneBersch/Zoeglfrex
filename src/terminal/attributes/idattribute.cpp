/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "idattribute.h"
#include "terminal/terminal.h"
#include "constants.h"

IdAttribute::IdAttribute(const ItemType item) : Attribute(item, AttributeIds::id, "ID") {}

bool IdAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList IdAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));
    QStringList output;

    valueKeys.prepend(item.getKey());
    QStringList newIds = keysToIds(valueKeys);
    newIds.removeDuplicates();
    if (newIds.isEmpty()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getPlural() + " IDs because no valid new " + item.getPlural() + " IDs were given."));
        return output;
    } else if (newIds.length() != ids.length()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getPlural() + " IDs because " + QString::number(newIds.length()) + " instead of " + QString::number(ids.length()) + " new IDs were given."));
        return output;
    }

    for (int i = 0; i < ids.length(); i++) {
        const QString id = ids[i];
        const QString newId = newIds[i];
        const int key = item.getItemKey(id, &output);
        if (key >= 0) {
            const int newKey = item.getItemKey(newId);
            if (newKey < 0) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET id = :newId WHERE key = :key");
                updateQuery.bindValue(":key", key);
                updateQuery.bindValue(":newId", newId);
                if (updateQuery.exec()) {
                    output.append(Terminal::formatSuccessMessage("Set ID of " + item.getSingular() + " " + id + " to " + newId + "."));
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Couldn't update the ID of " + item.getSingular() + " " + id + " because the request failed."));
                }
            } else {
                output.append(Terminal::formatWarningMessage("Can't set ID of " + item.getSingular() + " " + id + " to " + newId + " because this ID is already used."));
            }
        }
    }

    output.append(item.updateSortingKeys());

    return output;
}
