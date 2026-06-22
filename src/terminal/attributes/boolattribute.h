/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BOOLATTRIBUTE_H
#define BOOLATTRIBUTE_H

#include "attribute.h"

class BoolAttribute : public Attribute {
public:
    BoolAttribute(ItemType item, QString id, QString name, QString tableAttribute);
    bool matches(Keys::Key itemKey, QHash<Keys::Key, QStringList> attributes) override;
    void set(QStringList ids, QHash<Keys::Key, QStringList> attributes, QList<Keys::Key> valueKeys) override;
private:
    const QString tableAttribute;
};

#endif // BOOLATTRIBUTE_H
