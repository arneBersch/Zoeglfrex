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

Color* ColorList::getItem(QString id)
{
    int colorRow = getItemRow(id);
    if (colorRow < 0) {
        return nullptr;
    }
    return colors[colorRow];
}

int ColorList::getItemRow(QString id)
{
    for (int colorRow = 0; colorRow < colors.size(); colorRow++) {
        if (colors[colorRow]->id == id) {
            return colorRow;
        }
    }
    return -1;
}

bool ColorList::copyItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Color* color = getItem(id);
        if (color == nullptr) {
            kernel->terminal->error("Color can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Color can't be copied because Target ID is already used.");
            return false;
        }
        Color *targetColor = recordColor(targetId);
        targetColor->label = color->label;
        targetColor->red = color->red;
        targetColor->green = color->green;
        targetColor->blue = color->blue;
    }
    return true;
}

bool ColorList::deleteItems(QList<QString> ids)
{
    for (QString id : ids) {
        int colorRow = getItemRow(id);
        if (colorRow < 0) {
            kernel->terminal->error("Color can't be deleted because it doesn't exist.");
            return false;
        }
        Color *color = colors[colorRow];
        kernel->cues->deleteColor(color);
        colors.removeAt(colorRow);
        delete color;
    }
    return true;
}

bool ColorList::moveItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int colorRow = getItemRow(id);
        if (colorRow < 0) {
            kernel->terminal->error("Color can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Color can't be moved because Target ID is already used.");
            return false;
        }
        Color* color = colors[colorRow];
        colors.removeAt(colorRow);
        color->id = targetId;
        int position = 0;
        for (int index=0; index < colors.size(); index++) {
            if (greaterId(colors[index]->id, targetId)) {
                position++;
            }
        }
        colors.insert(position, color);
    }
    return true;
}

Color* ColorList::recordColor(QString id)
{
    Color* color = new Color;
    color->id = id;
    color->label = QString();
    color->red = 100;
    color->green = 100;
    color->blue = 100;
    int position = 0;
    for (int index=0; index < colors.size(); index++) {
        if (greaterId(colors[index]->id, id)) {
            position++;
        }
    }
    colors.insert(position, color);
    return color;
}

bool ColorList::recordColorRed(QList<QString> ids, float red)
{
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

bool ColorList::recordColorGreen(QList<QString> ids, float green)
{
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

bool ColorList::recordColorBlue(QList<QString> ids, float blue)
{
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

QList<QString> ColorList::getIds() {
    QList<QString> ids;
    for (Color *color : colors) {
        ids.append(color->id);
    }
    return ids;
}

int ColorList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return colors.size();
}

QVariant ColorList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        return colors[row]->id;
    }
    return QVariant();
}
