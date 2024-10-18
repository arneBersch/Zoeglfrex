/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INTENSITYLIST_H
#define INTENSITYLIST_H

#include <QtWidgets>

class Kernel;

struct Intensity {
    QString id;
    QString label;
    float dimmer;
};

namespace IntensityListColumns {
enum {
    id,
    label,
    dimmer,
};
}


class IntensityList : public QAbstractTableModel {
    Q_OBJECT
public:
    IntensityList(Kernel *core);
    Intensity* getIntensity(QString id);
    int getIntensityRow(QString id);
    QString copyIntensity(QList<QString> ids, QString targetId);
    QString deleteIntensity(QList<QString> ids);
    QString labelIntensity(QList<QString> ids, QString label);
    QString moveIntensity(QList<QString> ids, QString targetId);
    QString recordIntensityDimmer(QList<QString> ids, float dimmer);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    QList<QString> getIds();
private:
    QList<Intensity*> intensities;
    Kernel *kernel;
    Intensity* recordIntensity(QString id);
};

#include "kernel/kernel.h"

#endif // INTENSITYLIST_H
