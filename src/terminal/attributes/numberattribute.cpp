/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "numberattribute.h"
#include "terminal/terminal.h"

NumberAttribute::NumberAttribute(const ItemType item, const QString id, const QString name, const QString attribute, const NumberType numberType)
    : Attribute(item, id, name), tableAttribute(attribute), number(numberType) {}

bool NumberAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemKey, attributes) && (attributes.size() == 1);
}

QStringList NumberAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QStringList output;

    const bool difference = valueKeys.startsWith(Keys::Plus);
    QVariant value;
    if (!difference) {
        bool ok;
        value = keysToNumber(valueKeys, &ok, 0, number);
        if (!ok) {
            output.append(Terminal::formatErrorMessage("Invalid value given."));
            return output;
        }
    }

    createItems(item, ids);

    QStringList successfulIds;
    for (QString id : ids) {
        bool valueOk = true;
        if (difference) {
            QSqlQuery currentValueQuery;
            currentValueQuery.prepare("SELECT " + tableAttribute + " FROM " + item.getSelectTable() + " WHERE id = :id");
            currentValueQuery.bindValue(":id", id);
            if (currentValueQuery.exec()) {
                if (currentValueQuery.next()) {
                    value = keysToNumber(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                    if (!valueOk) {
                        output.append(Terminal::formatErrorMessage("Invalid value given for " + item.getSingular() + " " + id + "."));
                    }
                } else {
                    output.append(Terminal::formatErrorMessage("Failed loading the current " + tableAttribute + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist."));
                    valueOk = false;
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed loading the current " + tableAttribute + " of " + item.getSingular() + " " + id + "."));
                valueOk = false;
            }
        }
        if (valueOk) {
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
    }

    if (!successfulIds.isEmpty()) {
        if (difference) {
            output.append(Terminal::formatSuccessMessage("Changed " + tableAttribute + " of " + item.format(successfulIds) + " by " + value.toString() + number.getUnit() + "."));
        } else {
            output.append(Terminal::formatSuccessMessage("Set " + tableAttribute + " of " + item.format(successfulIds) + " to " + value.toString() + number.getUnit() + "."));
        }
    }
    return output;
}
