/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemtype.h"
#include "terminal/terminal.h"

ItemType::ItemType(QString table, QString singular, QString plural, Keys::Key key) : ItemType(table, table, singular, plural, key) {}

ItemType::ItemType(QString itemSelectTable, QString itemUpdateTable, QString itemSingular, QString itemPlural, Keys::Key itemKey) {
    selectTable = itemSelectTable;
    updateTable = itemUpdateTable;
    singular = itemSingular;
    plural = itemPlural;
    key = itemKey;
}

QString ItemType::getSelectTable() const {
    return selectTable;
}

QString ItemType::getUpdateTable() const {
    return updateTable;
}

QString ItemType::getSingular() const {
    return singular;
}

QString ItemType::getPlural() const {
    return plural;
}

Keys::Key ItemType::getKey() const {
    return key;
}

QString ItemType::format(const QStringList ids) const {
    if (ids.length() == 0) {
        return "0 " + plural;
    }
    if (ids.length() == 1) {
        return singular + " " + ids.first();
    }
    return plural + " " + ids.join(", ");
}

bool ItemType::operator==(ItemType item) const {
    return (item.selectTable == selectTable) && (item.updateTable == updateTable) && (item.singular == singular) && (item.plural == plural) && (item.key == key);
}

ItemType ItemType::model() {
    return ItemType("models", "models", "Model", "Models", Keys::Model);
}

ItemType ItemType::fixture() {
    return ItemType("fixtures", "fixtures", "Fixture", "Fixtures", Keys::Fixture);
}

ItemType ItemType::group() {
    return ItemType("groups", "groups", "Group", "Groups", Keys::Group);
}

ItemType ItemType::intensity() {
    return ItemType("intensities", "intensities", "Intensity", "Intensities", Keys::Intensity);
}

ItemType ItemType::color() {
    return ItemType("colors", "colors", "Color", "Colors", Keys::Color);
}

ItemType ItemType::position() {
    return ItemType("positions", "positions", "Position", "Positions", Keys::Position);
}

ItemType ItemType::raw() {
    return ItemType("raws", "raws", "Raw", "Raws", Keys::Raw);
}

ItemType ItemType::effect() {
    return ItemType("effects", "effects", "Effect", "Effects", Keys::Effect);
}

ItemType ItemType::cuelist() {
    return ItemType("cuelists", "cuelists", "Cuelist", "Cuelists", Keys::Cuelist);
}

ItemType ItemType::cue() {
    return ItemType("currentcuelist_cues", "cues", "Cue", "Cues", Keys::Cue);
}

QList<ItemType> ItemType::allTypes() {
    return {
        ItemType::model(),
        ItemType::fixture(),
        ItemType::group(),
        ItemType::intensity(),
        ItemType::color(),
        ItemType::position(),
        ItemType::raw(),
        ItemType::effect(),
        ItemType::cuelist(),
        ItemType::cue(),
    };
}

