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
    infos->setWordWrap(true);
    infos->setStyleSheet("border: 1px solid white; padding: 10px;");
    layout->addWidget(infos);
    infos->hide();
}

void Inspector::load(QList<int> keys)
{
    table->reset();
    int itemType = Keys::Zero;
    QList<int> itemIdKeys;
    bool addToId = true;
    for (int key : keys) {
        if (kernel->terminal->isItem(key)) {
            itemIdKeys.clear();
            addToId = true;
            itemType = key;
        } else if ((key == Keys::Attribute) || (key == Keys::Set)) {
            addToId = false;
        } else if (addToId) {
            itemIdKeys.append(key);
        }
    }
    ids = kernel->terminal->keysToSelection(itemIdKeys, itemType);
    QString id = QString();
    if (!ids.isEmpty()) {
        id = ids.last();
    }
    Item* item = nullptr;
    if (itemType == Keys::Model) {
        title->setText("Models");
        table->setModel(kernel->models);
        item = kernel->models->getItem(id);
    } else if (itemType == Keys::Fixture) {
        title->setText("Fixtures");
        table->setModel(kernel->fixtures);
        item = kernel->fixtures->getItem(id);
    } else if (itemType == Keys::Group) {
        title->setText("Groups");
        table->setModel(kernel->groups);
        item = kernel->groups->getItem(id);
    } else if (itemType == Keys::Intensity) {
        title->setText("Intensities");
        table->setModel(kernel->intensities);
        item = kernel->intensities->getItem(id);
    } else if (itemType == Keys::Color) {
        title->setText("Colors");
        table->setModel(kernel->colors);
        item = kernel->colors->getItem(id);
    } else if (itemType == Keys::Position) {
        title->setText("Positions");
        table->setModel(kernel->positions);
        item = kernel->positions->getItem(id);
    } else if (itemType == Keys::Raw) {
        title->setText("Raws");
        table->setModel(kernel->raws);
        item = kernel->raws->getItem(id);
    } else if (itemType == Keys::Effect) {
        title->setText("Effects");
        table->setModel(kernel->effects);
        item = kernel->effects->getItem(id);
    } else if (itemType == Keys::Cuelist) {
        title->setText("Cuelists");
        table->setModel(kernel->cuelists);
        item = kernel->cuelists->getItem(id);
    } else if (itemType == Keys::Cue) {
        Cuelist* cuelist = nullptr;
        if (keys.startsWith(Keys::Cuelist)) {
            QList<int> cuelistIdKeys;
            int keysIndex = 1;
            while ((keysIndex < keys.size()) && (keys[keysIndex] != Keys::Attribute) && (keys[keysIndex] != Keys::Set) && !kernel->terminal->isItem(keys[keysIndex])) {
                cuelistIdKeys.append(keys[keysIndex]);
                keysIndex++;
            }
            cuelist = kernel->cuelists->getItem(kernel->terminal->keysToId(cuelistIdKeys));
        } else {
            cuelist = kernel->cuelistView->currentCuelist;
        }
        if (cuelist == nullptr) {
            title->setText("Cues (No Cuelist selected!)");
            table->setModel(nullptr);
        } else {
            title->setText("Cues (Cuelist " + cuelist->name() + ")");
            table->setModel(cuelist->cues);
            item = cuelist->cues->getItem(id);
        }
    } else {
        return;
    }
    if (item == nullptr) {
        infos->hide();
    } else {
        infos->setText(item->info());
        infos->show();
    }
}
