/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "keys.h"

QString Keys::keysToString(QList<Key> keys) {
    QHash<Key, QString> keyStrings;
    keyStrings[Zero] = "0";
    keyStrings[One] = "1";
    keyStrings[Two] = "2";
    keyStrings[Three] = "3";
    keyStrings[Four] = "4";
    keyStrings[Five] = "5";
    keyStrings[Six] = "6";
    keyStrings[Seven] = "7";
    keyStrings[Eight] = "8";
    keyStrings[Nine] = "9";
    keyStrings[Plus] = " + ";
    keyStrings[Minus] = " - ";
    keyStrings[Thru] = " Thru ";
    keyStrings[Period] = ".";
    keyStrings[Set] = " Set ";
    keyStrings[Attribute] = " Attribute ";
    keyStrings[Model] = " Model ";
    keyStrings[Fixture] = " Fixture ";
    keyStrings[Group] = " Group ";
    keyStrings[Intensity] = " Intensity ";
    keyStrings[Color] = " Color ";
    keyStrings[Position] = " Position ";
    keyStrings[Raw] = " Raw ";
    keyStrings[Effect] = " Effect ";
    keyStrings[Cuelist] = " Cuelist ";
    keyStrings[Cue] = " Cue ";

    QString result;
    for(const Key key: keys) {
        Q_ASSERT(keyStrings.contains(key));
        result.append(keyStrings.value(key));
    }
    return result.simplified();
}

bool Keys::isItemKey(Key key) {
    const QList<Key> itemKeys = {
        Model,
        Fixture,
        Group,
        Intensity,
        Color,
        Position,
        Raw,
        Effect,
        Cuelist,
        Cue
    };

    return itemKeys.contains(key);
}