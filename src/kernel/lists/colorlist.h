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
#include "../items/color.h"

class Kernel;

class ColorList : public ItemList {
    Q_OBJECT
public:
    ColorList(Kernel *core);
    Color* getItem(QString color) const override;
    int getItemRow(QString id) const override;
    Color* getItemByRow(int row) const override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordColorRed(QList<QString> ids, float red);
    bool recordColorGreen(QList<QString> ids, float green);
    bool recordColorBlue(QList<QString> ids, float blue);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QList<QString> getIds() const override;
private:
    QList<Color*> items;
    Color* recordColor(QString id);
};

#include "kernel/kernel.h"

#endif // COLORLIST_H
