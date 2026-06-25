/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QtWidgets>
#include <QtSql>

#include "../types/itemtype.h"

class Attribute {
public:
    Attribute(ItemType item, QString attributeId, QString attributeName);
    virtual bool matches(ItemType itemType, QHash<Keys::Key, QStringList> attributes) const;
    virtual QStringList set(QStringList ids, QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) = 0;
protected:
    const ItemType item;
    const QString name;
private:
    QRegularExpression idRegex;
};

#endif // ATTRIBUTE_H
