/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawlist.h"

RawList::RawList(Kernel *core) : ItemList(Keys::Raw, "Raw", "Raws") {
    kernel = core;
    intAttributes[CHANNELATTRIBUTEID] ={"Channel", 1, 1, 512};
    intAttributes[VALUEATTRIBUTEID] ={"Value", 0, 0, 255};
}

void RawList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    kernel->terminal->error("Can't set Raw Attribute " + attributes.value(Keys::Attribute) + ".");
}
