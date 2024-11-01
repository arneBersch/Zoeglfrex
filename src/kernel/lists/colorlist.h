/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COLORLIST_H
#define COLORLIST_H

#include <QtWidgets>

#include "itemlist.h"

class Kernel;

struct Color {
    QString id;
    QString label;
    float red;
    float green;
    float blue;
};

class ColorList : public ItemList {
    Q_OBJECT
public:
    ColorList(Kernel *core);
    Color* getColor(QString color);

    bool copyColor(QList<QString> ids, QString targetId);
    bool deleteColor(QList<QString> ids);
    bool labelColor(QList<QString> ids, QString label);
    bool moveColor(QList<QString> ids, QString targetId);
    bool recordColorRed(QList<QString> ids, float red);
    bool recordColorGreen(QList<QString> ids, float green);
    bool recordColorBlue(QList<QString> ids, float blue);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QList<QString> getIds();
private:
    QList<Color*> colors;
    Color* recordColor(QString id);
    int getColorRow(QString id);
    Kernel *kernel;
};

#include "kernel/kernel.h"

#endif // COLORLIST_H
