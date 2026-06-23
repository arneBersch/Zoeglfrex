/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEMATTRIBUTE_H
#define ITEMATTRIBUTE_H

#include "attribute.h"

class ItemAttribute : public Attribute {
public:
    ItemAttribute(ItemType item, QString id, QString attributeName, QString tableAttribute, ItemType foreignItem);
    bool matches(Keys::Key itemKey, QHash<Keys::Key, QStringList> attributes) const override;
    QStringList set(QStringList ids, QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) override;
private:
    const QString tableAttribute;
    const ItemType foreignItem;
};

#endif // ITEMATTRIBUTE_H
