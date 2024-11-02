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

    cueModel = new CueModel(kernel);
    cueView = new QTableView();
    cueView->setSelectionMode(QAbstractItemView::NoSelection);
    cueView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cueView->setFocusPolicy(Qt::NoFocus);
    cueView->verticalHeader()->hide();
    cueView->setModel(cueModel);
    layout->addWidget(cueView);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *previousCue = new QPushButton("GO BACK");
    connect(previousCue, &QPushButton::pressed, this, &CuelistView::previousCue);
    buttons->addWidget(previousCue);
    QPushButton *nextCue = new QPushButton("GO");
    connect(nextCue, &QPushButton::pressed, this, &CuelistView::nextCue);
    buttons->addWidget(nextCue);
    layout->addLayout(buttons);

    engine = new DmxEngine(kernel);
    layout->addWidget(engine);
}


void CuelistView::loadCue() {
    cueModel->loadCue(currentCue);
    if (currentCue == nullptr) {
        cueLabel->setText(QString());
        if (!kernel->cues->getIds().isEmpty()) {
            currentCue = kernel->cues->getItem(kernel->cues->getIds()[0]); // select first cue
            loadCue();
        }
        return;
    }
    cueLabel->setText(currentCue->id);
}

void CuelistView::previousCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        if (kernel->cues->getIds().size() > 0) {
            QString firstCue = kernel->cues->getIds().at(0);
            currentCue = kernel->cues->getItem(firstCue);
            loadCue();
        }
    }
    QString previousCue;
    for (QString cueId : kernel->cues->getIds()) {
        if (cueId == currentCue->id && !previousCue.isEmpty()) {
            currentCue = kernel->cues->getItem(previousCue);
            loadCue();
            return;
        }
        previousCue = cueId;
    }
}

void CuelistView::nextCue() {
    QMutexLocker locker(kernel->mutex);
    if (currentCue == nullptr) {
        if (kernel->cues->getIds().size() > 0) {
            QString firstCue = kernel->cues->getIds().at(0);
            currentCue = kernel->cues->getItem(firstCue);
            loadCue();
        }
    }
    bool nextCue = false;
    for (QString cueId : kernel->cues->getIds()) {
        if (nextCue) {
            currentCue = kernel->cues->getItem(cueId);
            loadCue();
            return;
        }
        if (cueId == currentCue->id) {
            nextCue = true;
        }
    }
}
