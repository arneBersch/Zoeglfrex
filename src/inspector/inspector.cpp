/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "inspector.h"

Inspector::Inspector(Kernel *core, QWidget *parent) : QWidget{parent}
{
    kernel = core;
    QVBoxLayout *layout = new QVBoxLayout(this);
    title = new QLabel();
    layout->addWidget(title);
    table = new QListView();
    layout->addWidget(table);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    infos = new QLabel();
    layout->addWidget(infos);
}

void Inspector::loadItemList(int key)
{
    if (key == Keys::Model) {
        table->setModel(kernel->models);
        title->setText("Models");
    } else if (key == Keys::Fixture) {
        table->setModel(kernel->fixtures);
        title->setText("Fixtures");
    } else if (key == Keys::Group) {
        table->setModel(kernel->groups);
        title->setText("Groups");
    } else if (key == Keys::Intensity) {
        table->setModel(kernel->intensities);
        title->setText("Intensities");
    } else if (key == Keys::Color) {
        table->setModel(kernel->colors);
        title->setText("Colors");
    } else if (key == Keys::Color) {
        table->setModel(kernel->cues);
        title->setText("Cues");
    }
}
