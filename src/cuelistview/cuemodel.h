/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUEMODEL_H
#define CUEMODEL_H

#include <QtWidgets>

class Kernel;

namespace CueModelColumns {
enum {
    group,
    intensity,
    color,
    raws,
    effects,
};
}

class CueModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CueModel(Kernel *core);
    void loadCue();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
private:
    Kernel *kernel;
};

#endif // CUEMODEL_H
