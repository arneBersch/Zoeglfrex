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
    layout->addWidget(infos);
}

void Inspector::load(QList<int> keys)
{
    infos->setText(QString());
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
    QString id = QString();
    if (!kernel->terminal->keysToSelection(itemIdKeys, itemType).isEmpty()) {
        id = kernel->terminal->keysToSelection(itemIdKeys, itemType).last();
    }
    Item* item = nullptr;
    if (itemType == Keys::Model) {
        title->setText("Models");
        table->setModel(kernel->models);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentFixture != nullptr) && (kernel->cuelistView->currentFixture->model != nullptr)) {
            item = kernel->cuelistView->currentFixture->model;
        } else {
            item = kernel->models->getItem(id);
        }
    } else if (itemType == Keys::Fixture) {
        title->setText("Fixtures");
        table->setModel(kernel->fixtures);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentFixture != nullptr)) {
            item = kernel->cuelistView->currentFixture;
        } else if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentGroup != nullptr) && !kernel->cuelistView->currentGroup->fixtures.isEmpty()) {
            item = kernel->cuelistView->currentGroup->fixtures.last();
        } else {
            item = kernel->fixtures->getItem(id);
        }
    } else if (itemType == Keys::Group) {
        title->setText("Groups");
        table->setModel(kernel->groups);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentGroup != nullptr)) {
            item = kernel->cuelistView->currentGroup;
        } else {
            item = kernel->groups->getItem(id);
        }
    } else if (itemType == Keys::Intensity) {
        title->setText("Intensities");
        table->setModel(kernel->intensities);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup);
        } else {
            item = kernel->intensities->getItem(id);
        }
    } else if (itemType == Keys::Color) {
        title->setText("Colors");
        table->setModel(kernel->colors);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup);
        } else {
            item = kernel->colors->getItem(id);
        }
    } else if (itemType == Keys::Position) {
        title->setText("Positions");
        table->setModel(kernel->positions);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup);
        } else {
            item = kernel->positions->getItem(id);
        }
    } else if (itemType == Keys::Raw) {
        title->setText("Raws");
        table->setModel(kernel->raws);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->raws.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCue->raws.value(kernel->cuelistView->currentGroup).last();
        } else {
            item = kernel->raws->getItem(id);
        }
    } else if (itemType == Keys::Effect) {
        title->setText("Effects");
        table->setModel(kernel->effects);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->effects.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCue->effects.value(kernel->cuelistView->currentGroup).last();
        } else {
            item = kernel->effects->getItem(id);
        }
    } else if (itemType == Keys::Cuelist) {
        title->setText("Cuelists");
        table->setModel(kernel->cuelists);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCuelist != nullptr)) {
            item = kernel->cuelistView->currentCuelist;
        } else {
            item = kernel->cuelists->getItem(id);
        }
    } else if (itemType == Keys::Cue) {
        title->setText("Cues");
        table->setModel(kernel->cues);
        if (itemIdKeys.isEmpty() && (kernel->cuelistView->currentCue != nullptr)) {
            item = kernel->cuelistView->currentCue;
        } else {
            item = kernel->cues->getItem(id);
        }
    } else {
        return;
    }
    if (item == nullptr) {
        return;
    }
    infos->setText(item->info());
}
