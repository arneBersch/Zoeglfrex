/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NUMBERATTRIBUTE_H
#define NUMBERATTRIBUTE_H

#include "attribute.h"

template <typename T> class NumberAttribute : public Attribute {
public:
    NumberAttribute(ItemType item, QString tableAttribute, QString name, NumberType number);
    void set(QStringList ids, QList<Keys::Key> valueKeys);
private:
    const QString tableAttribute;
    const NumberType number;
};

#endif // NUMBERATTRIBUTE_H
