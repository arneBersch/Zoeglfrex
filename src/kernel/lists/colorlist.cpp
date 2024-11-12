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

Color* ColorList::recordItem(QString id) {
    Color* color = new Color(kernel);
    color->id = id;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    beginInsertRows(QModelIndex(), position, position);
    items.insert(position, color);
    endInsertRows();
    return color;
}

void ColorList::recordColorRed(QList<QString> ids, float red) {
    if (red > 100 || red < 0) {
        kernel->terminal->error("Didn't record Color because Record Color Red only allows values from 0% to 100%.");
        return;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordItem(id);
        }
        color->red = red;
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Colors with Red value " + QString::number(red) + "%.");
}

void ColorList::recordColorGreen(QList<QString> ids, float green) {
    if (green > 100 || green < 0) {
        kernel->terminal->error("Didn't record Color because Record Color Green only allows values from 0% to 100%.");
        return;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordItem(id);
        }
        color->green = green;
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Colors with Green value " + QString::number(green) + "%.");
}

void ColorList::recordColorBlue(QList<QString> ids, float blue) {
    if (blue > 100 || blue < 0) {
        kernel->terminal->error("Didn't record Color because Record Color Blue only allows values from 0% to 100%.");
        return;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordItem(id);
        }
        color->blue = blue;
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Colors with Blue value " + QString::number(blue) + "%.");
}
