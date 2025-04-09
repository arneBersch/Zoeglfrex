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
    QMap<Fixture*, float> fixtureDimmer;
    QMap<Fixture*, rgbColor> fixtureColor;
    for (Fixture* fixture : kernel->fixtures->items) {
        fixture->raws.clear();
    }
    if (kernel->cuelistView->currentCue == nullptr) {
        remainingFadeFrames = 0;
        totalFadeFrames = 0;
    } else {
        if (kernel->cuelistView->currentCue != lastCue) {
            totalFadeFrames = PROCESSINGRATE * kernel->cuelistView->currentCue->floatAttributes[kernel->cues->FADEATTRIBUTEID] + 0.5;
            remainingFadeFrames = totalFadeFrames;
            lastCue = kernel->cuelistView->currentCue;
        }
        if (skipFadeButton->isChecked()) {
            remainingFadeFrames = 0;
        }
        QMap<Group*, QMap<Effect*, int>> oldGroupEffectFrames = groupEffectFrames;
        groupEffectFrames.clear();
        for (Group* group : kernel->groups->items) {
            if (lastCue->intensities.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureDimmer[fixture] = lastCue->intensities.value(group)->getDimmer(fixture);
                }
            }
            if (lastCue->colors.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureColor[fixture] = lastCue->colors.value(group)->getRGB(fixture);
                }
            }
            if (lastCue->raws.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    for (Raw* raw : lastCue->raws[group]) {
                        fixture->raws.append(raw);
                    }
                }
            }
            if (lastCue->effects.contains(group)) {
                for (Effect* effect : lastCue->effects.value(group)) {
                    groupEffectFrames[group][effect] = 0;
                    if (oldGroupEffectFrames.contains(group) && oldGroupEffectFrames.value(group).contains(effect)) {
                        groupEffectFrames[group][effect] = (oldGroupEffectFrames.value(group).value(effect) + 1);
                    }
                    QList<int> stepFrames = QList<int>(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID), 0);
                    QList<int> fadeFrames = QList<int>(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID), 0);
                    int totalFrames = 0;
                    for (int step = 1; step <= effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID); step++) {
                        if (effect->stepSpecificFloatAttributes.value(kernel->effects->STEPFADEATTRIBUTEID).contains(step)) {
                            fadeFrames[step - 1] = (PROCESSINGRATE * effect->stepSpecificFloatAttributes.value(kernel->effects->STEPFADEATTRIBUTEID).value(step));
                        } else {
                            fadeFrames[step - 1] = (PROCESSINGRATE * effect->floatAttributes.value(kernel->effects->STEPFADEATTRIBUTEID));
                        }
                        stepFrames[step - 1] = fadeFrames[step - 1];
                        if (effect->stepSpecificFloatAttributes.value(kernel->effects->STEPHOLDATTRIBUTEID).contains(step)) {
                            stepFrames[step - 1] += (PROCESSINGRATE * effect->stepSpecificFloatAttributes.value(kernel->effects->STEPHOLDATTRIBUTEID).value(step));
                        } else {
                            stepFrames[step - 1] += (PROCESSINGRATE * effect->floatAttributes.value(kernel->effects->STEPHOLDATTRIBUTEID));
                        }
                        totalFrames += stepFrames[step - 1];
                    }
                    if (totalFrames > 0) {
                        for (Fixture* fixture : group->fixtures) {
                            int frame = groupEffectFrames[group][effect];
                            if (effect->fixtureSpecificAngleAttributes.value(kernel->effects->PHASEATTRIBUTEID).contains(fixture)) {
                                frame += effect->fixtureSpecificAngleAttributes.value(kernel->effects->PHASEATTRIBUTEID).value(fixture) * (float)totalFrames / 360.0;
                            } else {
                                frame += effect->angleAttributes.value(kernel->effects->PHASEATTRIBUTEID) * (float)totalFrames / 360.0;
                            }
                            frame %= totalFrames;
                            int step = 1;
                            while (frame >= stepFrames[step - 1]) {
                                frame -= stepFrames[step - 1];
                                step++;
                            }
                            int stepFadeFrames = 0;
                            if (frame < fadeFrames[step - 1]) {
                                stepFadeFrames = fadeFrames[step - 1] - frame;
                            }
                            if (stepFadeFrames > 0) {
                                if (!effect->intensitySteps.isEmpty()) {
                                    float formerDimmer = 0.0;
                                    if ((step > 1) && effect->intensitySteps.contains(step - 1)) {
                                        formerDimmer = effect->intensitySteps.value(step - 1)->getDimmer(fixture);
                                    } else if ((step == 1) && effect->intensitySteps.contains(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID))) {
                                        formerDimmer = effect->intensitySteps.value(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID))->getDimmer(fixture);
                                    } else if (lastCue->intensities.contains(group)) {
                                        formerDimmer = lastCue->intensities.value(group)->getDimmer(fixture);
                                    }
                                    if (effect->intensitySteps.contains(step)) {
                                        fixtureDimmer[fixture] = formerDimmer + (effect->intensitySteps.value(step)->getDimmer(fixture) - formerDimmer) * (1- ((float)stepFadeFrames / (float)fadeFrames[step - 1]));
                                    } else {
                                        fixtureDimmer[fixture] = formerDimmer * (float)stepFadeFrames / (float)fadeFrames[step - 1];
                                    }
                                }
                                if (!effect->colorSteps.isEmpty()) {
                                    rgbColor formerColor = {100, 100, 100};
                                    if ((step > 1) && effect->colorSteps.contains(step - 1)) {
                                        formerColor = effect->colorSteps.value(step - 1)->getRGB(fixture);
                                    } else if ((step == 1) && effect->colorSteps.contains(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID))) {
                                        formerColor = effect->colorSteps.value(effect->intAttributes.value(kernel->effects->STEPSATTRIBUTEID))->getRGB(fixture);
                                    } else if (lastCue->colors.contains(group)) {
                                        formerColor = lastCue->colors.value(group)->getRGB(fixture);
                                    }
                                    fixtureColor[fixture] = {0, 0, 0};
                                    if (effect->colorSteps.contains(step)) {
                                        fixtureColor[fixture].red = formerColor.red + (effect->colorSteps.value(step)->getRGB(fixture).red - formerColor.red) * (1- ((float)stepFadeFrames / (float)fadeFrames[step - 1]));
                                        fixtureColor[fixture].green = formerColor.green + (effect->colorSteps.value(step)->getRGB(fixture).green - formerColor.green) * (1- ((float)stepFadeFrames / (float)fadeFrames[step - 1]));
                                        fixtureColor[fixture].blue = formerColor.blue + (effect->colorSteps.value(step)->getRGB(fixture).blue - formerColor.blue) * (1- ((float)stepFadeFrames / (float)fadeFrames[step - 1]));
                                    } else {
                                        fixtureColor[fixture].red = formerColor.red * (float)stepFadeFrames / (float)fadeFrames[step - 1];
                                        fixtureColor[fixture].green = formerColor.green * (float)stepFadeFrames / (float)fadeFrames[step - 1];
                                        fixtureColor[fixture].blue = formerColor.blue * (float)stepFadeFrames / (float)fadeFrames[step - 1];
                                    }
                                }
                            } else {
                                if (effect->intensitySteps.contains(step)) {
                                    fixtureDimmer[fixture] = effect->intensitySteps.value(step)->getDimmer(fixture);
                                }
                                if (effect->colorSteps.contains(step)) {
                                    fixtureColor[fixture] = effect->colorSteps.value(step)->getRGB(fixture);
                                }
                            }
                            if (effect->rawSteps.contains(step)) {
                                fixture->raws.append(effect->rawSteps.value(step));
                            }
                        }
                    }
                }
            }
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        float dimmer = 0.0;
        float red = 0.0;
        float green = 0.0;
        float blue = 0.0;
        if (fixtureDimmer.contains(fixture)) {
            dimmer = fixtureDimmer.value(fixture);
        }
        if (fixtureColor.contains(fixture)) {
            red = fixtureColor.value(fixture).red;
            green = fixtureColor.value(fixture).green;
            blue = fixtureColor.value(fixture).blue;
        } else if (fixtureDimmer.contains(fixture)) {
            red = 100.0;
            green = 100.0;
            blue = 100.0;
        }
        if (remainingFadeFrames > 0) {
            fixture->dimmer += (dimmer - fixture->dimmer) / remainingFadeFrames;
        } else {
            fixture->dimmer = dimmer;
        }
        if (remainingFadeFrames > 0) {
            fixture->red += (red - fixture->red) / remainingFadeFrames;
            fixture->green += (green - fixture->green) / remainingFadeFrames;
            fixture->blue += (blue - fixture->blue) / remainingFadeFrames;
        } else {
            fixture->red = red;
            fixture->green = green;
            fixture->blue = blue;
        }
    }
    QMap<int, QByteArray> dmxUniverses;
    for (Fixture* fixture : kernel->fixtures->items) {
        const int address = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID);
        if ((address > 0) && (fixture->model != nullptr)) {
            const QString channels = fixture->model->stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID);
            const int universe = fixture->intAttributes.value(kernel->fixtures->UNIVERSEATTRIBUTEID);
            if (!dmxUniverses.contains(universe)) {
                dmxUniverses[universe] = QByteArray(512, 0);
            }
            float dimmer = fixture->dimmer;
            float red = fixture->red;
            float green = fixture->green;
            float blue = fixture->blue;
            if (highlightButton->isChecked() && (kernel->cuelistView->currentGroup != nullptr) && (((kernel->cuelistView->currentFixture == nullptr) && (kernel->cuelistView->currentGroup->fixtures.contains(fixture))) || (kernel->cuelistView->currentFixture == fixture))) { // Highlight
                dimmer = 100.0;
                red = 100.0;
                green = 100.0;
                blue = 100.0;
            }
            if (!channels.contains('D')) {
                red *= (dimmer / 100.0);
                green *= (dimmer / 100.0);
                blue *= (dimmer / 100.0);
            }
            for (int channel = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID); channel < (address + channels.size()); channel++) {
                float value = 0.0;
                QChar channelType = channels.at(channel - address);
                bool fine = (
                    (channelType == QChar('d')) ||
                    (channelType == QChar('r')) ||
                    (channelType == QChar('g')) ||
                    (channelType == QChar('b')) ||
                    (channelType == QChar('w')) ||
                    (channelType == QChar('c')) ||
                    (channelType == QChar('m')) ||
                    (channelType == QChar('y'))
                );
                if (fine) {
                    channelType = channelType.toUpper();
                }
                if (channelType == QChar('D')) { // Dimmer
                    value = dimmer;
                } else if (channelType == QChar('R')) { // Red
                    value = red;
                } else if (channelType == QChar('G')) { // Green
                    value = green;
                } else if (channelType == QChar('B')) { // Blue
                    value = blue;
                } else if (channelType == QChar('W')) { // White
                    value = 0.0;
                } else if (channelType == QChar('C')) { // Cyan
                    value = (100.0 - red);
                } else if (channelType == QChar('M')) { // Magenta
                    value = (100.0 - green);
                } else if (channelType == QChar('Y')) { // Yellow
                    value = (100.0 - blue);
                } else if (channelType == QChar('0')) { // DMX 0
                    value = 0.0;
                } else if (channelType == QChar('1')) { // DMX 255
                    value = 100.0;
                } else {
                    Q_ASSERT(false);
                }
                if (channel <= 512) {
                    value *= 655.35;
                    if (fine) {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value % 256);
                    } else {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value / 256);
                    }
                }
            }
            for (Raw* raw : fixture->raws) {
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
    if (remainingFadeFrames > 0) {
        remainingFadeFrames--;
    }
    sacnServer->send(dmxUniverses);
    fadeProgress->setValue(totalFadeFrames + 1 - remainingFadeFrames);
    fadeProgress->setRange(0, totalFadeFrames + 1);

    kernel->cuelistView->preview2d->updateImage();
}
