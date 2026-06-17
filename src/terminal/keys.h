/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KEYS_H
#define KEYS_H

#include <QtWidgets>

#include "numbertype.h"

namespace Keys {

enum Key {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Plus,
    Minus,
    Thru,
    Period,
    Set,
    Attribute,
    Model,
    Fixture,
    Group,
    Intensity,
    Color,
    Position,
    Raw,
    Effect,
    Cuelist,
    Cue,
};

QString keysToString(QList<Key> keys);
QVariant keysToNumber(QList<Keys::Key> keys, bool* ok, float currentValue, NumberType number);
bool isItemKey(Key key);

}

#endif // KEYS_H
