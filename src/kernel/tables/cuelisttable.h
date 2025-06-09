/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELISTTABLE_H
#define CUELISTTABLE_H

#include <QtWidgets>

#include "itemtable.h"
#include "../items/cuelist.h"

class Kernel;

template class ItemTable<Cuelist>;
class CuelistTable : public ItemTable<Cuelist> {
    Q_OBJECT
public:
    CuelistTable(Kernel *core);
    void setAttribute(QStringList ids, QMap<int, QString> attribute, QList<int> value, QString text = QString()) override;
private:
    Cuelist* addItem(QString id) override;
};

#include "kernel/kernel.h"

#endif // CUELISTTABLE_H
