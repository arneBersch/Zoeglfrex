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

void Inspector::load(QList<int> keys)
{
    infos->setText(QString()); // reset Info label
    for (int keyIndex = (keys.length() - 1); keyIndex >= 0; keyIndex--) {
        QString id = QString();
        if (keyIndex < (keys.length() - 1)) {
            id = kernel->keysToId(keys.mid((keyIndex + 1), (keys.length() - keyIndex)));
        }
        if (keys[keyIndex] == Keys::Model) {
            table->setModel(kernel->models);
            title->setText("Models");
            Model* item = kernel->models->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        } else if (keys[keyIndex] == Keys::Fixture) {
            table->setModel(kernel->fixtures);
            title->setText("Fixtures");
            Fixture* item = kernel->fixtures->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        } else if (keys[keyIndex] == Keys::Group) {
            table->setModel(kernel->groups);
            title->setText("Groups");
            Group* item = kernel->groups->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        } else if (keys[keyIndex] == Keys::Intensity) {
            table->setModel(kernel->intensities);
            title->setText("Intensities");
            Intensity* item = kernel->intensities->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        } else if (keys[keyIndex] == Keys::Color) {
            table->setModel(kernel->colors);
            title->setText("Colors");
            Color* item = kernel->colors->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        } else if (keys[keyIndex] == Keys::Cue) {
            table->setModel(kernel->cues);
            title->setText("Cues");
            Cue* item = kernel->cues->getItem(id);
            if (item != nullptr) {
                infos->setText(item->info());
            }
            return;
        }
    }
}
