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

    blindButton = new QPushButton("Blind");
    blindButton->setCheckable(true);
    connect(blindButton, &QPushButton::clicked, this, [this] { kernel->cuelistView->reload(); });
    layout->addWidget(blindButton);

    highlightButton = new QPushButton("Highlight");
    highlightButton->setCheckable(true);
    layout->addWidget(highlightButton);

    soloButton = new QPushButton("Solo");
    soloButton->setCheckable(true);
    layout->addWidget(soloButton);

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

    QMap<Group*, QMap<Effect*, int>> oldGroupEffectFrames = groupEffectFrames;
    groupEffectFrames.clear();

    QMap<Fixture*, float> fixtureDimmer;
    QMap<Fixture*, rgbColor> fixtureColor;
    QMap<Fixture*, int> fixtureColorPriority;
    QMap<Fixture*, positionAngles> fixturePosition;
    QMap<Fixture*, int> fixturePositionPriority;
    QMap<Fixture*, QMap<int, uint8_t>> fixtureRaws;
    QMap<Fixture*, QMap<int, int>> fixtureRawsPriority;

    if (kernel->cuelistView->currentCuelist == nullptr) {
        fadeProgress->setRange(0, 1);
        fadeProgress->setValue(0);
    }

    for (Cuelist* cuelist : kernel->cuelists->items) {
        if (cuelist == kernel->cuelistView->currentCuelist) {
            if ((cuelist->remainingFadeFrames > 0) && !blindButton->isChecked()) {
                if (skipFadeButton->isChecked()) {
                    cuelist->remainingFadeFrames = 0;
                }
                fadeProgress->setRange(0, cuelist->totalFadeFrames);
                fadeProgress->setValue(cuelist->totalFadeFrames - cuelist->remainingFadeFrames);
            } else {
                fadeProgress->setRange(0, 1);
                fadeProgress->setValue(1);
            }
        }
        if (cuelist->remainingFadeFrames > 0) {
            cuelist->remainingFadeFrames--;
        }
        if (cuelist->currentCue != nullptr) {
            QMap<Fixture*, float> currentCueFixtureDimmer;
            QMap<Fixture*, rgbColor> currentCueFixtureColor;
            QMap<Fixture*, positionAngles> currentCueFixturePosition;
            QMap<Fixture*, QMap<int, uint8_t>> currentCueFixtureRaws;
            QMap<Fixture*, QMap<int, bool>> currentCueFixtureRawsFade;
            renderCue(cuelist->currentCue, oldGroupEffectFrames, &currentCueFixtureDimmer, &currentCueFixtureColor, &currentCueFixturePosition, &currentCueFixtureRaws, &currentCueFixtureRawsFade);

            QMap<Fixture*, float> lastCueFixtureDimmer;
            QMap<Fixture*, rgbColor> lastCueFixtureColor;
            QMap<Fixture*, positionAngles> lastCueFixturePosition;
            QMap<Fixture*, QMap<int, uint8_t>> lastCueFixtureRaws;
            QMap<Fixture*, QMap<int, bool>> lastCueFixtureRawsFade;

            Q_ASSERT(cuelist->remainingFadeFrames <= cuelist->totalFadeFrames);
            const float fade = (float)cuelist->remainingFadeFrames / (float)cuelist->totalFadeFrames;
            if ((cuelist->previousCue != nullptr) && (cuelist->remainingFadeFrames > 0)) {
                renderCue(cuelist->previousCue, oldGroupEffectFrames, &lastCueFixtureDimmer, &lastCueFixtureColor, &lastCueFixturePosition, &lastCueFixtureRaws, &lastCueFixtureRawsFade);
            }

            for (Fixture* fixture : kernel->fixtures->items) {
                float dimmer = currentCueFixtureDimmer.value(fixture, 0);
                rgbColor color = currentCueFixtureColor.value(fixture);
                positionAngles position = currentCueFixturePosition.value(fixture, {});
                QMap<int, uint8_t> raws = currentCueFixtureRaws.value(fixture, QMap<int, uint8_t>());;

                if (cuelist->remainingFadeFrames > 0) {
                    float lastCueDimmer = lastCueFixtureDimmer.value(fixture, 0);
                    dimmer += (lastCueDimmer - dimmer) * fade;

                    rgbColor lastCueColor = {};
                    if (lastCueFixtureColor.contains(fixture)) {
                        lastCueColor = lastCueFixtureColor.value(fixture);
                    } else if (!lastCueFixtureDimmer.contains(fixture)) {
                        lastCueColor = color;
                    }
                    if (!currentCueFixtureDimmer.contains(fixture)) {
                        color = lastCueColor;
                    }
                    color.red += (lastCueColor.red - color.red) * fade;
                    color.green += (lastCueColor.green - color.green) * fade;
                    color.blue += (lastCueColor.blue - color.blue) * fade;
                    color.quality += (lastCueColor.quality - color.quality) * fade;

                    positionAngles lastCuePosition = {};
                    if (lastCueFixturePosition.contains(fixture)) {
                        lastCuePosition = lastCueFixturePosition.value(fixture);
                    } else if (!lastCueFixtureDimmer.contains(fixture)) {
                        lastCuePosition = position;
                    }
                    if (!currentCueFixtureDimmer.contains(fixture)) {
                        position = lastCuePosition;
                    }
                    if (std::abs(lastCuePosition.pan - position.pan) > 180) {
                        if (lastCuePosition.pan > position.pan) {
                            position.pan += 360;
                        } else if (lastCuePosition.pan < position.pan) {
                            lastCuePosition.pan += 360;
                        }
                    }
                    position.pan += (lastCuePosition.pan - position.pan) * fade;
                    while (position.pan >= 360) {
                        position.pan -= 360;
                    }
                    position.tilt += (lastCuePosition.tilt - position.tilt) * fade;
                    position.zoom += (lastCuePosition.zoom - position.zoom) * fade;

                    QList<int> rawChannels;
                    rawChannels.append(currentCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).keys());
                    rawChannels.append(lastCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).keys());
                    for (int channel : rawChannels) {
                        uint8_t value = 0;
                        if (currentCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel)) {
                            if (lastCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel)) {
                                value = lastCueFixtureRaws.value(fixture).value(channel);
                            }
                            if (currentCueFixtureRawsFade.value(fixture).value(channel)) {
                                value = currentCueFixtureRaws.value(fixture).value(channel) + (value - currentCueFixtureRaws.value(fixture).value(channel)) * fade;
                            } else {
                                value = currentCueFixtureRaws.value(fixture).value(channel);
                            }
                        } else if (!currentCueFixtureDimmer.contains(fixture) && lastCueFixtureDimmer.contains(fixture) && lastCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel)) {
                            value = lastCueFixtureRaws.value(fixture).value(channel);
                        } else if (lastCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel) && lastCueFixtureRawsFade.value(fixture, QMap<int, bool>()).value(channel)) {
                            value = lastCueFixtureRaws.value(fixture).value(channel) * fade;
                        }
                        raws[channel] = value;
                    }
                }
                if (dimmer > fixtureDimmer.value(fixture, 0)) {
                    fixtureDimmer[fixture] = dimmer;
                }

                if ((cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID) >= fixtureColorPriority.value(fixture, 0)) && (currentCueFixtureColor.contains(fixture) || ((cuelist->remainingFadeFrames > 0) && lastCueFixtureColor.contains(fixture)))) {
                    fixtureColor[fixture] = color;
                    fixtureColorPriority[fixture] = cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID);
                }
                if ((cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID) >= fixturePositionPriority.value(fixture, 0)) && (currentCueFixturePosition.contains(fixture) || ((cuelist->remainingFadeFrames > 0) && lastCueFixturePosition.contains(fixture)))) {
                    fixturePosition[fixture] = position;
                    fixturePositionPriority[fixture] = cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID);
                }
                for (int channel : raws.keys()) {
                    if ((cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID) >= fixtureRawsPriority.value(fixture, QMap<int, int>()).value(channel, 0)) && (currentCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel) || ((cuelist->remainingFadeFrames > 0) && lastCueFixtureRaws.value(fixture, QMap<int, uint8_t>()).contains(channel)))) {
                        if (!fixtureRaws.contains(fixture)) {
                            fixtureRaws[fixture] = QMap<int, uint8_t>();
                            fixtureRawsPriority[fixture] = QMap<int, int>();
                        }
                        fixtureRaws[fixture][channel] = raws.value(channel);
                        fixtureRawsPriority[fixture][channel] = cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID);
                    }
                }
            }
        }
    }

    for (Fixture* fixture : kernel->fixtures->items) {
        if (!fixtureDimmer.contains(fixture) && !fixtureColor.contains(fixture) && !fixturePosition.contains(fixture) && !fixtureRaws.contains(fixture)) {
            int cueDifference = -1;
            int cuelistPriority = 0;
            for (Cuelist* cuelist : kernel->cuelists->items) {
                if ((cuelist->currentCue != nullptr) && cuelist->boolAttributes.value(kernel->CUELISTMOVEWHILEDARKATTRIBUTEID)) {
                    int cueRow = cuelist->cues->getItemRow(cuelist->currentCue->id);
                    rgbColor color = {};
                    positionAngles position = {};
                    bool fixtureInformation = false;
                    QMap<int, uint8_t> rawChannels;
                    while ((cueRow < cuelist->cues->items.length()) && !fixtureInformation) {
                        Cue* cue = cuelist->cues->items[cueRow];
                        for (Group* group : kernel->groups->items) {
                            if (group->fixtures.contains(fixture)) {
                                QList<Raw*> raws;

                                if (cue->intensities.contains(group)) {
                                    fixtureInformation = true;
                                    raws.append(cue->intensities.value(group)->rawListAttributes.value(kernel->INTENSITYRAWSATTRIBUTEID));
                                }

                                if (cue->colors.contains(group)) {
                                    fixtureInformation = true;
                                    color = cue->colors.value(group)->getRGB(fixture);
                                    raws.append(cue->colors.value(group)->rawListAttributes.value(kernel->COLORRAWSATTRIBUTEID));
                                }

                                if (cue->positions.contains(group)) {
                                    fixtureInformation = true;
                                    position = cue->positions.value(group)->getAngles(fixture);
                                    raws.append(cue->positions.value(group)->rawListAttributes.value(kernel->POSITIONRAWSATTRIBUTEID));
                                }

                                if (cue->raws.contains(group)) {
                                    raws.append(cue->raws.value(group));
                                }
                                for (Raw* raw : raws) {
                                    if (raw->boolAttributes.value(kernel->RAWMOVEWHILEDARKATTRIBUTEID)) {
                                        fixtureInformation = true;
                                        const QMap<int, uint8_t> channels = raw->getChannels(fixture);
                                        for (int channel : channels.keys()) {
                                            rawChannels[channel] = channels.value(channel);
                                        }
                                    }
                                }

                                if (cue->effects.contains(group)) {
                                    for (Effect* effect : cue->effects.value(group)) {
                                        if (!effect->intensitySteps.isEmpty()) {
                                            fixtureInformation = true;
                                        }

                                        if (!effect->colorSteps.isEmpty()) {
                                            fixtureInformation = true;
                                            color = effect->getRGB(fixture, 0);
                                        }

                                        if (!effect->positionSteps.isEmpty()) {
                                            fixtureInformation = true;
                                            position = effect->getPosition(fixture, 0);
                                        }

                                        if (!effect->getRaws(fixture, 0, false).isEmpty()) {
                                            fixtureInformation = true;
                                            const QMap<int, uint8_t> channels = effect->getRaws(fixture, 0, false);
                                            for (int channel : channels.keys()) {
                                                rawChannels[channel] = channels.value(channel);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        cueRow++;
                    }
                    if (fixtureInformation) {
                        if ((cueDifference < 0) || (cueDifference > (cueRow - cuelist->cues->getItemRow(cuelist->currentCue->id) - 1))) {
                            cueDifference = cueRow - cuelist->cues->getItemRow(cuelist->currentCue->id) - 1;
                            cuelistPriority = cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID);
                            fixtureColor[fixture] = color;
                            fixturePosition[fixture] = position;
                            fixtureRaws[fixture] = rawChannels;
                        } else if ((cueDifference == (cueRow - cuelist->cues->getItemRow(cuelist->currentCue->id) - 1)) && (cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID) > cuelistPriority)) {
                            cuelistPriority = cuelist->intAttributes.value(kernel->CUELISTPRIORITYATTRIBUTEID);
                            fixtureColor[fixture] = color;
                            fixturePosition[fixture] = position;
                            fixtureRaws[fixture] = rawChannels;
                        }
                    }
                }
            }
        }
    }

    QMap<int, QByteArray> dmxUniverses;

    QMap<Fixture*, float> lastFrameFixturePan = fixturePan;
    fixturePan.clear();

    for (Fixture* fixture : kernel->fixtures->items) {
        float dimmer = fixtureDimmer.value(fixture, 0);
        rgbColor color = fixtureColor.value(fixture, {});;
        positionAngles position = fixturePosition.value(fixture, {});

        if (highlightButton->isChecked() && kernel->cuelistView->isSelected(fixture)) { // Highlight
            dimmer = 100;
            color = {100, 100, 100, 0};
        }
        if (soloButton->isChecked() && !kernel->cuelistView->isSelected(fixture)) { // Solo
            dimmer = 0;
        }

        kernel->preview2d->fixtureCircles.value(fixture)->red = (color.red / 100 * dimmer / 100 * 255);
        kernel->preview2d->fixtureCircles.value(fixture)->green = (color.green / 100 * dimmer / 100 * 255);
        kernel->preview2d->fixtureCircles.value(fixture)->blue = (color.blue / 100 * dimmer / 100 * 255);
        kernel->preview2d->fixtureCircles.value(fixture)->pan = position.pan;
        kernel->preview2d->fixtureCircles.value(fixture)->tilt = position.tilt;
        kernel->preview2d->fixtureCircles.value(fixture)->zoom = position.zoom;

        const int address = fixture->intAttributes.value(kernel->FIXTUREADDRESSATTRIBUTEID);
        if ((address > 0) && (fixture->model != nullptr)) {
            const QString channels = fixture->model->stringAttributes.value(kernel->MODELCHANNELSATTRIBUTEID);
            const int universe = fixture->intAttributes.value(kernel->FIXTUREUNIVERSEATTRIBUTEID);
            if (!dmxUniverses.contains(universe)) {
                dmxUniverses[universe] = QByteArray(512, 0);
            }

            if (!channels.contains('D')) {
                color.red *= (dimmer / 100);
                color.green *= (dimmer / 100);
                color.blue *= (dimmer / 100);
            }

            const float white = std::min(std::min(color.red, color.green), color.blue);
            if (channels.contains('W')) {
                color.red -= white * (color.quality / 100);
                color.green -= white * (color.quality / 100);
                color.blue -= white * (color.quality / 100);
            }

            if (!fixture->boolAttributes.value(kernel->FIXTUREINVERTPANATTRIBUTEID)) {
                position.pan = fixture->angleAttributes.value(kernel->FIXTUREROTATIONATTRIBUTEID) + position.pan;
            } else {
                position.pan = fixture->angleAttributes.value(kernel->FIXTUREROTATIONATTRIBUTEID) - position.pan;
            }
            while (position.pan >= 360) {
                position.pan -= 360;
            }
            while (position.pan < 0) {
                position.pan += 360;
            }
            if (!lastFrameFixturePan.contains(fixture)) {
                lastFrameFixturePan[fixture] = 0;
            }
            float pan = position.pan / fixture->model->floatAttributes.value(kernel->MODELPANRANGEATTRIBUTEID) * 100;
            pan = std::min<float>(pan, 100);
            for (float angle = position.pan; angle <= fixture->model->floatAttributes.value(kernel->MODELPANRANGEATTRIBUTEID); angle += 360) {
                if (std::abs(lastFrameFixturePan.value(fixture) - (angle / fixture->model->floatAttributes.value(kernel->MODELPANRANGEATTRIBUTEID) * 100)) < std::abs(lastFrameFixturePan.value(fixture) - pan)) {
                    pan = angle / fixture->model->floatAttributes.value(kernel->MODELPANRANGEATTRIBUTEID) * 100;
                }
            }
            fixturePan[fixture] = pan;
            float tilt = 50 + (position.tilt / (fixture->model->floatAttributes.value(kernel->MODELTILTRANGEATTRIBUTEID) / 2) * 50);
            tilt = std::min<float>(tilt, 100);
            tilt = std::max<float>(tilt, 0);
            float zoom = (position.zoom - fixture->model->floatAttributes.value(kernel->MODELMINZOOMATTRIBUTEID)) / (fixture->model->floatAttributes.value(kernel->MODELMAXZOOMATTRIBUTEID) - fixture->model->floatAttributes.value(kernel->MODELMINZOOMATTRIBUTEID)) * 100;
            zoom = std::min<float>(zoom, 100);
            zoom = std::max<float>(zoom, 0);

            for (int channel = fixture->intAttributes.value(kernel->FIXTUREADDRESSATTRIBUTEID); channel < (address + channels.size()); channel++) {
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
                    (channelType == QChar('t')) ||
                    (channelType == QChar('z'))
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
                    value = white;
                } else if (channelType == QChar('C')) { // Cyan
                    value = (100 - color.red);
                } else if (channelType == QChar('M')) { // Magenta
                    value = (100 - color.green);
                } else if (channelType == QChar('Y')) { // Yellow
                    value = (100 - color.blue);
                } else if (channelType == QChar('P')) { // Pan
                    value = pan;
                } else if (channelType == QChar('T')) { // Tilt
                    value = tilt;
                } else if (channelType == QChar('Z')) { // Zoom
                    value = zoom;
                } else if (channelType == QChar('0')) { // DMX 0
                    value = 0;
                } else if (channelType == QChar('1')) { // DMX 255
                    value = 100;
                } else {
                    Q_ASSERT(false);
                }
                Q_ASSERT((value <= 100) && (value >= 0));

                if (channel <= 512) {
                    value *= 655.35;
                    if (fine) {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value % 256);
                    } else {
                        dmxUniverses[universe][channel - 1] = (uchar)((int)value / 256);
                    }
                }
            }

            for (int channel : fixtureRaws.value(fixture, QMap<int, uint8_t>()).keys()) {
                if ((channel <= channels.size()) && ((address + channel - 1) <= 512)) {
                    dmxUniverses[universe][address + channel - 2] = fixtureRaws.value(fixture).value(channel);
                }
            }
        }
    }
    sacnServer->send(dmxUniverses);
    kernel->preview2d->updateImage();
}


void DmxEngine::renderCue(Cue* cue, QMap<Group*, QMap<Effect*, int>> oldGroupEffectFrames, QMap<Fixture*, float>* fixtureDimmers, QMap<Fixture*, rgbColor>* fixtureColors, QMap<Fixture*, positionAngles>* fixturePositions, QMap<Fixture*, QMap<int, uint8_t>>* fixtureRaws, QMap<Fixture*, QMap<int, bool>>* fixtureRawFade) {
    Q_ASSERT(cue != nullptr);
    for (Group* group : kernel->groups->items) {
        QList<Raw*> raws = QList<Raw*>();
        if (cue->intensities.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                float dimmer = cue->intensities.value(group)->getDimmer(fixture);
                if (dimmer > fixtureDimmers->value(fixture, 0)) {
                    (*fixtureDimmers)[fixture] = dimmer;
                }
            }
            raws.append(cue->intensities.value(group)->rawListAttributes.value(kernel->INTENSITYRAWSATTRIBUTEID));
        }

        if (cue->colors.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                (*fixtureColors)[fixture] = cue->colors.value(group)->getRGB(fixture);
            }
            raws.append(cue->colors.value(group)->rawListAttributes.value(kernel->COLORRAWSATTRIBUTEID));
        }

        if (cue->positions.contains(group)) {
            for (Fixture* fixture : group->fixtures) {
                (*fixturePositions)[fixture] = cue->positions.value(group)->getAngles(fixture);
            }
            raws.append(cue->positions.value(group)->rawListAttributes.value(kernel->POSITIONRAWSATTRIBUTEID));
        }

        if (cue->raws.contains(group)) {
            raws.append(cue->raws.value(group));
        }
        for (Raw* raw : raws) {
            for (Fixture* fixture : group->fixtures) {
                if (!fixtureRaws->contains(fixture)) {
                    (*fixtureRaws)[fixture] = QMap<int, uint8_t>();
                    (*fixtureRawFade)[fixture] = QMap<int, bool>();
                }
                const QMap<int, uint8_t> channels = raw->getChannels(fixture);
                for (int channel : channels.keys()) {
                    (*fixtureRaws)[fixture][channel] = channels.value(channel);
                    (*fixtureRawFade)[fixture][channel] = raw->boolAttributes.value(kernel->RAWFADEATTRIBUTEID);
                }
            }
        }

        if (cue->effects.contains(group)) {
            for (Effect* effect : cue->effects.value(group)) {
                groupEffectFrames[group][effect] = 0;
                if (oldGroupEffectFrames.contains(group) && oldGroupEffectFrames.value(group).contains(effect)) {
                    groupEffectFrames[group][effect] = (oldGroupEffectFrames.value(group).value(effect) + 1);
                }

                for (Fixture* fixture : group->fixtures) {
                    if (!effect->intensitySteps.isEmpty()) {
                        float dimmer = effect->getDimmer(fixture, groupEffectFrames[group][effect]);
                        if (dimmer > fixtureDimmers->value(fixture, 0)) {
                            (*fixtureDimmers)[fixture] = dimmer;
                        }
                    }

                    if (!effect->colorSteps.isEmpty()) {
                        (*fixtureColors)[fixture] = effect->getRGB(fixture, groupEffectFrames[group][effect]);
                    }

                    if (!effect->positionSteps.isEmpty()) {
                        (*fixturePositions)[fixture] = effect->getPosition(fixture, groupEffectFrames[group][effect]);
                    }

                    if (!effect->getRaws(fixture, groupEffectFrames[group][effect]).isEmpty()) {
                        if (!fixtureRaws->contains(fixture)) {
                            (*fixtureRaws)[fixture] = QMap<int, uint8_t>();
                            (*fixtureRawFade)[fixture] = QMap<int, bool>();
                        }
                        const QMap<int, uint8_t> channels = effect->getRaws(fixture, groupEffectFrames[group][effect]);
                        for (int channel : channels.keys()) {
                            (*fixtureRaws)[fixture][channel] = channels.value(channel);
                            (*fixtureRawFade)[fixture][channel] = false;
                        }
                    }
                }
            }
        }
    }
}
