/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEMLISTATTRIBUTE_H
#define ITEMLISTATTRIBUTE_H

#include "attribute.h"

class ItemListAttribute : public Attribute {
public:
    ItemListAttribute(ItemType item, QString name, ItemType foreignItem, QString valueTable);
    void set(QStringList ids, QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) override;
private:
    const QString valueTable;
    const ItemType foreignItem;
};

#endif // ITEMLISTATTRIBUTE_H
