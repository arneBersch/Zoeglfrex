/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colorlist.h"

ColorList::ColorList(Kernel *core) : ItemList("Color", "Colors") {
    kernel = core;
}

void ColorList::recordColorHue(QList<QString> ids, float hue) {
    if (hue >= 360 || hue < 0) {
        kernel->terminal->error("Didn't record Color because Record Color Hue only allows values greater or equal to 0° and smaller than 360°.");
        return;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordItem(id);
        }
        color->hue = hue;
        emit dataChanged(index(getItemRow(color->id), 0), index(getItemRow(color->id), 0), {Qt::DisplayRole, Qt::EditRole});
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Colors with Hue value " + QString::number(hue) + "°.");
}

void ColorList::recordColorSaturation(QList<QString> ids, float saturation) {
    if (saturation > 100 || saturation < 0) {
        kernel->terminal->error("Didn't record Color because Record Color Saturation only allows values from 0% to 100%.");
        return;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordItem(id);
        }
        color->saturation = saturation;
        emit dataChanged(index(getItemRow(color->id), 0), index(getItemRow(color->id), 0), {Qt::DisplayRole, Qt::EditRole});
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Colors with Saturation value " + QString::number(saturation) + "%.");
}
