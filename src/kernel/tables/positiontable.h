/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef POSITIONTABLE_H
#define POSITIONTABLE_H

#include <QtWidgets>

#include "itemtable.h"
#include "../items/position.h"

class Kernel;

template class ItemTable<Position>;
class PositionTable : public ItemTable<Position> {
    Q_OBJECT
public:
    PositionTable(Kernel *core);
};

#include "kernel/kernel.h"

#endif // POSITIONTABLE_H
