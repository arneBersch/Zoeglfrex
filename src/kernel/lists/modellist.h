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

class ModelList : public ItemList {
    Q_OBJECT
public:
    ModelList(Kernel *core);
    Model* getItem(QString id) override;
    int getItemRow(QString id) override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordModelChannels(QList<QString> ids, QString channels);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QList<QString> getIds();
private:
    QList<Model*> models;
    Kernel *kernel;
    Model* recordModel(QString id);
};

#include "kernel/kernel.h"

#endif // MODELLIST_H
