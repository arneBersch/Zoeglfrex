/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELIST_H
#define CUELIST_H

#include <QtWidgets>

#include "itemlist.h"
#include "../items/cue.h"

class Kernel;

class CueList : public ItemList {
    Q_OBJECT
public:
    CueList(Kernel *core);
    Cue* getItem(QString id) override;
    int getItemRow(QString id) override;
    bool copyItems(QList<QString> ids, QString targetId) override;
    bool deleteItems(QList<QString> ids) override;
    bool deleteCueGroupIntensity(QList<QString> ids, QString groupId);
    bool deleteCueGroupColor(QList<QString> ids, QString groupId);
    void deleteIntensity(Intensity *intensity);
    void deleteColor(Color *color);
    void deleteTransition(Transition *transition);
    void deleteGroup(Group *group);
    bool moveItems(QList<QString> ids, QString targetId) override;
    bool recordCueTransition(QList<QString> ids, QString transitionId);
    bool recordCueIntensity(QList<QString> ids, QString groupId, QString intensityId);
    bool recordCueColor(QList<QString> ids, QString groupId, QString colorId);
    QList<QString> getIds();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
private:
    QList<Cue*> items;
    Cue* recordCue(QString id, Transition *transition);
};

#include "kernel/kernel.h"

#endif // CUELIST_H
