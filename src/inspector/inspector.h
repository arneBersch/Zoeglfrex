/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QtWidgets>

class Kernel;

class Inspector : public QWidget {
    Q_OBJECT
public:
    Inspector(Kernel *core, QWidget *parent = nullptr);
    void loadItemList(int key);
private:
    Kernel *kernel;
    QListView *table;
    QLabel *title;
    QLabel *infos;
};

#include "kernel/kernel.h"

#endif // INSPECTOR_H
