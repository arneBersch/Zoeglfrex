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

    QGridLayout* labelHeader = new QGridLayout();
    layout->addLayout(labelHeader);
    cuelistLabel = new QLabel();
    labelHeader->addWidget(cuelistLabel, 0, 0);
    cueLabel = new QLabel();
    labelHeader->addWidget(cueLabel, 0, 1);
    groupLabel = new QLabel();
    labelHeader->addWidget(groupLabel, 1, 0);
    fixtureLabel = new QLabel();
    labelHeader->addWidget(fixtureLabel, 1, 1);

    QHBoxLayout *buttonHeader = new QHBoxLayout();
    layout->addLayout(buttonHeader);

    cueViewModeComboBox = new QComboBox();
    cueViewModeComboBox->insertItem(CuelistViewModes::cueMode, "Cue Mode");
    cueViewModeComboBox->insertItem(CuelistViewModes::groupMode, "Group Mode");
    connect(cueViewModeComboBox, &QComboBox::currentIndexChanged, this, &CuelistView::updateCuelistView);
    buttonHeader->addWidget(cueViewModeComboBox);

    trackingButton = new QPushButton("Tracking");
    trackingButton->setCheckable(true);
    trackingButton->setChecked(true);
    buttonHeader->addWidget(trackingButton);

    filterComboBox = new QComboBox();
    filterComboBox->insertItem(CuelistViewFilters::noFilter, "All Rows");
    filterComboBox->insertItem(CuelistViewFilters::activeRowsFilter, "Active Rows only");
    filterComboBox->insertItem(CuelistViewFilters::changedRowsFilter, "Changed Rows only");
    connect(filterComboBox, &QComboBox::currentIndexChanged, this, &CuelistView::updateCuelistView);
    buttonHeader->addWidget(filterComboBox);

    cueModel = new CueModel(kernel);
    groupModel = new GroupModel(kernel);

    cuelistTableView = new QTableView();
    cuelistTableView->setSelectionMode(QAbstractItemView::NoSelection);
    cuelistTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cuelistTableView->setFocusPolicy(Qt::NoFocus);
    cuelistTableView->verticalHeader()->hide();
    cuelistTableView->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(cuelistTableView);

    layout->addWidget(kernel->dmxEngine);
}

void CuelistView::reload() {
    cueModel->loadCue();
    groupModel->loadGroup();
    kernel->inspector->load(kernel->terminal->command);
    kernel->controlPanel->reload();
    kernel->playbackView->reset();
    cueLabel->setText(QString());
    cuelistLabel->setText("No Cuelist selected.");
    if (currentCuelist != nullptr) {
        cuelistLabel->setText("Cuelist " + currentCuelist->name());
    }
    cueLabel->setText("No Cue selected.");
    if ((currentCuelist != nullptr) && (currentCuelist->currentCue != nullptr)) {
        cueLabel->setText("Cue " + currentCuelist->currentCue->name());
    }
    groupLabel->setText("No Group selected.");
    if (currentGroup != nullptr) {
        groupLabel->setText("Group " + currentGroup->name());
    }
    fixtureLabel->setText("No Fixture selected.");
    if (currentFixture != nullptr) {
        fixtureLabel->setText("Fixture " + currentFixture->name());
    }
    if ((currentCuelist == nullptr) && !kernel->cuelists->items.isEmpty()) {
        currentCuelist = kernel->cuelists->items.first();
        reload();
        return;
    }
    if ((currentCuelist != nullptr) && (currentCuelist->currentCue == nullptr) && !currentCuelist->cues->items.isEmpty()) {
        loadCue(currentCuelist->cues->items.first()->id);
        return;
    }
    if ((currentGroup == nullptr) && !kernel->groups->items.isEmpty()) {
        loadGroup(kernel->groups->items.first()->id);
        return;
    }
}

void CuelistView::loadCuelist(QString cuelistId) {
    Cuelist* cuelist = kernel->cuelists->getItem(cuelistId);
    if (cuelist == nullptr) {
        kernel->terminal->error("Can't select Cuelist because Cuelist " + cuelistId + " doesn't exist.");
        return;
    }
    currentCuelist = cuelist;
    reload();
}

