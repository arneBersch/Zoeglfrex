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
    QMap<Fixture*, Intensity*> fixtureIntensities;
    QMap<Fixture*, Color*> fixtureColors;
    QMap<Fixture*, QList<Raw*>> fixtureRaws;
    QMap<Fixture*, QList<Effect*>> fixtureEffects;
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
        for (Group* group : kernel->groups->items) {
            if (lastCue->intensities.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureIntensities[fixture] = lastCue->intensities.value(group);
                }
            }
            if (lastCue->colors.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureColors[fixture] = lastCue->colors[group];
                }
            }
            if (lastCue->raws.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    if (!fixtureRaws.contains(fixture)) {
                        fixtureRaws[fixture] = QList<Raw*>();
                    }
                    for (Raw* raw : lastCue->raws[group]) {
                        fixtureRaws[fixture].append(raw);
                    }
                }
            }
            if (lastCue->effects.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    if (!fixtureEffects.contains(fixture)) {
                        fixtureEffects[fixture] = QList<Effect*>();
                    }
                    for (Effect* effect : lastCue->effects[group]) {
                        fixtureEffects[fixture].append(effect);
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
        if (fixtureIntensities.contains(fixture)) {
            dimmer = fixtureIntensities.value(fixture)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID);
            if (fixtureIntensities.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).contains(fixture)) {
                dimmer = fixtureIntensities.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(fixture);
            } else if (fixtureIntensities.value(fixture)->modelSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).contains(fixture->model)) {
                dimmer = fixtureIntensities.value(fixture)->modelSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(fixture->model);
            }
        }
        if (fixtureColors.contains(fixture)) {
            float hue = fixtureColors.value(fixture)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID);
            if (fixtureColors.value(fixture)->fixtureSpecificAngleAttributes.value(kernel->colors->HUEATTRIBUTEID).contains(fixture)) {
                hue = fixtureColors.value(fixture)->fixtureSpecificAngleAttributes.value(kernel->colors->HUEATTRIBUTEID).value(fixture);
            } else if (fixtureColors.value(fixture)->modelSpecificAngleAttributes.value(kernel->colors->HUEATTRIBUTEID).contains(fixture->model)) {
                hue = fixtureColors.value(fixture)->modelSpecificAngleAttributes.value(kernel->colors->HUEATTRIBUTEID).value(fixture->model);
            }
            const double h = (hue / 60.0);
            const int i = (int)h;
            const double f = h - i;
            float saturation = fixtureColors.value(fixture)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID);
            if (fixtureColors.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).contains(fixture)) {
                saturation = fixtureColors.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).value(fixture);
            } else if (fixtureColors.value(fixture)->modelSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).contains(fixture->model)) {
                saturation = fixtureColors.value(fixture)->modelSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).value(fixture->model);
            }
            const double p = (100.0 - saturation);
            const double q = (100.0 - (saturation * f));
            const double t = (100.0 - (saturation * (1.0 - f)));
            if (i == 0) {
                red = 100.0;
                green = t;
                blue = p;
            } else if (i == 1) {
                red = q;
                green = 100.0;
                blue = p;
            } else if (i == 2) {
                red = p;
                green = 100.0;
                blue = t;
            } else if (i == 3) {
                red = p;
                green = q;
                blue = 100.0;
            } else if (i == 4) {
                red = t;
                green = p;
                blue = 100.0;
            } else if (i == 5) {
                red = 100.0;
                green = p;
                blue = q;
            }
        } else {
            if (fixtureIntensities.contains(fixture)) {
                red = 100.0;
                green = 100.0;
                blue = 100.0;
            }
        }
        if (remainingFadeFrames > 0) {
            fixture->dimmer += (dimmer - fixture->dimmer) / remainingFadeFrames;
            fixture->red += (red - fixture->red) / remainingFadeFrames;
            fixture->green += (green - fixture->green) / remainingFadeFrames;
            fixture->blue += (blue - fixture->blue) / remainingFadeFrames;
        } else {
            fixture->dimmer = dimmer;
            fixture->red = red;
            fixture->green = green;
            fixture->blue = blue;
        }
        fixture->raws.clear();
        if (fixtureRaws.contains(fixture)) {
            fixture->raws = fixtureRaws.value(fixture);
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
                if (channelType == QChar('D')) { // DIMMER
                    value = dimmer;
                } else if (channelType == QChar('R')) { // RED
                    value = red;
                } else if (channelType == QChar('G')) { // GREEN
                    value = green;
                } else if (channelType == QChar('B')) { // BLUE
                    value = blue;
                } else if (channelType == QChar('W')) { // WHITE
                    value = 0.0;
                } else if (channelType == QChar('C')) { // CYAN
                    value = (100.0 - red);
                } else if (channelType == QChar('M')) { // MAGENTA
                    value = (100.0 - green);
                } else if (channelType == QChar('Y')) { // YELLOW
                    value = (100.0 - blue);
                } else if (channelType == QChar('0')) {
                    value = 0.0;
                } else if (channelType == QChar('1')) {
                    value = 100.0;
                }
                if (channel <= 512) {
                    dmxUniverses[universe][channel - 1] = (uchar)(value * 2.55 + 0.5);
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
