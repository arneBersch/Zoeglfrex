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

    cueLabel = new QLabel();
    layout->addWidget(cueLabel);

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
    cueLabel->setText(QString());
    if ((cueViewModeComboBox->currentText() == CUEVIEWCUEMODE) && (currentCue != nullptr)) {
        cueLabel->setText(currentCue->name());
    } else if ((cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE) && (currentGroup != nullptr)) {
        cueLabel->setText(currentGroup->name());
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
        if (kernel->cues->rowCount() > 0) {
            currentCue = kernel->cues->items[0];
            loadCue();
        }
    }
    Cue* previousCue = nullptr;
    for (Cue* cue : kernel->cues->items) {
        if (cue == currentCue && previousCue != nullptr) {
            currentCue = previousCue;
            loadCue();
            return;
        }
        previousCue = cue;
    }
}

void CuelistView::nextCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        if (kernel->cues->rowCount() > 0) {
            currentCue = kernel->cues->items[0];
            loadCue();
        }
    }
    bool nextCue = false;
    for (Cue* cue : kernel->cues->items) {
        if (nextCue) {
            currentCue = cue;
            loadCue();
            return;
        }
        if (cue == currentCue) {
            nextCue = true;
        }
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
