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

class Kernel;

struct Transition {
    QString id;
    QString label;
    float fade;
};

namespace TransitionListColumns {
enum {
    id,
    label,
    fade,
};
}

class TransitionList : public QAbstractTableModel {
    Q_OBJECT
public:
    TransitionList(Kernel *core);
    Transition* getTransition(QString id);
    QString copyTransition(QList<QString> ids, QString targetId);
    QString deleteTransition(QList <QString> ids);
    QString labelTransition(QList<QString> ids, QString label);
    QString moveTransition(QList<QString> ids, QString targetId);
    QString recordTransitionFade(QList<QString> ids, float fadeIn);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    QList<QString> getIds();
private:
    QList<Transition*> transitions;
    Kernel *kernel;
    Transition* recordTransition(QString id);
    int getTransitionRow(QString id);
};

#include "kernel/kernel.h"

#endif // TRANSITIONLIST_H
