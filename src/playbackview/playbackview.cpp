/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "playbackview.h"
#include "kernel/kernel.h"

PlaybackView::PlaybackView(Kernel* core) {
    kernel = core;

    kernel->mainWindow->setupShortcuts(this);
    setWindowTitle("Zöglfrex Playback View");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    tableView = new QTableView();
    layout->addWidget(tableView);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->hide();

    cuelistModel = new CuelistModel(kernel);
    tableView->setModel(cuelistModel);
}

void PlaybackView::reset() {
    cuelistModel->reset();
    for (Cuelist* cuelist : kernel->cuelists->items) {
        int cuelistRow = kernel->cuelists->items.indexOf(cuelist);
        QPushButton* goButton = new QPushButton("GO");
        connect(goButton, &QPushButton::clicked, this, [cuelist] { cuelist->go(); });
        tableView->setIndexWidget(cuelistModel->index(cuelistRow, CuelistModelColumns::goButton), goButton);

        QPushButton* goBackButton = new QPushButton("GO BACK");
        connect(goBackButton, &QPushButton::clicked, this, [cuelist] { cuelist->goBack(); });
        tableView->setIndexWidget(cuelistModel->index(cuelistRow, CuelistModelColumns::goBackButton), goBackButton);
    }
}
