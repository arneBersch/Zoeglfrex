/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "boolattribute.h"

BoolAttribute::BoolAttribute(const ItemType item, const QString attribute, const QString name) : Attribute(item, name), tableAttribute(attribute) {}

void BoolAttribute::set(QStringList ids, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    int value = 0;
    QString valueText = "False";
    if ((valueKeys.size() == 1) && valueKeys.startsWith(Keys::Zero)) {
    } else if ((valueKeys.size() == 1) && valueKeys.startsWith(Keys::One)) {
        value = 1;
        valueText = "True";
    } else {
        error("Can't set " + item.getSingular() + " " + name + " because no valid value was given.");
        return;
    }

    createItems(item, ids);

    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + tableAttribute + " = :value WHERE key = :key");
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                updateQuery.bindValue(":value", value);
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + name + " of " + item.getSingular() + " " + id + ".");
                }
            } else {
                warning("Failed to set " + name + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Set " + name + " of " + item.format(successfulIds) + " to " + valueText + ".");
    }
}
