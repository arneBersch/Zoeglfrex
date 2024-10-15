/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef POSITIONLIST_H
#define POSITIONLIST_H

#include <QtWidgets>

class CueList;

struct Position {
    QString id;
    QString label;
    float pan;
    float tilt;
};

namespace PositionListColumns {
enum {
    id,
    label,
    pan,
    tilt,
};
}

class PositionList : public QAbstractTableModel
{
    Q_OBJECT
public:
    PositionList();
    void setLists(CueList *cueList);
    Position* getPosition(QString id);
    int getPositionRow(QString id);
    QString copyPosition(QList<QString> ids, QString targetId);
    QString deletePosition(QList<QString> ids);
    QString labelPosition(QList<QString> ids, QString label);
    QString movePosition(QList<QString> ids, QString targetId);
    QString recordPositionPan(QList<QString> ids, float pan);
    QString recordPositionTilt(QList<QString> ids, float tilt);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
private:
    QList<Position*> positions;
    Position* recordPosition(QString id);
    CueList *cues;
};

#include "cuelist.h"

#endif // POSITIONLIST_H
