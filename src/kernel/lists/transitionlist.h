/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TRANSITIONLIST_H
#define TRANSITIONLIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/transition.h"

class Kernel;

class TransitionList : public ItemList {
    Q_OBJECT
public:
    TransitionList(Kernel *core);
    Transition* getItem(QString id) override;
    int getItemRow(QString id) override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList <QString> ids) override;
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordTransitionFade(QList<QString> ids, float fadeIn);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QList<QString> getIds();
private:
    QList<Transition*> transitions;
    Transition* recordTransition(QString id);
};

#include "kernel/kernel.h"

#endif // TRANSITIONLIST_H
