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

void ColorList::setAttribute(QList<QString> ids, QString attribute, float value) {
    if (attribute == "2") {
        if (value >= 360 || value < 0) {
            kernel->terminal->error("Can't set Color Hue because Hue only allows values from 0° and smaller than 360°.");
            return;
        }
        for (QString id : ids) {
            Color* color = getItem(id);
            if (color == nullptr) {
                color = recordItem(id);
            }
            color->hue = value;
            emit dataChanged(index(getItemRow(color->id), 0), index(getItemRow(color->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Hue of " + QString::number(ids.length()) + " Colors to " + QString::number(value) + "°.");
    } else if (attribute == "3") {
        if (value > 100 || value < 0) {
            kernel->terminal->error("Can't set Color Saturation because Saturation only allows values from 0% to 100%.");
            return;
        }
        for (QString id : ids) {
            Color* color = getItem(id);
            if (color == nullptr) {
                color = recordItem(id);
            }
            color->saturation = value;
            emit dataChanged(index(getItemRow(color->id), 0), index(getItemRow(color->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Saturation of " + QString::number(ids.length()) + " Colors to " + QString::number(value) + "%.");
    } else {
        kernel->terminal->error("Can't set Color attribute " + attribute + ".");
    }
}
