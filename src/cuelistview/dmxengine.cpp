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

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &DmxEngine::sendDmx);
    timer->start(25);

    for (int channel=0; channel<=512; channel++) {
        currentCueValues.append(0);
        lastCueValues.append(0);
    }
}

void DmxEngine::generateDmx() {
    QMap<Fixture*, Intensity*> fixtureIntensities;
    QMap<Fixture*, Color*> fixtureColors;
    QMap<Fixture*, QList<Raw*>> fixtureRaws;
    if (kernel->cuelistView->currentCue == nullptr) {
        remainingFadeFrames = 0;
        totalFadeFrames = 0;
    } else {
        if (kernel->cuelistView->currentCue != lastCue) {
            float fade = kernel->cuelistView->currentCue->floatAttributes[kernel->cues->FADEATTRIBUTEID];
            totalFadeFrames = 40 * fade + 0.5;
            remainingFadeFrames = totalFadeFrames;
            lastCue = kernel->cuelistView->currentCue;
            lastCueValues = currentCueValues;
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
        }
    }
    for (int channel = 1; channel <= 512; channel++) {
        currentCueValues[channel] = 0; // reset current cue values
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        QString channels = "D";
        if (fixture->model != nullptr) {
            channels = fixture->model->channels;
        }
        float dimmer = 0.0;
        float red = 0.0;
        float green = 0.0;
        float blue = 0.0;
        if (fixtureIntensities.contains(fixture)) {
            dimmer = fixtureIntensities.value(fixture)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID);
            if (fixtureIntensities.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).contains(fixture)) {
                dimmer = fixtureIntensities.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID).value(fixture);
            }
        }
        if (fixtureColors.contains(fixture)) {
            const double h = (fixtureColors.value(fixture)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID) / 60.0);
            const int i = (int)h;
            const double f = h - i;
            float saturation = fixtureColors.value(fixture)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID);
            if (fixtureColors.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).contains(fixture)) {
                saturation = fixtureColors.value(fixture)->fixtureSpecificFloatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID).value(fixture);
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
        if (highlightButton->isChecked() && (kernel->cuelistView->currentGroup != nullptr) && (kernel->cuelistView->currentGroup->fixtures.contains(fixture))) { // Highlight
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
        float white = std::min(std::min(red, green), blue);
        if (channels.contains('W')) { // RGB to RGBW
            red -= white;
            green -= white;
            blue -= white;
        }
        int address = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID);
        if (address > 0) {
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
                    value = white;
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
                uint8_t raw = (value * 2.55 + 0.5);
                if (channel <= 512) {
                    currentCueValues[channel] = raw;
                }
            }
            if (fixtureRaws.contains(fixture)) {
                for (Raw* raw : fixtureRaws[fixture]) {
                    int channel = address + raw->intAttributes.value(kernel->raws->CHANNELATTRIBUTEID) - 1;
                    if ((channel <= 512) && (raw->intAttributes.value(kernel->raws->CHANNELATTRIBUTEID) <= channels.size())) {
                        currentCueValues[channel] = raw->intAttributes.value(kernel->raws->VALUEATTRIBUTEID);
                    }
                }
            }
        }
    }
}

void DmxEngine::sendDmx() {
    QMutexLocker(kernel->mutex);
    if (remainingFadeFrames > 0) {
        for (int channel = 1; channel <= 512; channel++) {
            float delta = ((float)currentCueValues[channel] - (float)lastCueValues[channel]);
            delta *= (((float)totalFadeFrames - (float)remainingFadeFrames) / (float)totalFadeFrames);
            sacnServer->setChannel(channel, lastCueValues[channel] + delta);
        }
        remainingFadeFrames--;
    } else {
        for (int channel = 1; channel <= 512; channel++) {
            sacnServer->setChannel(channel, currentCueValues[channel]);
        }
    }
    sacnServer->send();
    fadeProgress->setValue(totalFadeFrames + 1 - remainingFadeFrames);
    fadeProgress->setRange(0, totalFadeFrames + 1);
}
