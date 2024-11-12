/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FIXTURELIST_H
#define FIXTURELIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/fixture.h"

class Kernel;

template class ItemList<Fixture>;
class FixtureList : public ItemList<Fixture> {
    Q_OBJECT
public:
    FixtureList(Kernel *core);
    void deleteModel(Model *model);
    void recordFixtureAddress(QList<QString> ids, int address);
    void recordFixtureModel(QList<QString> ids, QString model);
    QSet<int> usedChannels();
private:
    Fixture* recordItem(QString id, Model* model);
    int findFreeAddress(int channelCount);
};

#include "kernel/kernel.h"

#endif // FIXTURELIST_H
