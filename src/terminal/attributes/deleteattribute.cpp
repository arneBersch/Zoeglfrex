/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "deleteattribute.h"
#include "terminal/terminal.h"

DeleteAttribute::DeleteAttribute(const ItemType item) : Attribute(item, "", "Delete") {}

bool DeleteAttribute::matches(const Keys::Key itemKey, const QHash<Keys::Key, QStringList> attributes) const {
    return Attribute::matches(itemKey, attributes) && attributes.isEmpty();
}

QStringList DeleteAttribute::set(const QStringList ids, const QHash<Keys::Key, QStringList> attributes, const QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    QStringList output;

    const QList<Keys::Key> expectedValue = {Keys::Minus};
    if (valueKeys != expectedValue) {
        output.append(Terminal::formatErrorMessage("Can't delete " + item.getPlural() + " because an unexpected Value was given."));
        return output;
    }

    QMessageBox msgBox;
    msgBox.setText("Delete " + QString::number(ids.length()) + " " + item.getPlural() + "?");
    msgBox.setInformativeText("Do you want to delete " + item.getSingular() + " " + ids.join(", ") + "?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if (msgBox.exec() != QMessageBox::Yes) {
        output.append(Terminal::formatErrorMessage("Popup canceled."));
        return output;
    }

    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + item.getUpdateTable() + " WHERE key = :key");
                deleteQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (deleteQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    output.append(Terminal::formatErrorMessage("Can't delete " + item.getSingular() + " " + id + " because the request failed."));
                }
            } else {
                output.append(Terminal::formatWarningMessage("Can't delete " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist."));
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            output.append(Terminal::formatErrorMessage("Couldn't delete " + item.getSingular() + " " + id + ": "));
        }
    }

    if (!successfulIds.isEmpty()) {
        output.append(Terminal::formatSuccessMessage("Deleted " + item.format(successfulIds) + "."));
    }

    output.append(updateSortingKeys(item));
    return output;
}
