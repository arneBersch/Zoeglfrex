/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUETABLE_H
#define CUETABLE_H

#include <QtWidgets>

#include "itemtable.h"
#include "../items/cue.h"

class Kernel;

template class ItemTable<Cue>;
class CueTable : public ItemTable<Cue> {
    Q_OBJECT
public:
    CueTable(Kernel *core);
    void setAttribute(QStringList ids, QMap<int, QString> attribute, QList<int> value, QString text = QString()) override;
private:
    Cue* addItem(QString id) override;
    bool isCurrentItem(Cue* item) const override;
};

#endif // CUETABLE_H
