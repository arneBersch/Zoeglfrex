/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attribute.h"

Attribute::Attribute(const ItemType attributeItem, const QString id, const QString attributeName) : item(attributeItem), attributeId(id), name(attributeName) {}

void Attribute::success(const QString message) {}

void Attribute::warning(const QString message) {}

void Attribute::error(const QString message) {}

bool Attribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) {
    const bool attributeMatches = (attributeId.isEmpty() && !attributes.contains(Keys::Attribute)) || (attributes.contains(Keys::Attribute) && (attributes.value(Keys::Attribute).size() == 1) && (attributes.value(Keys::Attribute).first() == attributeId));
    return itemKey == item.getKey() && attributeMatches;
}

void Attribute::createItems(const ItemType item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + item.getUpdateTable() + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                    error("Failed to insert " + item.getSingular() + " " + id + ".");
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            error("Failed to check if " + item.getSingular() + " " + id + " already exists.");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Created " + item.format(successfulIds) + ".");
    }
    updateSortingKeys(item);
    if (item == ItemType::cue()) {
        for (QString id : successfulIds) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
            keyQuery.bindValue(":id", id);
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    const int key = keyQuery.value(0).toInt();
                    QSqlQuery previousCueQuery;
                    previousCueQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE sortkey = (SELECT MAX(sortkey) FROM " + item.getSelectTable() + " WHERE sortkey < (SELECT sortkey FROM " + item.getSelectTable() + " WHERE key = :key))");
                    previousCueQuery.bindValue(":key", key);
                    if (previousCueQuery.exec()) {
                        if (previousCueQuery.next()) {
                            const int previousCueKey = previousCueQuery.value(0).toInt();
                            QStringList tables;
                            tables.append("cue_group_intensities");
                            tables.append("cue_group_colors");
                            tables.append("cue_group_positions");
                            tables.append("cue_group_raws");
                            tables.append("cue_group_effects");
                            for (QString table : tables) {
                                QSqlQuery valueQuery;
                                valueQuery.prepare("SELECT foreignItem_key, valueItem_key FROM " + table + " WHERE item_key = :key");
                                valueQuery.bindValue(":key", previousCueKey);
                                if (valueQuery.exec()) {
                                    while (valueQuery.next()) {
                                        QSqlQuery updateQuery;
                                        updateQuery.prepare("INSERT INTO " + table + " (item_key, foreignItem_key, valueItem_key) VALUES (:key, :foreignItem, :valueItem)");
                                        updateQuery.bindValue(":key", key);
                                        updateQuery.bindValue(":foreignItem", valueQuery.value(0).toInt());
                                        updateQuery.bindValue(":valueItem", valueQuery.value(1).toInt());
                                        if (!updateQuery.exec()) {
                                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                                            error("Failed to copy data of the previous " + item.getSingular() + " to " + item.getSingular() + " " + id + ".");
                                        }
                                    }
                                } else {
                                    qWarning() << Q_FUNC_INFO << valueQuery.executedQuery() << valueQuery.lastError().text();
                                    error("Failed to copy the data of the previous " + item.getSingular() + " to " + item.getSingular() + " " + id + ".");
                                }
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << previousCueQuery.executedQuery() << previousCueQuery.lastError().text();
                        error("Failed to get the " + item.getSingular() + " before " + item.getSingular() + " " + id + ".");
                    }
                } else {
                    error(item.getSingular() + " " + id + " wasn't found.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                error("Failed to get " + item.getSingular() + " " + id + ".");
            }
        }
    }
}

void Attribute::updateSortingKeys(const ItemType item) {
    QSqlQuery idsQuery;
    idsQuery.prepare("SELECT key, id, sortkey FROM " + item.getSelectTable());
    if (idsQuery.exec()) {
        struct IdKey {
            int key;
            QString id;
            int sortkey;
        };
        QList<IdKey> idKeys;
        while (idsQuery.next()) {
            IdKey idKey;
            idKey.key = idsQuery.value(0).toInt();
            idKey.id = idsQuery.value(1).toString();
            idKey.sortkey = idsQuery.value(2).toInt();
            idKeys.append(idKey);
        }
        std::sort(idKeys.begin(), idKeys.end(), [] (IdKey a, IdKey b) { return compareIds(a.id, b.id); });
        for (int index = 1; index <= idKeys.length(); index++) {
            const IdKey idKey = idKeys.at(index - 1);
            if (idKey.sortkey != index) {
                QSqlQuery query;
                query.prepare("UPDATE " + item.getUpdateTable() + " SET sortkey = :sortkey WHERE key = :key");
                query.bindValue(":key", idKey.key);
                query.bindValue(":sortkey", index);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                    error("Failed to update the sorting key of " + item.getSingular() + " " + idKey.id + ".");
                }
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << idsQuery.executedQuery() << idsQuery.lastError().text();
        error("Failed to update the " + item.getSingular() + " sorting keys.");
    }
}

bool Attribute::compareIds(const QString a, const QString b) {
    QStringList aParts = a.split(".");
    QStringList bParts = b.split(".");
    for (int part = 0; part < std::min(aParts.length(), bParts.length()); part++) {
        const int aPart = aParts.at(part).toInt();
        const int bPart = bParts.at(part).toInt();
        if (aPart != bPart) {
            return aPart < bPart;
        }
    }
    return (aParts.length() < bParts.length());
}
