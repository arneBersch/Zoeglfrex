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

void CuelistView::loadView() {
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
        fixtureLabel->setText("Fixture " + currentFixture->name());
    }
    if ((currentCue == nullptr) && !kernel->cues->items.isEmpty()) {
        currentCue = kernel->cues->items.first();
        loadView();
        return;
    }
    if ((currentGroup == nullptr) && !kernel->groups->items.isEmpty()) {
        currentGroup = kernel->groups->items.first();
        loadView();
        return;
    }
}

void CuelistView::loadCue(QString cueId) {
    Cue* cue = kernel->cues->getItem(cueId);
    if (cue == nullptr) {
        kernel->terminal->error("Can't select Cue because Cue " + cueId + " doesn't exist.");
        return;
    }
    currentCue = cue;
    loadView();
    if ((cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE)) {
        cuelistTableView->scrollTo(groupModel->index(kernel->cues->items.indexOf(currentCue), GroupModelColumns::cue));
    }
}

void CuelistView::previousCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        loadView();
        return;
    }
    if (kernel->cues->items.indexOf(currentCue) > 0) {
        loadCue(kernel->cues->items[kernel->cues->items.indexOf(currentCue) - 1]->id);
    }
}

void CuelistView::nextCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        loadView();
        return;
    }
    if ((kernel->cues->items.indexOf(currentCue) + 1) < kernel->cues->items.length()) {
        loadCue(kernel->cues->items[kernel->cues->items.indexOf(currentCue) + 1]->id);
    }
}

void CuelistView::loadGroup(QString groupId) {
    Group* group = kernel->groups->getItem(groupId);
    if (group == nullptr) {
        kernel->terminal->error("Can't select Group because Group " + groupId + " doesn't exist.");
        return;
    }
    currentGroup = group;
    currentFixture = nullptr;
    loadView();
    if ((cueViewModeComboBox->currentText() == CUEVIEWCUEMODE)) {
        cuelistTableView->scrollTo(cueModel->index(kernel->groups->items.indexOf(currentGroup), CueModelColumns::group));
    }
}

void CuelistView::previousGroup() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        loadView();
        return;
    }
    if (kernel->groups->items.indexOf(currentGroup) > 0) {
        loadGroup(kernel->groups->items[kernel->groups->items.indexOf(currentGroup) - 1]->id);
    }
}

void CuelistView::nextGroup() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        loadView();
        return;
    }
    if ((kernel->groups->items.indexOf(currentGroup) + 1) < kernel->groups->items.length()) {
        loadGroup(kernel->groups->items[kernel->groups->items.indexOf(currentGroup) + 1]->id);
    }
}

void CuelistView::previousFixture() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        return;
    }
    if (currentFixture == nullptr) {
        if (!currentGroup->fixtures.isEmpty()) {
            currentFixture = currentGroup->fixtures.first();
            loadView();
        }
        return;
    }
    if (currentGroup->fixtures.indexOf(currentFixture) > 0) {
        currentFixture = currentGroup->fixtures[currentGroup->fixtures.indexOf(currentFixture) - 1];
        loadView();
    }
}

void CuelistView::nextFixture() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        return;
    }
    if (currentFixture == nullptr) {
        if (!currentGroup->fixtures.isEmpty()) {
            currentFixture = currentGroup->fixtures.first();
            loadView();
        }
        return;
    }
    if ((currentGroup->fixtures.indexOf(currentFixture) + 1) < currentGroup->fixtures.length()) {
        currentFixture = currentGroup->fixtures[currentGroup->fixtures.indexOf(currentFixture) + 1];
        loadView();
    }
}

void CuelistView::updateCuelistView() {
    if (cueViewModeComboBox->currentText() == CUEVIEWCUEMODE) {
        cuelistTableView->setModel(cueModel);
        if (currentGroup != nullptr) {
            cuelistTableView->scrollTo(cueModel->index(kernel->groups->items.indexOf(currentGroup), CueModelColumns::group));
        }
    } else if (cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE) {
        cuelistTableView->setModel(groupModel);
        if (currentCue != nullptr) {
            cuelistTableView->scrollTo(groupModel->index(kernel->cues->items.indexOf(currentCue), GroupModelColumns::cue));
        }
    }
    loadView();
    if ((cueViewModeComboBox->currentText() == CUEVIEWCUEMODE) && (currentGroup != nullptr)) {
        cuelistTableView->scrollTo(cueModel->index(kernel->groups->items.indexOf(currentGroup), CueModelColumns::group));
    } else if ((cueViewModeComboBox->currentText() == CUEVIEWGROUPMODE) && (currentCue != nullptr)) {
        cuelistTableView->scrollTo(groupModel->index(kernel->cues->items.indexOf(currentCue), GroupModelColumns::cue));
    }
}
