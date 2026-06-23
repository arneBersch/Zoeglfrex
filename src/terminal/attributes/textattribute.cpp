/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "textattribute.h"
#include "terminal/terminal.h"

TextAttribute::TextAttribute(
    const ItemType item,
    const QString id,
    const QString attributeName,
    const QString attribute,
    const QString attributeRegex,
    QWidget* attributeWidget
    ) : Attribute(item, id, attributeName), tableAttribute(attribute), regex(attributeRegex) {
    widget = attributeWidget;
}

bool TextAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemKey, attributes) && (attributes.size() == 1);
}

QStringList TextAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QStringList output;

    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + tableAttribute + " FROM " + item.getSelectTable() + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            output.append(Terminal::formatErrorMessage("Failed to load current " + name + " of " + item.getSingular() + " " + ids.first() + "."));
            return output;
        }
        while (query.next()) {
            textValue = query.value(0).toString();
        }
    }
    bool ok;
    textValue = QInputDialog::getText(widget, QString(), (item.getSingular()+ " " + name), QLineEdit::Normal, textValue, &ok);
    if (!ok) {
        output.append(Terminal::formatErrorMessage("Popup canceled."));
        return output;
    }
    if (!regex.isEmpty() && !textValue.contains(QRegularExpression(regex))) {
        output.append(Terminal::formatErrorMessage("Can't set " + item.getSingular() + " " + name + " because the given value \"" + textValue + "\" is not valid."));
        return output;
    }

    output.append(item.createItems(ids));

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
                updateQuery.bindValue(":value", textValue);
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

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " to \"" + textValue + "\"."));
    }

    return output;
}
