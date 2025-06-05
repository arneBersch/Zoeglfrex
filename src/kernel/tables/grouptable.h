/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GROUPTABLE_H
#define GROUPTABLE_H

#include <QtWidgets>

#include "itemtable.h"
#include "../items/group.h"

class Kernel;

template class ItemTable<Group>;
class GroupTable : public ItemTable<Group> {
    Q_OBJECT
public:
    GroupTable(Kernel *core);
    void setAttribute(QStringList ids, QMap<int, QString> attribute, QList<int> value, QString text = QString()) override;
private:
    bool isCurrentItem(Group* item) const override;
};

#include "kernel/kernel.h"

#endif // GROUPTABLE_H
