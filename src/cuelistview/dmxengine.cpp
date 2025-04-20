/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"
#include "kernel/kernel.h"

DmxEngine::DmxEngine(Kernel *core, QWidget* parent) : QWidget(parent) {
    kernel = core;
    sacnServer = new SacnServer(kernel);

    QHBoxLayout *layout = new QHBoxLayout(this);

    highlightButton = new QPushButton("Highlight");
    highlightButton->setCheckable(true);
    connect(highlightButton,&QPushButton::clicked, this, &DmxEngine::generateDmx);
    layout->addWidget(highlightButton);

    fadeProgress = new QProgressBar();
    fadeProgress->setRange(0, 1);
    fadeProgress->setValue(1);
    layout->addWidget(fadeProgress);

    skipFadeButton = new QPushButton("Skip Fade");
    skipFadeButton->setCheckable(true);
    layout->addWidget(skipFadeButton);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &DmxEngine::generateDmx);
    timer->start(25);
}

void DmxEngine::generateDmx() {
    QMutexLocker(kernel->mutex);
    QMap<Fixture*, float> currentCueFixtureDimmer;
    QMap<Fixture*, rgbColor> currentCueFixtureColor;
    QMap<Fixture*, positionAngles> currentCueFixturePosition;
    QMap<Fixture*, QList<Raw*>> currentCueFixtureRaws;
    QMap<Fixture*, float> lastCueFixtureDimmer;
    QMap<Fixture*, rgbColor> lastCueFixtureColor;
    QMap<Fixture*, positionAngles> lastCueFixturePosition;
    QMap<Fixture*, QList<Raw*>> lastCueFixtureRaws;
    if (kernel->cuelistView->currentCue == nullptr) {
        currentCue = nullptr;
        remainingFadeFrames = 0;
        totalFadeFrames = 0;
    } else {
        if (kernel->cuelistView->currentCue != currentCue) {
            lastCue = nullptr;
            if (kernel->cues->items.contains(currentCue)) {
                lastCue = currentCue;
            }
            currentCue = kernel->cuelistView->currentCue;
            totalFadeFrames = PROCESSINGRATE * currentCue->floatAttributes[kernel->cues->FADEATTRIBUTEID] + 0.5;
            remainingFadeFrames = totalFadeFrames;
        }
        if (skipFadeButton->isChecked()) {
            remainingFadeFrames = 0;
        }
        QMap<Group*, QMap<Effect*, int>> newGroupEffectFrames = renderCue(currentCue, &currentCueFixtureDimmer, &currentCueFixtureColor, &currentCueFixturePosition, &currentCueFixtureRaws);
        QMap<Group*, QMap<Effect*, int>> fadeGroupEffectFrames;
        if ((lastCue != nullptr) && (remainingFadeFrames > 0)) {
            fadeGroupEffectFrames = renderCue(lastCue, &lastCueFixtureDimmer, &lastCueFixtureColor, &lastCueFixturePosition, &lastCueFixtureRaws);
        }
        groupEffectFrames.clear();
        for (Group* group : fadeGroupEffectFrames.keys()) {
            for (Effect* effect : fadeGroupEffectFrames.value(group).keys()) {
                groupEffectFrames[group][effect] = fadeGroupEffectFrames.value(group).value(effect);
            }
        }
        for (Group* group : newGroupEffectFrames.keys()) {
            for (Effect* effect : newGroupEffectFrames.value(group).keys()) {
                groupEffectFrames[group][effect] = newGroupEffectFrames.value(group).value(effect);
            }
        }
    }
    QMap<int, QByteArray> dmxUniverses;
    for (Fixture* fixture : kernel->fixtures->items) {
        float dimmer = 0;
        rgbColor color = {};
        positionAngles position = {};
        if (currentCueFixtureDimmer.contains(fixture)) {
            dimmer = currentCueFixtureDimmer.value(fixture);
        }
        if (currentCueFixtureColor.contains(fixture)) {
            color = currentCueFixtureColor.value(fixture);
        }
        if (currentCueFixturePosition.contains(fixture)) {
            position = currentCueFixturePosition.value(fixture);
        }
        if (remainingFadeFrames > 0) {
            float lastCueDimmer = 0;
            rgbColor lastCueColor = {};
            positionAngles lastCuePosition = {};
            if (lastCueFixtureDimmer.contains(fixture)) {
                lastCueDimmer = lastCueFixtureDimmer.value(fixture);
            }
            dimmer += (lastCueDimmer - dimmer) * (float)remainingFadeFrames / (float)totalFadeFrames;
            if (lastCueFixtureColor.contains(fixture)) {
                lastCueColor = lastCueFixtureColor.value(fixture);
            }
            if (lastCueFixturePosition.contains(fixture)) {
                lastCuePosition = lastCueFixturePosition.value(fixture);
            }
            if (!lastCueFixtureDimmer.contains(fixture)) {
                lastCueColor = color;
                lastCuePosition = position;
            }
            if (currentCueFixtureDimmer.contains(fixture)) {
                color.red += (lastCueColor.red - color.red) * (float)remainingFadeFrames / (float)totalFadeFrames;
                color.green += (lastCueColor.green - color.green) * (float)remainingFadeFrames / (float)totalFadeFrames;
                color.blue += (lastCueColor.blue - color.blue) * (float)remainingFadeFrames / (float)totalFadeFrames;
                position.pan += (lastCuePosition.pan - position.pan) * (float)remainingFadeFrames / (float)totalFadeFrames;
                position.tilt += (lastCuePosition.tilt - position.tilt) * (float)remainingFadeFrames / (float)totalFadeFrames;
            } else {
                color = lastCueColor;
                position = lastCuePosition;
            }
        }
        if (highlightButton->isChecked() && (kernel->cuelistView->currentGroup != nullptr) && (((kernel->cuelistView->currentFixture == nullptr) && (kernel->cuelistView->currentGroup->fixtures.contains(fixture))) || (kernel->cuelistView->currentFixture == fixture))) { // Highlight
            dimmer = 100;
            color = {100, 100, 100};
        }
        kernel->cuelistView->preview2d->fixtureCircles[fixture]->setBrush(QBrush(QColor((color.red / 100 * dimmer / 100 * 255), (color.green / 100 * dimmer / 100 * 255), (color.blue / 100 * dimmer / 100 * 255))));
        const int address = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID);
        if ((address > 0) && (fixture->model != nullptr)) {
            const QString channels = fixture->model->stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID);
            const int universe = fixture->intAttributes.value(kernel->fixtures->UNIVERSEATTRIBUTEID);
            if (!dmxUniverses.contains(universe)) {
                dmxUniverses[universe] = QByteArray(512, 0);
            }
            if (!channels.contains('D')) {
                color.red *= (dimmer / 100);
                color.green *= (dimmer / 100);
                color.blue *= (dimmer / 100);
            }
            for (int channel = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID); channel < (address + channels.size()); channel++) {
                float value = 0;
                QChar channelType = channels.at(channel - address);
                bool fine = (
                    (channelType == QChar('d')) ||
                    (channelType == QChar('r')) ||
                    (channelType == QChar('g')) ||
                    (channelType == QChar('b')) ||
                    (channelType == QChar('w')) ||
                    (channelType == QChar('c')) ||
                    (channelType == QChar('m')) ||
                    (channelType == QChar('y')) ||
                    (channelType == QChar('p')) ||
                    (channelType == QChar('t'))
                );
                if (fine) {
                    channelType = channelType.toUpper();
                }
                if (channelType == QChar('D')) { // Dimmer
                    value = dimmer;
                } else if (channelType == QChar('R')) { // Red
                    value = color.red;
                } else if (channelType == QChar('G')) { // Green
                    value = color.green;
                } else if (channelType == QChar('B')) { // Blue
                    value = color.blue;
                } else if (channelType == QChar('W')) { // White
                    value = 0;
                } else if (channelType == QChar('C')) { // Cyan
                    value = (100 - color.red);
                } else if (channelType == QChar('M')) { // Magenta
                    value = (100 - color.green);
                } else if (channelType == QChar('Y')) { // Yellow
                    value = (100 - color.blue);
                } else if (channelType == QChar('P')) { // Pan
                    value = position.pan;
                } else if (channelType == QChar('T')) { // Tilt
                    value = position.tilt;
                } else if (channelType == QChar('0')) { // DMX 0
                    value = 0;
                } else if (channelType == QChar('1')) { // DMX 255
                    value = 100;
                } else {
                    Q_ASSERT(false);
                }
                Q_ASSERT((value <= 100) && (value >= 0));;
                if (channel <= 512) {
                    value *= 655.35;
                    if (fine) {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value % 256);
                    } else {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value / 256);
                    }
                }
            }
            if (currentCueFixtureRaws.contains(fixture)) {
                for (Raw* raw : currentCueFixtureRaws.value(fixture)) {
                    for (int channel : raw->channelValues.keys()) {
                        if (((address + channel - 1) <= 512) && (channel <= channels.size())) {
                            dmxUniverses[universe][address + channel - 1] = raw->channelValues.value(channel);
                        }
                    }
                    if (raw->modelSpecificChannelValues.contains(fixture->model)) {
                        for (int channel : raw->modelSpecificChannelValues.value(fixture->model).keys()) {
                            if (((address + channel - 1) <= 512) && (channel <= channels.size())) {
                                dmxUniverses[universe][address + channel - 1] = raw->modelSpecificChannelValues.value(fixture->model).value(channel);
                            }
                        }
                    }
                    if (raw->fixtureSpecificChannelValues.contains(fixture)) {
                        for (int channel : raw->fixtureSpecificChannelValues.value(fixture).keys()) {
                            if (((address + channel - 1) <= 512) && (channel <= channels.size())) {
                                dmxUniverses[universe][address + channel - 1] = raw->fixtureSpecificChannelValues.value(fixture).value(channel);
                            }
                        }
                    }
                }
            }
        }
    }
    sacnServer->send(dmxUniverses);
    if (remainingFadeFrames > 0) {
        remainingFadeFrames--;
        fadeProgress->setValue(totalFadeFrames - remainingFadeFrames);
        fadeProgress->setRange(0, totalFadeFrames);
    } else {
        fadeProgress->setValue(1);
        fadeProgress->setRange(0, 1);
    }
    kernel->cuelistView->preview2d->updateImage();
}

