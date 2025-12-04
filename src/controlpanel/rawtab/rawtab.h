/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RAWTAB_H
#define RAWTAB_H

#include <QtWidgets>
#include <QtSql>

#include "fixturechannelmodel.h"

class RawTab : public QWidget {
    Q_OBJECT
public:
    RawTab(QWidget* parent = nullptr);
public slots:
    void reload();
signals:
    void dbChanged();
private:
    QComboBox* rawComboBox;
    QList<int> rawKeys;
    FixtureChannelModel* model;
    QTableView* tableView;
};

#endif // RAWTAB_H
