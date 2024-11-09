/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colorlist.h"

ColorList::ColorList(Kernel *core) {
    kernel = core;
}

Color* ColorList::recordColor(QString id) {
    Color* color = new Color(kernel);
    color->id = id;
    color->label = QString();
    color->red = 100;
    color->green = 100;
    color->blue = 100;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, color);
    return color;
}

bool ColorList::recordColorRed(QList<QString> ids, float red) {
    if (red > 100 || red < 0) {
        kernel->terminal->error("Record Color Red only allows from 0% to 100%.");
        return false;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->red = red;
    }
    return true;
}

bool ColorList::recordColorGreen(QList<QString> ids, float green) {
    if (green > 100 || green < 0) {
        kernel->terminal->error("Record Color Green only allows from 0% to 100%.");
        return false;
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->green = green;
    }
    return true;
}

bool ColorList::recordColorBlue(QList<QString> ids, float blue) {
    if (blue > 100 || blue < 0) {
        return "Record Color Blue only allows from 0% to 100%.";
    }
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->blue = blue;
    }
    return true;
}
