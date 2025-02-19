/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawlist.h"

RawList::RawList(Kernel *core) : ItemList("Raw", "Raws") {
    kernel = core;
}

void RawList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> value, QString text) {
    QString attributeString = attribute.value(Keys::Attribute);
    if (attributeString == CHANNELATTRIBUTEID) {
        int channel = kernel->keysToValue(value);
        if ((channel > 512) || (channel < 1)) {
            kernel->terminal->error("Can't set Channel because Channel only allows values from 1 to 512.");
            return;
        }
        for (QString id : ids) {
            Raw* raw = getItem(id);
            if (raw == nullptr) {
                raw = addItem(id);
            }
            raw->channel = channel;
            emit dataChanged(index(getItemRow(raw->id), 0), index(getItemRow(raw->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Channel of " + QString::number(ids.length()) + " Raws to " + QString::number(channel) + "°.");
    } else if (attributeString == VALUEATTRIBUTEID) {
        int channelValue = kernel->keysToValue(value);
        if ((channelValue > 255) || (channelValue < 0)) {
            kernel->terminal->error("Can't set Raw Value because Value only allows values from 0 to 255.");
            return;
        }
        for (QString id : ids) {
            Raw* raw = getItem(id);
            if (raw == nullptr) {
                raw = addItem(id);
            }
            raw->value = channelValue;
            emit dataChanged(index(getItemRow(raw->id), 0), index(getItemRow(raw->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Value of " + QString::number(ids.length()) + " Raws to " + QString::number(channelValue) + ".");
    } else {
        kernel->terminal->error("Can't set Raw Attribute " + attributeString + ".");
    }
}
