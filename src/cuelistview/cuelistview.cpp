/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelistview.h"

CuelistView::CuelistView(Kernel *core, QWidget *parent) : QWidget {parent} {
    kernel = core;

    QVBoxLayout *layout = new QVBoxLayout(this);

    cueOrGroupLabel = new QLabel();
    layout->addWidget(cueOrGroupLabel);
    fixtureLabel = new QLabel();
    layout->addWidget(fixtureLabel);

    cueViewModeComboBox = new QComboBox();
    cueViewModeComboBox->addItem(CUEVIEWCUEMODE);
    cueViewModeComboBox->addItem(CUEVIEWGROUPMODE);
    connect(cueViewModeComboBox, &QComboBox::currentIndexChanged, this, &CuelistView::updateCuelistView);
    layout->addWidget(cueViewModeComboBox);

    cueModel = new CueModel(kernel);
    groupModel = new GroupModel(kernel);

    cuelistTableView = new QTableView();
    cuelistTableView->setSelectionMode(QAbstractItemView::NoSelection);
    cuelistTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cuelistTableView->setFocusPolicy(Qt::NoFocus);
    cuelistTableView->verticalHeader()->hide();
    cuelistTableView->horizontalHeader()->setStretchLastSection(true);
    cuelistTableView->setModel(cueModel);
    layout->addWidget(cuelistTableView);

    dmxEngine = new DmxEngine(kernel);
    layout->addWidget(dmxEngine);
}

void CuelistView::loadCue() {
    cueModel->loadCue();
    groupModel->loadGroup();
    dmxEngine->generateDmx();
    cueOrGroupLabel->setText(QString());
    if ((cueViewModeComboBox->currentText() == CUEVIEWCUEMODE)) {
        cueOrGroupLabel->setText("No Cue selected.");
        if (currentCue != nullptr) {
            cueOrGroupLabel->setText("Cue " + currentCue->name());
        }
    } else if ((cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE)) {
        cueOrGroupLabel->setText("No Group selected.");
        if (currentGroup != nullptr) {
            cueOrGroupLabel->setText("Group " + currentGroup->name());
        }
    }
    fixtureLabel->setText("No Fixture selected.");
    if (currentFixture != nullptr) {
        fixtureLabel->setText("Fixture " + currentFixture->id);
    }
    if (currentCue == nullptr) {
        if (!kernel->cues->items.isEmpty()) {
            currentCue = kernel->cues->items.first();
            loadCue();
        }
        return;
    }
}

void CuelistView::previousCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        if (!kernel->cues->items.isEmpty()) {
            currentCue = kernel->cues->items.first();
            loadCue();
        }
        return;
    }
    if (kernel->cues->items.indexOf(currentCue) > 0) {
        currentCue = kernel->cues->items[kernel->cues->items.indexOf(currentCue) - 1];
        loadCue();
    }
}

void CuelistView::nextCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        if (!kernel->cues->items.isEmpty()) {
            currentCue = kernel->cues->items.first();
            loadCue();
        }
        return;
    }
    if ((kernel->cues->items.indexOf(currentCue) + 1) < kernel->cues->items.length()) {
        currentCue = kernel->cues->items[kernel->cues->items.indexOf(currentCue) + 1];
        loadCue();
    }
}

void CuelistView::updateCuelistView() {
    if (cueViewModeComboBox->currentText() == CUEVIEWCUEMODE) {
        cuelistTableView->setModel(cueModel);
    } else if (cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE) {
        cuelistTableView->setModel(groupModel);
    }
    loadCue();
}
