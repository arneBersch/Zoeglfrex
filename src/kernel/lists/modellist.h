/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MODELLIST_H
#define MODELLIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/model.h"

class Kernel;

template class ItemList<Model>;
class ModelList : public ItemList<Model> {
    Q_OBJECT
public:
    ModelList(Kernel *core);
    Model* getItem(QString id) const override;
    int getItemRow(QString id) const override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordModelChannels(QList<QString> ids, QString channels);
    QList<QString> getIds() const override;
private:
    Model* recordModel(QString id);
};

#include "kernel/kernel.h"

#endif // MODELLIST_H
