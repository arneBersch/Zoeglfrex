/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attribute.h"

Attribute::Attribute(const ItemType attributeItem, const QString attributeName) : item(attributeItem), name(attributeName) {}

void Attribute::success(const QString message) {}

void Attribute::warning(const QString message) {}

void Attribute::error(const QString message) {}

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

QStringList Attribute::keysToIds(QList<Keys::Key> keys) {
    if (keys.isEmpty()) {
        return QStringList();
    }
    const Keys::Key itemType = keys.first();
    keys.removeFirst();
    QStringList ids;
    if (keys.isEmpty()) {
        QSqlQuery query;
        if (itemType == ItemType::model().getKey()) {
            query.prepare("SELECT models.id FROM models, currentfixtures WHERE currentfixtures.model_key = models.key");
        } else if (itemType == ItemType::fixture().getKey()) {
            query.prepare("SELECT id FROM currentfixtures");
        } else if (itemType == ItemType::group().getKey()) {
            query.prepare("SELECT groups.id FROM groups, currentitems WHERE groups.key = currentitems.group_key");
        } else if (itemType == ItemType::intensity().getKey()) {
            query.prepare("SELECT intensities.id FROM intensities, currentcue, currentitems, cue_group_intensities WHERE currentitems.group_key = cue_group_intensities.foreignItem_key AND cue_group_intensities.valueItem_key = intensities.key AND cue_group_intensities.item_key = currentcue.key");
        } else if (itemType == ItemType::color().getKey()) {
            query.prepare("SELECT colors.id FROM colors, currentcue, currentitems, cuelists, cue_group_colors WHERE currentitems.group_key = cue_group_colors.foreignItem_key AND cue_group_colors.valueItem_key = colors.key AND cue_group_colors.item_key = currentcue.key");
        } else if (itemType == ItemType::position().getKey()) {
            query.prepare("SELECT positions.id FROM positions, currentcue, currentitems, cuelists, cue_group_positions WHERE currentitems.group_key = cue_group_positions.foreignItem_key AND cue_group_positions.valueItem_key = positions.key AND cue_group_positions.item_key = currentcue.key");
        } else if (itemType == ItemType::raw().getKey()) {
            query.prepare("SELECT raws.id FROM raws, currentcue, currentitems, cuelists, cue_group_raws WHERE currentitems.group_key = cue_group_raws.foreignItem_key AND cue_group_raws.valueItem_key = raws.key AND cue_group_raws.item_key = currentcue.key");
        } else if (itemType == ItemType::effect().getKey()) {
            query.prepare("SELECT effects.id FROM effects, currentcue, currentitems, cuelists, cue_group_effects WHERE currentitems.group_key = cue_group_effects.foreignItem_key AND cue_group_effects.valueItem_key = effects.key AND cue_group_effects.item_key = currentcue.key");
        } else if (itemType == ItemType::cuelist().getKey()) {
            query.prepare("SELECT cuelists.id FROM cuelists, currentitems WHERE cuelists.key = currentitems.cuelist_key");
        } else if (itemType == ItemType::cue().getKey()) {
            query.prepare("SELECT id FROM currentcue");
        } else {
            return QStringList();
        }
        if (query.exec()) {
            while (query.next()) {
                ids.append(query.value(0).toString());
            }
        } else {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
    } else {
        QStringList allIds;
        QString table;
        if (itemType == ItemType::model().getKey()) {
            table = ItemType::model().getSelectTable();
        } else if (itemType == ItemType::fixture().getKey()) {
            table = ItemType::fixture().getSelectTable();
        } else if (itemType == ItemType::group().getKey()) {
            table = ItemType::group().getSelectTable();
        } else if (itemType == ItemType::intensity().getKey()) {
            table = ItemType::intensity().getSelectTable();
        } else if (itemType == ItemType::color().getKey()) {
            table = ItemType::color().getSelectTable();
        } else if (itemType == ItemType::position().getKey()) {
            table = ItemType::position().getSelectTable();
        } else if (itemType == ItemType::raw().getKey()) {
            table = ItemType::raw().getSelectTable();
        } else if (itemType == ItemType::effect().getKey()) {
            table = ItemType::effect().getSelectTable();
        } else if (itemType == ItemType::cuelist().getKey()) {
            table = ItemType::cuelist().getSelectTable();
        } else if (itemType == ItemType::cue().getKey()) {
            table = ItemType::cue().getSelectTable();
        } else if (itemType == Keys::Attribute) {
        } else {
            return QStringList();
        }
        if (!table.isEmpty()) {
            QSqlQuery query;
            if (query.exec("SELECT id FROM " + table)) {
                while (query.next()) {
                    allIds.append(query.value(0).toString());
                }
            } else {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            }
        }

        keys.append(Keys::Plus);
        QStringList thruParts;
        QStringList idParts;
        QList<Keys::Key> currentIdPartKeys;
        bool idAdding = true;
        bool idStartsWithPeriod = false;
        for (const Keys::Key key : keys) {
            if (((key == Keys::Plus) || (key == Keys::Minus)) && currentIdPartKeys.isEmpty()) { // IDs that end with a period
                if (idStartsWithPeriod) {
                    if (!thruParts.isEmpty() || !idParts.isEmpty()) {
                        return QStringList();
                    }
                    for (QString existingId : allIds) {
                        if (idAdding) {
                            ids.append(existingId);
                        } else {
                            ids.removeAll(existingId);
                        }
                    }
                    idStartsWithPeriod = false;
                } else {
                    if (!thruParts.isEmpty()) {
                        return QStringList();
                    }
                    for (QString id : allIds) {
                        if (id.startsWith(idParts.join(".") + ".") || (id == idParts.join("."))) {
                            if (idAdding) {
                                ids.append(id);
                            } else {
                                ids.removeAll(id);
                            }
                        }
                    }
                    idParts.clear();
                }
            } else if ((key == Keys::Period) || (key == Keys::Plus) || (key == Keys::Minus) || (key == Keys::Thru)) {
                if ((key == Keys::Period) && idParts.isEmpty() && currentIdPartKeys.isEmpty() && !idStartsWithPeriod) {
                    idStartsWithPeriod = true;
                } else {
                    bool ok;
                    const int idPart = keysToString(currentIdPartKeys).toInt(&ok);
                    if (!ok || (idPart < 0)) {
                        return QStringList();
                    }
                    idParts.append(QString::number(idPart));
                    currentIdPartKeys.clear();
                }
                if ((key == Keys::Plus) || (key == Keys::Minus)) {
                    if (idParts.isEmpty() && !idStartsWithPeriod) {
                        return QStringList();
                    }
                    QString id = idParts.join(".");
                    while (id.endsWith(".0")) {
                        id.chop(2);
                    }
                    if (idStartsWithPeriod) {
                        if (thruParts.isEmpty() || (idParts.size() != 1)) {
                            return QStringList();
                        }
                        for (int ending = thruParts.last().toInt(); ending <= idParts.last().toInt(); ending++) {
                            QStringList currentIdParts = thruParts.first(thruParts.length() - 1);
                            currentIdParts.append(QString::number(ending));
                            QString currentId = currentIdParts.join(".");
                            while (currentId.endsWith(".0")) {
                                currentId.chop(2);
                            }
                            if (idAdding) {
                                ids.append(currentId);
                            } else {
                                ids.removeAll(currentId);
                            }
                        }
                        thruParts.clear();
                        idStartsWithPeriod = false;
                    } else if (thruParts.isEmpty()) {
                        if (idAdding) {
                            ids.append(id);
                        } else {
                            ids.removeAll(id);
                        }
                    } else {
                        const QString thruId = thruParts.join(".");
                        QStringList allIdsExtended = allIds;
                        if (!allIdsExtended.contains(thruId)) {
                            allIdsExtended.append(thruId);
                        }
                        if (!allIdsExtended.contains(id)) {
                            allIdsExtended.append(id);
                        }
                        std::sort(allIdsExtended.begin(), allIdsExtended.end(), Attribute::compareIds);
                        for (int index = allIdsExtended.indexOf(thruId); index <= allIdsExtended.indexOf(id); index++) {
                            QString currentId = allIdsExtended.at(index);
                            if (allIds.contains(currentId)) {
                                if (idAdding) {
                                    ids.append(currentId);
                                } else {
                                    ids.removeAll(currentId);
                                }
                            }
                        }
                    }
                    thruParts.clear();
                    idParts.clear();
                }
            } else {
                currentIdPartKeys.append(key);
            }
            if (key == Keys::Thru) {
                if (!thruParts.isEmpty() || idParts.isEmpty() || idStartsWithPeriod) {
                    return QStringList();
                }
                thruParts = idParts;
                idParts.clear();
            } else if (key == Keys::Plus) {
                idAdding = true;
            } else if (key == Keys::Minus) {
                idAdding = false;
            }
        }
    }
    ids.removeDuplicates();
    std::sort(ids.begin(), ids.end(), Attribute::compareIds);
    return ids;
}
