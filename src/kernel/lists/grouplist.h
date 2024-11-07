/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GROUPLIST_H
#define GROUPLIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/group.h"

class Kernel;

class GroupList : public ItemList<Group> {
    Q_OBJECT
public:
    GroupList(Kernel *core);
    Group* getItem(QString id) const override;
    int getItemRow(QString id) const override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    void deleteFixture(Fixture *fixture);
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordGroupFixtures(QList<QString> ids, QList<QString> fixtureIds);
    QList<QString> getIds() const override;
private:
    Group* recordGroup(QString id);
};

#include "kernel/kernel.h"

#endif // GROUPLIST_H
