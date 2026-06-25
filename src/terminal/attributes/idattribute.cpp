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
    if (newIds.length() != ids.length()) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " ID because no valid IDs were given."));
        return output;
    }

    QList<int> itemKeys;
    QStringList existingIds;
    for (int i = ids.length() - 1; i >= 0; i--) {
        const int key = item.getItemKey(ids[i], &output);
        if (key >= 0) {
            itemKeys.append(key);
            existingIds.append(ids[i]);
        } else {
            newIds.removeAt(i);
        }
    }
    Q_ASSERT(itemKeys.length() == existingIds.length());
    Q_ASSERT(itemKeys.length() == newIds.length());

    for (int i = 0; i < itemKeys.length(); i++) {
        const int newKey = item.getItemKey(newIds[i]);
        if ((newKey >= 0) && !itemKeys.contains(newKey)) {
            output.append("Can't set " + item.getSingular() + " IDs because the ID " + newIds[i] + " would be used twice.");
            return output;
        }
    }

    for (int i = 0; i < itemKeys.length(); i++) {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET id = :newId WHERE key = :key");
        updateQuery.bindValue(":key", itemKeys[i]);
        updateQuery.bindValue(":newId", newIds[i]);
        if (updateQuery.exec()) {
            output.append(Terminal::formatSuccessMessage("Set ID of " + item.getSingular() + " " + ids[i] + " to " + newIds[i] + "."));
        } else {
            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Couldn't update the ID of " + item.getSingular() + " " + ids[i] + " because the request failed. Your file could be damaged!"));
        }
    }

    output.append(item.updateSortingKeys());

    return output;
}
