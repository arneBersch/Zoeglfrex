/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INTENSITYLIST_H
#define INTENSITYLIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/intensity.h"

class Kernel;

template class ItemList<Intensity>;
class IntensityList : public ItemList<Intensity> {
    Q_OBJECT
public:
    IntensityList(Kernel *core);
    Intensity* getItem(QString id) const override;
    int getItemRow(QString id) const override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordIntensityDimmer(QList<QString> ids, float dimmer);
    QList<QString> getIds() const override;
private:
    Intensity* recordIntensity(QString id);
};

#include "kernel/kernel.h"

#endif // INTENSITYLIST_H