QStringList ItemType::createItems(const QStringList ids) const {
    QStringList output;

    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + selectTable + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                output.append(Terminal::formatWarningMessage("Can't create " + singular + " " + id + " because this " + singular + " already exists."));
            } else {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + updateTable + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed to create " + singular + " " + id + "."));
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed to check if " + singular + " " + id + " already exists."));
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Created " + format(successfulIds) + "."));
    }

    output.append(updateSortingKeys());

    if (*this == cue()) {
        QStringList tables;
        tables.append("cue_group_intensities");
        tables.append("cue_group_colors");
        tables.append("cue_group_positions");
        tables.append("cue_group_raws");
        tables.append("cue_group_effects");
        for (QString id : successfulIds) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key, sortkey FROM " + selectTable + " WHERE id = :id");
            keyQuery.bindValue(":id", id);
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    const int key = keyQuery.value(0).toInt();
                    const int sortkey = keyQuery.value(1).toInt();
                    QSqlQuery previousCueQuery;
                    previousCueQuery.prepare("SELECT key FROM " + selectTable + " WHERE sortkey = (SELECT MAX(sortkey) FROM " + selectTable + " WHERE sortkey < :sort)");
                    previousCueQuery.bindValue(":sort", sortkey);
                    if (previousCueQuery.exec()) {
                        if (previousCueQuery.next()) {
                            const int previousCueKey = previousCueQuery.value(0).toInt();
                            for (QString table : tables) {
                                QSqlQuery valueQuery;
                                valueQuery.prepare("SELECT foreignitem_key, valueitem_key FROM " + table + " WHERE item_key = :key");
                                valueQuery.bindValue(":key", previousCueKey);
                                if (valueQuery.exec()) {
                                    while (valueQuery.next()) {
                                        QSqlQuery updateQuery;
                                        updateQuery.prepare("INSERT INTO " + table + " (item_key, foreignitem_key, valueitem_key) VALUES (:key, :foreignItem, :valueItem)");
                                        updateQuery.bindValue(":key", key);
                                        updateQuery.bindValue(":foreignItem", valueQuery.value(0).toInt());
                                        updateQuery.bindValue(":valueItem", valueQuery.value(1).toInt());
                                        if (!updateQuery.exec()) {
                                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                                            output.append(Terminal::formatErrorMessage("Failed to copy data of the previous " + singular + " to " + singular + " " + id + "."));
                                        }
                                    }
                                } else {
                                    qWarning() << Q_FUNC_INFO << valueQuery.executedQuery() << valueQuery.lastError().text();
                                    output.append(Terminal::formatErrorMessage("Failed to copy the data of the previous " + singular + " to " + singular + " " + id + "."));
                                }
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << previousCueQuery.executedQuery() << previousCueQuery.lastError().text();
                        output.append(Terminal::formatErrorMessage("Failed to get the " + singular + " before " + singular + " " + id + "."));
                    }
                } else {
                    output.append(Terminal::formatErrorMessage(singular + " " + id + " wasn't found."));
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed to get " + singular + " " + id + "."));
            }
        }
    }

    return output;
}

QStringList ItemType::deleteItems(QStringList ids) const {
    QStringList output;

    QMessageBox msgBox;
    msgBox.setText("Delete " + QString::number(ids.length()) + " " + plural + "?");
    msgBox.setInformativeText("Do you really want to delete " + format(ids) + "?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if (msgBox.exec() != QMessageBox::Yes) {
        output.append(Terminal::formatErrorMessage("Popup canceled."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + updateTable + " WHERE key = :key");
                deleteQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (deleteQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Can't delete " + singular + " " + id + " because the request failed."));
                }
            } else {
                output.append(Terminal::formatWarningMessage("Can't delete " + singular + " " + id + " because this " + singular + " doesn't exist."));
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Couldn't delete " + singular + " " + id + "."));
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Deleted " + format(successfulIds) + "."));
    }

    output.append(updateSortingKeys());
    return output;
}

QStringList ItemType::updateSortingKeys() const {
    QStringList output;

    QSqlQuery idsQuery;
    idsQuery.prepare("SELECT key, id, sortkey FROM " + selectTable);
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
                query.prepare("UPDATE " + updateTable + " SET sortkey = :sortkey WHERE key = :key");
                query.bindValue(":key", idKey.key);
                query.bindValue(":sortkey", index);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed to update the sorting key of " + singular + " " + idKey.id + "."));
                }
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << idsQuery.executedQuery() << idsQuery.lastError().text();
        output.append(Terminal::formatErrorMessage("Failed to update the " + singular + " sorting keys."));
    }
    return output;
}

