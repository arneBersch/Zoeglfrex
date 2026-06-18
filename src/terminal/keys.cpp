/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "keys.h"
#include <QtSql>
#include "itemtype.h"

QString Keys::keysToString(QList<Key> keys) {
    QHash<Key, QString> keyStrings;
    keyStrings[Zero] = "0";
    keyStrings[One] = "1";
    keyStrings[Two] = "2";
    keyStrings[Three] = "3";
    keyStrings[Four] = "4";
    keyStrings[Five] = "5";
    keyStrings[Six] = "6";
    keyStrings[Seven] = "7";
    keyStrings[Eight] = "8";
    keyStrings[Nine] = "9";
    keyStrings[Plus] = " + ";
    keyStrings[Minus] = " - ";
    keyStrings[Thru] = " Thru ";
    keyStrings[Period] = ".";
    keyStrings[Set] = " Set ";
    keyStrings[Attribute] = " Attribute ";
    keyStrings[Model] = " Model ";
    keyStrings[Fixture] = " Fixture ";
    keyStrings[Group] = " Group ";
    keyStrings[Intensity] = " Intensity ";
    keyStrings[Color] = " Color ";
    keyStrings[Position] = " Position ";
    keyStrings[Raw] = " Raw ";
    keyStrings[Effect] = " Effect ";
    keyStrings[Cuelist] = " Cuelist ";
    keyStrings[Cue] = " Cue ";

    QString result;
    for(const Key key: keys) {
        Q_ASSERT(keyStrings.contains(key));
        result.append(keyStrings.value(key));
    }
    return result.simplified();
}

QVariant Keys::keysToNumber(QList<Key> keys, bool* ok, const float currentValue, const NumberType number) {
    const bool difference = keys.startsWith(Plus);
    if (difference) {
        keys.removeFirst();
    }
    float value = keysToString(keys).replace(" ", "").toFloat(ok);
    if (!(*ok)) {
        return value;
    }
    if (difference) {
        value += currentValue;
    }
    return number.format(ok, value);
}

QStringList Keys::keysToIds(QList<Key> keys) {
    if (keys.isEmpty()) {
        return QStringList();
    }
    const Keys::Key itemType = keys.first();
    keys.removeFirst();

    QStringList ids;
    if (keys.isEmpty()) {
        QSqlQuery query;
        if (itemType == ItemType::model().getKey()) {
            query.prepare("SELECT models.id FROM models, currentfixtures WHERE currentfixtures.model_key = models.key ORDER BY models.sortkey");
        } else if (itemType == ItemType::fixture().getKey()) {
            query.prepare("SELECT fixtures.id FROM fixtures, currentfixtures WHERE currentfixtures.key = fixtures.key ORDER BY fixtures.sortkey");
        } else if (itemType == ItemType::group().getKey()) {
            query.prepare("SELECT groups.id FROM groups, currentitems WHERE groups.key = currentitems.group_key");
        } else if (itemType == ItemType::intensity().getKey()) {
            query.prepare("SELECT intensities.id FROM intensities, currentcue, currentitems, cue_group_intensities WHERE currentitems.group_key = cue_group_intensities.foreignItem_key AND cue_group_intensities.valueItem_key = intensities.key AND cue_group_intensities.item_key = currentcue.key");
        } else if (itemType == ItemType::color().getKey()) {
            query.prepare("SELECT colors.id FROM colors, currentcue, currentitems, cuelists, cue_group_colors WHERE currentitems.group_key = cue_group_colors.foreignItem_key AND cue_group_colors.valueItem_key = colors.key AND cue_group_colors.item_key = currentcue.key");
        } else if (itemType == ItemType::position().getKey()) {
            query.prepare("SELECT positions.id FROM positions, currentcue, currentitems, cuelists, cue_group_positions WHERE currentitems.group_key = cue_group_positions.foreignItem_key AND cue_group_positions.valueItem_key = positions.key AND cue_group_positions.item_key = currentcue.key");
        } else if (itemType == ItemType::raw().getKey()) {
            query.prepare("SELECT raws.id FROM raws, currentcue, currentitems, cuelists, cue_group_raws WHERE currentitems.group_key = cue_group_raws.foreignItem_key AND cue_group_raws.valueItem_key = raws.key AND cue_group_raws.item_key = currentcue.key ORDER BY raws.sortkey");
        } else if (itemType == ItemType::effect().getKey()) {
            query.prepare("SELECT effects.id FROM effects, currentcue, currentitems, cuelists, cue_group_effects WHERE currentitems.group_key = cue_group_effects.foreignItem_key AND cue_group_effects.valueItem_key = effects.key AND cue_group_effects.item_key = currentcue.key ORDER BY effects.sortkey");
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
        } else if (itemType != Keys::Attribute) {
            return QStringList();
        }
        QStringList allIds;
        if (!table.isEmpty()) {
            QSqlQuery query;
            if (query.exec("SELECT id FROM " + table + " ORDER BY sortkey")) {
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
                if (idStartsWithPeriod) { // select all IDs with .
                    if (!thruParts.isEmpty() || !idParts.isEmpty()) {
                        return QStringList();
                    }
                    for (QString existingId : allIds) {
                        if (!idAdding) {
                            ids.removeAll(existingId);
                        } else if (!ids.contains(existingId)) {
                            ids.append(existingId);
                        }
                    }
                    idStartsWithPeriod = false;
                } else {
                    if (!thruParts.isEmpty()) {
                        return QStringList();
                    }
                    const QString idBeginning = idParts.join(".");
                    for (QString id : allIds) {
                        if (id.startsWith(idBeginning + ".") || (id == idBeginning)) {
                            if (!idAdding) {
                                ids.removeAll(id);
                            } else if (!ids.contains(id)) {
                                ids.append(id);
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
                            if (!idAdding) {
                                ids.removeAll(currentId);
                            } else if (!ids.contains(currentId)) {
                                ids.append(currentId);
                            }
                        }
                        thruParts.clear();
                        idStartsWithPeriod = false;
                    } else if (thruParts.isEmpty()) {
                        if (!idAdding) {
                            ids.removeAll(id);
                        } else if (!ids.contains(id)) {
                            ids.append(id);
                        }
                    } else {
                        const QString thruId = thruParts.join(".");
                        if (!allIds.contains(id) || !allIds.contains(thruId)) {
                            return QStringList();
                        }
                        for (int index = allIds.indexOf(thruId); index <= allIds.indexOf(id); index++) {
                            const QString currentId = allIds.at(index);
                            if (!idAdding) {
                                ids.append(currentId);
                            } else if (!ids.contains(currentId)) {
                                ids.append(currentId);
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

    return ids;
}

bool Keys::isItemKey(Key key) {
    const QList<Key> itemKeys = {
        Model,
        Fixture,
        Group,
        Intensity,
        Color,
        Position,
        Raw,
        Effect,
        Cuelist,
        Cue
    };

    return itemKeys.contains(key);
}