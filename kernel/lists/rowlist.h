/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ROWLIST_H
#define ROWLIST_H

#include <QtWidgets>

class Kernel;
struct Group;

struct Row {
    QString id;
    QString label;
    Group* group;
};

namespace RowListColumns {
enum {
    id,
    label,
    group,
};
}

class RowList : public QAbstractTableModel {
    Q_OBJECT
public:
    RowList(Kernel *core);
    Row* getRow(QString id);
    int getRowRow(QString id);
    QString copyRow(QList<QString> ids, QString targetId);
    QString deleteRow(QList<QString> ids);
    void deleteGroup(Group* group);
    QString labelRow(QList<QString> ids, QString label);
    QString moveRow(QList<QString> ids, QString targetId);
    QString recordRowGroup(QList<QString> ids, QString groupId);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    QList<QString> getIds();
private:
    QList<Row*> rows;
    Kernel *kernel;
    Row* recordRow(QString id, Group *group);
};

#include "kernel/kernel.h"

#endif // ROWLIST_H