bool ItemType::compareIds(const QString a, const QString b) {
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

QStringList ItemType::selectItems(const QStringList ids) const {
    QStringList output;

    if (*this == fixture()) {
        output = setCurrentItem(ids, "currentgroup_fixtures", "UPDATE currentitems SET fixture_key = :key");
    } else if (*this == group()) {
        output = setCurrentItem(ids, selectTable, "UPDATE currentitems SET group_key = :key");
    } else if (*this == intensity()) {
        output = setCueItem(ids, "cue_group_intensities", false);
    } else if (*this == color()) {
        output = setCueItem(ids, "cue_group_colors", false);
    } else if (*this == position()) {
        output = setCueItem(ids, "cue_group_positions", false);
    } else if (*this == raw()) {
        output = setCueItem(ids, "cue_group_raws", true);
    } else if (*this == effect()) {
        output = setCueItem(ids, "cue_group_effects", true);
    } else if (*this == cuelist()) {
        output = setCurrentItem(ids, selectTable, "UPDATE currentitems SET cuelist_key = :key");
    } else if (*this == cue()) {
        QSqlQuery blindQuery;
        if (blindQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
            if (blindQuery.next()) {
                output = setCurrentItem(ids, selectTable, "UPDATE currentitems SET cue_key = :key");
            } else {
                output = setCurrentItem(ids, selectTable, "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)");
            }
        } else {
            qWarning() << Q_FUNC_INFO << blindQuery.executedQuery() << blindQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Can't select " + singular + " because the Blind Request failed."));
        }
    } else {
        output.append(Terminal::formatErrorMessage("Can't select the given " + plural + "."));
    }

    return output;
}

QStringList ItemType::deselectItems() const {
    QStringList output;

    if (*this == intensity()) {
        output = setCueItem(QStringList(), "cue_group_intensities", false);
    } else if (*this == color()) {
        output = setCueItem(QStringList(), "cue_group_colors", false);
    } else if (*this == position()) {
        output = setCueItem(QStringList(), "cue_group_positions", false);
    } else if (*this == raw()) {
        output = setCueItem(QStringList(), "cue_group_raws", true);
    } else if (*this == effect()) {
        output = setCueItem(QStringList(), "cue_group_effects", true);
    } else {
        output.append(Terminal::formatErrorMessage("Can't deselect the current " + plural + "."));
    }

    return output;
}

QStringList ItemType::setCurrentItem(const QStringList ids, const QString itemTable, const QString updateQueryText) const {
    QStringList output;

    if (ids.length() != 1) {
        output.append(Terminal::formatErrorMessage("Can't select " + singular + " because no valid ID was given."));
        return output;
    }
    const QString id = ids.first();

    QSqlQuery keyQuery;
    keyQuery.prepare("SELECT key FROM " + itemTable + " WHERE id = :id");
    keyQuery.bindValue(":id", id);
    if (!keyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
        output.append(Terminal::formatErrorMessage("Can't select " + singular + " because the key request for " + singular + " " + id + " failed."));
        return output;
    }
    if (!keyQuery.next()) {
        output.append(Terminal::formatErrorMessage("Can't select " + singular + " " + id + " because this " + singular + " was not found."));
        return output;
    }
    const int key = keyQuery.value(0).toInt();

    QSqlQuery updateQuery;
    updateQuery.prepare(updateQueryText);
    updateQuery.bindValue(":key", key);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
        output.append(Terminal::formatErrorMessage("Failed to select " + singular + "."));
    }

    return output;
}

