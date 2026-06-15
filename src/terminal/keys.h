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

namespace Keys {

enum Key {
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Nine = 9,
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

}

#endif // KEYS_H