void CuelistView::loadCue(QString cueId) {
    if (currentCuelist == nullptr) {
        kernel->terminal->error("Can't select Cue because no Cuelist is currently selected.");
        return;
    }
    Cue* cue = currentCuelist->cues->getItem(cueId);
    if (cue == nullptr) {
        kernel->terminal->error("Can't select Cue because Cue " + cueId + " doesn't exist.");
        return;
    }
    if (cue == currentCuelist->currentCue) {
        return;
    }
    currentCuelist->lastCue = currentCuelist->currentCue;
    currentCuelist->currentCue = cue;
    currentCuelist->totalFadeFrames = kernel->dmxEngine->PROCESSINGRATE * cue->floatAttributes[kernel->CUEFADEATTRIBUTEID] + 0.5;
    currentCuelist->remainingFadeFrames = currentCuelist->totalFadeFrames;
    reload();
    if ((cueViewModeComboBox->currentIndex() == CuelistViewModes::groupMode) && groupModel->getCueRows().contains(currentCuelist->currentCue)) {
        cuelistTableView->scrollTo(groupModel->index(groupModel->getCueRows().indexOf(currentCuelist->currentCue), GroupModelColumns::cue));
    }
}

void CuelistView::previousCue() {
    QMutexLocker locker(kernel->mutex);
    if ((currentCuelist == nullptr) || (currentCuelist->currentCue == nullptr)) {
        reload();
        return;
    }
    if (currentCuelist->cues->items.indexOf(currentCuelist->currentCue) > 0) {
        loadCue(currentCuelist->cues->items[currentCuelist->cues->items.indexOf(currentCuelist->currentCue) - 1]->id);
    }
}

void CuelistView::nextCue() {
    QMutexLocker locker(kernel->mutex);
    if ((currentCuelist == nullptr) || (currentCuelist->currentCue == nullptr)) {
        reload();
        return;
    }
    if ((currentCuelist->cues->items.indexOf(currentCuelist->currentCue) + 1) < currentCuelist->cues->items.length()) {
        loadCue(currentCuelist->cues->items[currentCuelist->cues->items.indexOf(currentCuelist->currentCue) + 1]->id);
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
    reload();
    if ((cueViewModeComboBox->currentIndex() == CuelistViewModes::cueMode) && cueModel->getGroupRows().contains(currentGroup)) {
        cuelistTableView->scrollTo(cueModel->index(cueModel->getGroupRows().indexOf(currentGroup), CueModelColumns::group));
    }
}

void CuelistView::previousGroup() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        reload();
        return;
    }
    if (kernel->groups->items.indexOf(currentGroup) > 0) {
        loadGroup(kernel->groups->items[kernel->groups->items.indexOf(currentGroup) - 1]->id);
    }
}

void CuelistView::nextGroup() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        reload();
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
            reload();
        }
        return;
    }
    if (currentGroup->fixtures.indexOf(currentFixture) > 0) {
        currentFixture = currentGroup->fixtures[currentGroup->fixtures.indexOf(currentFixture) - 1];
        reload();
    }
}

void CuelistView::noFixture() {
    QMutexLocker locker(kernel->mutex);
    currentFixture = nullptr;
    reload();
}

void CuelistView::nextFixture() {
    QMutexLocker locker(kernel->mutex);
    if (currentGroup == nullptr) {
        return;
    }
    if (currentFixture == nullptr) {
        if (!currentGroup->fixtures.isEmpty()) {
            currentFixture = currentGroup->fixtures.first();
            reload();
        }
        return;
    }
    if ((currentGroup->fixtures.indexOf(currentFixture) + 1) < currentGroup->fixtures.length()) {
        currentFixture = currentGroup->fixtures[currentGroup->fixtures.indexOf(currentFixture) + 1];
        reload();
    }
}

bool CuelistView::isSelected(Fixture* fixture) {
    if (currentFixture == fixture) {
        return true;
    }
    if ((currentGroup != nullptr) && (currentFixture == nullptr) && (currentGroup->fixtures.contains(fixture))) {
        return true;
    }
    return false;
}

void CuelistView::updateCuelistView() {
    if (cueViewModeComboBox->currentIndex() == CuelistViewModes::cueMode) {
        cuelistTableView->setModel(cueModel);
        if ((currentGroup != nullptr) && cueModel->getGroupRows().contains(currentGroup)) {
            cuelistTableView->scrollTo(cueModel->index(cueModel->getGroupRows().indexOf(currentGroup), CueModelColumns::group));
        }
    } else if (cueViewModeComboBox->currentIndex() == CuelistViewModes::groupMode) {
        cuelistTableView->setModel(groupModel);
        if ((currentCuelist != nullptr) && (currentCuelist->currentCue != nullptr) && groupModel->getCueRows().contains(currentCuelist->currentCue)) {
            cuelistTableView->scrollTo(groupModel->index(groupModel->getCueRows().indexOf(currentCuelist->currentCue), GroupModelColumns::cue));
        }
    } else {
        Q_ASSERT(false);
    }
    reload();
}