QStringList ItemType::setCueItem(const QStringList ids, const QString valueTable, const bool multipleItemsAllowed) const {
    QStringList output;

    if (!multipleItemsAllowed && (ids.size() > 1)) {
        output.append(Terminal::formatErrorMessage("Can't select multiple " + plural + "."));
        return output;
    }

    QList<int> itemKeys;
    for (QString id : ids) {
        QSqlQuery itemKeyQuery;
        itemKeyQuery.prepare("SELECT key FROM " + selectTable + " WHERE id = :id");
        itemKeyQuery.bindValue(":id", id);
        if (itemKeyQuery.exec()) {
            if (itemKeyQuery.next()) {
                itemKeys.append(itemKeyQuery.value(0).toInt());
            } else {
                output.append(Terminal::formatWarningMessage("Can't select " + singular + " " + id + " because this " + singular + " doesn't exist."));
            }
        } else {
            qWarning() << Q_FUNC_INFO << itemKeyQuery.executedQuery() << itemKeyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because the key request for " + singular + " " + ids.first() + " failed."));
        }
    }
    if (itemKeys.isEmpty() && !ids.isEmpty()) {
        output.append(Terminal::formatErrorMessage("No valid " + plural + " were given."));
        return output;
    }

    QSqlQuery groupKeyQuery;
    if (!groupKeyQuery.exec("SELECT group_key FROM currentitems WHERE group_key IS NOT NULL")) {
        qWarning() << Q_FUNC_INFO << groupKeyQuery.executedQuery() << groupKeyQuery.lastError().text();
        output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because request for the current Group failed."));
        return output;
    }
    if (!groupKeyQuery.next()) {
        output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because no Group is currently selected."));
        return output;
    }
    const int groupKey = groupKeyQuery.value(0).toInt();

    QSqlQuery cueKeyQuery;
    if (!cueKeyQuery.exec("SELECT key, sortkey FROM currentcue")) {
        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
        output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because the request for the current Cue failed."));
        return output;
    }
    if (!cueKeyQuery.next()) {
        output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because no Cue is currently selected."));
        return output;
    }
    QList<int> cueKeys;
    cueKeys.append(cueKeyQuery.value(0).toInt());

    if (QSettings().value("terminal/tracking", false).toBool()) {
        QSqlQuery currentCueValueQuery;
        currentCueValueQuery.prepare("SELECT valueitem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
        currentCueValueQuery.bindValue(":cue", cueKeys.first());
        currentCueValueQuery.bindValue(":group", groupKey);
        if (currentCueValueQuery.exec()) {
            QSet<int> currentCueValueKeys;
            while (currentCueValueQuery.next()) {
                currentCueValueKeys.insert(currentCueValueQuery.value(0).toInt());
            }

            QSqlQuery cueTrackingKeyQuery;
            cueTrackingKeyQuery.prepare("SELECT key, block FROM currentcuelist_cues WHERE sortkey > :sortkey ORDER BY sortkey");
            cueTrackingKeyQuery.bindValue(":sortkey", cueKeyQuery.value(1).toInt());
            if (cueTrackingKeyQuery.exec()) {
                bool sameValue = true;
                while (cueTrackingKeyQuery.next() && (cueTrackingKeyQuery.value(1).toInt() == 0) && sameValue) {
                    const int cueKey = cueTrackingKeyQuery.value(0).toInt();
                    QSqlQuery cueValueQuery;
                    cueValueQuery.prepare("SELECT valueitem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
                    cueValueQuery.bindValue(":cue", cueKey);
                    cueValueQuery.bindValue(":group", groupKey);
                    if (cueValueQuery.exec()) {
                        QSet<int> cueValueKeys;
                        while (cueValueQuery.next()) {
                            cueValueKeys.insert(cueValueQuery.value(0).toInt());
                        }
                        sameValue = cueValueKeys == currentCueValueKeys;
                        if (sameValue) {
                            cueKeys.append(cueKey);
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                        output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because the Cue tracking request failed."));
                        return output;
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because the Cue tracking request failed."));
                return output;
            }
        } else {
            qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Can't set Cue " + plural + " because the request for the " + singular + " in the current Cue failed."));
            return output;
        }
    }

    for (const int cueKey : cueKeys) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
        deleteQuery.bindValue(":cue", cueKey);
        deleteQuery.bindValue(":group", groupKey);
        if (!deleteQuery.exec()) {
            qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed deleting Cue " + plural + "."));
        }

        for (const int key : itemKeys) {
            QSqlQuery query;
            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, valueitem_key) VALUES (:cue, :group, :item)");
            query.bindValue(":cue", cueKey);
            query.bindValue(":group", groupKey);
            query.bindValue(":item", key);
            if (!query.exec()) {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed inserting " + singular + "."));
            }
        }
    }
    return output;
}
