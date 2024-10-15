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

Color* ColorList::getColor(QString id)
{
    int colorRow = getColorRow(id);
    if (colorRow < 0) {
        return nullptr;
    }
    return colors[colorRow];
}

int ColorList::getColorRow(QString id)
{
    for (int colorRow = 0; colorRow < colors.size(); colorRow++) {
        if (colors[colorRow]->id == id) {
            return colorRow;
        }
    }
    return -1;
}

QString ColorList::copyColor(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Color* color = getColor(id);
        if (color == nullptr) {
            return "Color can't be copied because it doesn't exist.";
        }
        if (getColor(targetId) != nullptr) {
            return "Color can't be copied because Target ID is already used.";
        }
        Color *targetColor = recordColor(targetId);
        targetColor->label = color->label;
        targetColor->red = color->red;
        targetColor->green = color->green;
        targetColor->blue = color->blue;
    }
    return QString();
}

QString ColorList::deleteColor(QList<QString> ids)
{
    for (QString id : ids) {
        int colorRow = getColorRow(id);
        if (colorRow < 0) {
            return "Color can't be deleted because it doesn't exist.";
        }
        Color *color = colors[colorRow];
        kernel->cues->deleteColor(color);
        colors.removeAt(colorRow);
        delete color;
    }
    return QString();
}

QString ColorList::labelColor(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Color* color = getColor(id);
        if (color == nullptr) {
            return "Color can't be labeled because it doesn't exist.";
        }
        color->label = label;
    }
    return QString();
}

QString ColorList::moveColor(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int colorRow = getColorRow(id);
        if (colorRow < 0) {
            return "Color can't be moved because it doesn't exist.";
        }
        if (getColor(targetId) != nullptr) {
            return "Color can't be moved because Target ID is already used.";
        }
        Color* color = colors[colorRow];
        colors.removeAt(colorRow);
        color->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < colors.size(); index++) {
            QList<QString> indexIdParts = (colors[index]->id).split(".");
            if (indexIdParts[0].toInt() < idParts[0].toInt()) {
                position++;
            } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
                if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                    position++;
                } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                    if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                        position++;
                    } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                        if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                            position++;
                        } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                            if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                                position++;
                            }
                        }
                    }
                }
            }
        }
        colors.insert(position, color);
    }
    return QString();
}

Color* ColorList::recordColor(QString id)
{
    Color* color = new Color;
    color->id = id;
    color->label = QString();
    color->red = 100;
    color->green = 100;
    color->blue = 100;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < colors.size(); index++) {
        QList<QString> indexIdParts = (colors[index]->id).split(".");
        if (indexIdParts[0].toInt() < idParts[0].toInt()) {
            position++;
        } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
            if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                position++;
            } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                    position++;
                } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                    if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                        position++;
                    } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                        if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                            position++;
                        }
                    }
                }
            }
        }
    }
    colors.insert(position, color);
    return color;
}

QString ColorList::recordColorRed(QList<QString> ids, float red)
{
    if (red > 100 || red < 0) {
        return "Record Color Red only allows from 0% to 100%.";
    }
    for (QString id : ids) {
        Color* color = getColor(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->red = red;
    }
    return QString();
}

QString ColorList::recordColorGreen(QList<QString> ids, float green)
{
    if (green > 100 || green < 0) {
        return "Record Color Green only allows from 0% to 100%.";
    }
    for (QString id : ids) {
        Color* color = getColor(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->green = green;
    }
    return QString();
}

QString ColorList::recordColorBlue(QList<QString> ids, float blue)
{
    if (blue > 100 || blue < 0) {
        return "Record Color Blue only allows from 0% to 100%.";
    }
    for (QString id : ids) {
        Color* color = getColor(id);
        if (color == nullptr) {
            color = recordColor(id);
        }
        color->blue = blue;
    }
    return QString();
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

int ColorList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant ColorList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == ColorListColumns::id) {
            return colors[row]->id;
        } else if (column == ColorListColumns::label) {
            return colors[row]->label;
        } else if (column == ColorListColumns::red) {
            return colors[row]->red;
        } else if (column == ColorListColumns::green) {
            return colors[row]->green;
        } else if (column == ColorListColumns::blue) {
            return colors[row]->blue;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ColorList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == ColorListColumns::id) {
            return "ID";
        } else if (column == ColorListColumns::label) {
            return "Label";
        } else if (column == ColorListColumns::red) {
            return "Red (%)";
        } else if (column == ColorListColumns::green) {
            return "Green (%)";
        } else if (column == ColorListColumns::blue) {
            return "Blue (%)";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
