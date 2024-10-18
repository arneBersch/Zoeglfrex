/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FIXTURELIST_H
#define FIXTURELIST_H

#include <QtWidgets>

class Kernel;
struct Model;

struct Fixture {
    QString id;
    QString label;
    Model* model;
    int address;
};

namespace FixtureListColumns {
enum {
    id,
    label,
    model,
    address,
};
}

class FixtureList : public QAbstractTableModel {
    Q_OBJECT
public:
    FixtureList(Kernel *core);
    Fixture* getFixture(QString id);
    int getFixtureRow(QString id);
    QString copyFixture(QList<QString> ids, QString targetId);
    QString deleteFixture(QList<QString> ids);
    void deleteModel(Model *model);
    QString labelFixture(QList<QString> ids, QString label);
    QString moveFixture(QList<QString> ids, QString targetId);
    QString recordFixtureAddress(QList<QString> ids, int address);
    QString recordFixtureModel(QList<QString> ids, QString model, int address=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    QList<QString> getIds();
    QSet<int> usedChannels();
private:
    QList<Fixture*> fixtures;
    Kernel *kernel;
    Fixture* recordFixture(QString id, Model* model);
    int findFreeAddress(int channelCount);
};

#include "kernel/kernel.h"

#endif // FIXTURELIST_H
