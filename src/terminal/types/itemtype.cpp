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
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + updateTable + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Failed to insert " + singular + " " + id + "."));
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

    if (*this == ItemType::cue()) {
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
    msgBox.setInformativeText("Do you want to delete " + format(ids) + "?");
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
