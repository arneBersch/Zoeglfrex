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

bool TextAttribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemType, attributes) && (attributes.size() == 1);
}

QStringList TextAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(matches(item, attributes));
    Q_ASSERT(valueKeys.isEmpty());

    QStringList output;
    QList<int> keys = item.getItemKeys(ids, &output);

    QString textValue = QString();
    if ((keys.length() == 1) && (keys.first() >= 0)) {
        QSqlQuery query;
        query.prepare("SELECT " + tableAttribute + " FROM " + item.getSelectTable() + " WHERE key = :key");
        query.bindValue(":key", keys.first());
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        if (query.next()) {
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

    QStringList successfulIds;
    for (int i = 0; i < keys.length(); i++) {
        if (keys[i] >= 0) {
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + tableAttribute + " = :value WHERE key = :key");
            updateQuery.bindValue(":key", keys[i]);
            updateQuery.bindValue(":value", textValue);
            if (updateQuery.exec()) {
                successfulIds.append(ids[i]);
            } else {
                qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                output.append(Terminal::formatErrorMessage("Failed setting " + name + " of " + item.getSingular() + " " + ids[i] + "."));
            }
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Set " + name + " of " + item.format(successfulIds) + " to \"" + textValue + "\"."));
    }

    return output;
}
