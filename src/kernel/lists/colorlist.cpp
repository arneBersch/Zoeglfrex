/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colorlist.h"

ColorList::ColorList(Kernel *core) : ItemList(Keys::Color, "Color", "Colors") {
    kernel = core;
    floatAttributes[SATURATIONATTRIBUTEID] ={"Saturation", 100, 0, 100, "%"};
}

void ColorList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attributeString = attributes.value(Keys::Attribute);
    if (attributeString == HUEATTRIBUTEID) {
        float hue = kernel->keysToValue(value);
        if (hue >= 360 || hue < 0) {
            kernel->terminal->error("Can't set Hue because Hue only allows values from 0° and smaller than 360°.");
            return;
        }
        for (QString id : ids) {
            Color* color = getItem(id);
            if (color == nullptr) {
                color = addItem(id);
            }
            color->hue = hue;
            emit dataChanged(index(getItemRow(color->id), 0), index(getItemRow(color->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Hue of " + QString::number(ids.length()) + " Colors to " + QString::number(hue) + "°.");
    } else {
        kernel->terminal->error("Can't set Color Attribute " + attributeString + ".");
    }
}