QMap<Group*, QMap<Effect*, int>> DmxEngine::renderCue(Cue* cue, QMap<Fixture*, float>* fixtureDimmers, QMap<Fixture*, rgbColor>* fixtureColors, QMap<Fixture*, positionAngles>* fixturePositions, QMap<Fixture*, QList<Raw*>>* fixtureRaws) {
    Q_ASSERT(cue != nullptr);
    QMap<Group*, QMap<Effect*, int>> newGroupEffectFrames;
    for (Group* group : kernel->groups->items) {
        if (cue->intensities.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                (*fixtureDimmers)[fixture] = cue->intensities.value(group)->getDimmer(fixture);
            }
        }
        if (cue->colors.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                (*fixtureColors)[fixture] = cue->colors.value(group)->getRGB(fixture);
            }
        }
        if (cue->positions.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                (*fixturePositions)[fixture] = cue->positions.value(group)->getAngles(fixture);
            }
        }
        if (cue->raws.contains(group)) {
            for (Raw* raw : cue->raws[group]) {
                for (Fixture* fixture : group->fixtures) {
                    if (!(*fixtureRaws).contains(fixture)) {
                        (*fixtureRaws)[fixture] = QList<Raw*>();
                    }
                    (*fixtureRaws)[fixture].append(raw);
                }
            }
        }
        if (cue->effects.contains(group)) {
            for (Effect* effect : cue->effects.value(group)) {
                newGroupEffectFrames[group][effect] = 0;
                if (groupEffectFrames.contains(group) && groupEffectFrames.value(group).contains(effect)) {
                    newGroupEffectFrames[group][effect] = (groupEffectFrames.value(group).value(effect) + 1);
                }
                if (!effect->intensitySteps.isEmpty()) {
                    for (Fixture* fixture : group->fixtures) {
                        (*fixtureDimmers)[fixture] = effect->getDimmer(fixture, newGroupEffectFrames[group][effect]);
                    }
                }
                if (!effect->colorSteps.isEmpty()) {
                    for (Fixture* fixture : group->fixtures) {
                        (*fixtureColors)[fixture] = effect->getRGB(fixture, newGroupEffectFrames[group][effect]);
                    }
                }
                if (!effect->rawSteps.isEmpty()) {
                    for (Fixture* fixture : group->fixtures) {
                        if (!fixtureRaws->contains(fixture)) {
                            (*fixtureRaws)[fixture] = QList<Raw*>();
                        }
                        (*fixtureRaws)[fixture].append(effect->getRaws(fixture, newGroupEffectFrames[group][effect]));
                    }
                }
            }
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        if (!fixtureDimmers->contains(fixture)) {
            int cueRow = kernel->cues->getItemRow(cue->id);
            while ((cueRow < kernel->cues->items.length()) && !fixtureColors->contains(fixture)) {
                Cue* cue = kernel->cues->items[cueRow];
                for (Group* group : kernel->groups->items) {
                    if (group->fixtures.contains(fixture)) {
                        if (cue->intensities.contains(group) && !fixtureColors->contains(fixture)) {
                            (*fixtureColors)[fixture] = {};
                        }
                        if (cue->colors.contains(group)) {
                            (*fixtureColors)[fixture] = cue->colors.value(group)->getRGB(fixture);
                        }
                        if (cue->effects.contains(group)) {
                            for (Effect* effect : cue->effects.value(group)) {
                                if (!effect->intensitySteps.isEmpty() && !fixtureColors->contains(fixture)) {
                                    (*fixtureColors)[fixture] = {};
                                }
                                if (!effect->colorSteps.isEmpty()) {
                                    (*fixtureColors)[fixture] = effect->getRGB(fixture, 0);
                                }
                            }
                        }
                    }
                }
                cueRow++;
            }
            cueRow = kernel->cues->getItemRow(cue->id);
            while ((cueRow < kernel->cues->items.length()) && !fixturePositions->contains(fixture)) {
                Cue* cue = kernel->cues->items[cueRow];
                for (Group* group : kernel->groups->items) {
                    if (group->fixtures.contains(fixture)) {
                        if (cue->intensities.contains(group) && !fixturePositions->contains(fixture)) {
                            (*fixturePositions)[fixture] = {};
                        }
                        if (cue->positions.contains(group)) {
                            (*fixturePositions)[fixture] = cue->positions.value(group)->getAngles(fixture);
                        }
                        if (cue->effects.contains(group)) {
                            for (Effect* effect : cue->effects.value(group)) {
                                if (!effect->intensitySteps.isEmpty() && !fixturePositions->contains(fixture)) {
                                    (*fixturePositions)[fixture] = {};
                                }
                            }
                        }
                    }
                }
                cueRow++;
            }
            cueRow = kernel->cues->getItemRow(cue->id);
            while ((cueRow < kernel->cues->items.length()) && !fixtureRaws->contains(fixture)) {
                Cue* cue = kernel->cues->items[cueRow];
                for (Group* group : kernel->groups->items) {
                    if (group->fixtures.contains(fixture)) {
                        if (cue->intensities.contains(group) && !fixtureRaws->contains(fixture)) {
                            (*fixtureRaws)[fixture] = QList<Raw*>();
                        }
                        if (cue->raws.contains(group)) {
                            for (Raw* raw : cue->raws.value(group)) {
                                if (raw->boolAttributes.value(kernel->raws->MOVEINBLACKATTRIBUTEID)) {
                                    if (!fixtureRaws->contains(fixture)) {
                                        (*fixtureRaws)[fixture] = QList<Raw*>();
                                    }
                                    (*fixtureRaws)[fixture].append(raw);
                                }
                            }
                        }
                        if (cue->effects.contains(group)) {
                            for (Effect* effect : cue->effects.value(group)) {
                                if (!effect->intensitySteps.isEmpty() && !fixtureRaws->contains(fixture)) {
                                    (*fixtureRaws)[fixture] = QList<Raw*>();
                                }
                                if (!effect->rawSteps.isEmpty()) {
                                    if (!fixtureRaws->contains(fixture)) {
                                        (*fixtureRaws)[fixture] = QList<Raw*>();
                                    }
                                    for (Raw* raw : effect->getRaws(fixture, 0)) {
                                        if (raw->boolAttributes.value(kernel->raws->MOVEINBLACKATTRIBUTEID)) {
                                            (*fixtureRaws)[fixture].append(raw);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                cueRow++;
            }
        }
    }
    return newGroupEffectFrames;
}
