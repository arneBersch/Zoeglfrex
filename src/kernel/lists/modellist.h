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

class Kernel;

struct Model {
    QString id;
    QString label;
    QString channels;
};

namespace ModelListColumns {
enum {
    id,
    label,
    channels,
};
}

class ModelList : public QAbstractTableModel {
    Q_OBJECT
public:
    ModelList(Kernel *core);
    Model* getModel(QString id);
    int getModelRow(QString id);
    QString copyModel(QList<QString> ids, QString targetId);
    QString deleteModel(QList<QString> ids);
    QString labelModel(QList<QString> ids, QString label);
    QString moveModel(QList<QString> ids, QString targetId);
    QString recordModelChannels(QList<QString> ids, QString channels);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    QList<QString> getIds();
private:
    QList<Model*> models;
    Kernel *kernel;
    Model* recordModel(QString id);
};

#include "kernel/kernel.h"

#endif // MODELLIST_H
