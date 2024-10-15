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
    table = new QTableView();
    layout->addWidget(table);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
}

void Inspector::loadModels()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->models);
    title->setText("Models");
}

void Inspector::loadFixtures()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->fixtures);
    title->setText("Fixtures");
}

void Inspector::loadGroups()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->groups);
    title->setText("Groups");
}

void Inspector::loadIntensities()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->intensities);
    title->setText("Intensities");
}

void Inspector::loadColors()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->colors);
    title->setText("Colors");
}

void Inspector::loadTransitions()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->transitions);
    title->setText("Transitions");
}

void Inspector::loadCues()
{
    table->setModel(kernel->rows);
    table->setModel(kernel->cues);
    title->setText("Cues");
}

void Inspector::loadRows()
{
    table->setModel(kernel->cues);
    table->setModel(kernel->rows);
    title->setText("Rows");
}
