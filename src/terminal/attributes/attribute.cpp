/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attribute.h"

Attribute::Attribute(const ItemType attributeItem, QString id, const QString attributeName) : item(attributeItem), name(attributeName) {
    id.replace(".", "\\.");
    id.replace("*", ".*");
    idRegex = QRegularExpression(id);
}

bool Attribute::matches(const ItemType itemType, const QHash<Keys::Key, QStringList> attributes) const {
    const bool attributeMatches = attributes.contains(Keys::Attribute) && (attributes.value(Keys::Attribute).size() == 1) && (attributes.value(Keys::Attribute).contains(idRegex));
    return (itemType == item) && attributeMatches;
}

QVariant Attribute::processKeyAttribute(const QString attribute, const NumberType keyNumber, bool* ok) {
    const QList<QString> numberIdParts = attribute.split(".");
    if (numberIdParts.length() != (idRegex.pattern().count("\\.") + 1)) {
        return -1;
    }

    QVariant key = numberIdParts.last().toInt(ok);
    if (!ok) {
        return -1;
    }

    key = keyNumber.format(ok, key.toInt());
    if (!ok) {
        return -1;
    }

    return key;
}