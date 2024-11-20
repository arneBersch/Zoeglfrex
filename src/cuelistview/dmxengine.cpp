/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"
#include "kernel/kernel.h"

DmxEngine::DmxEngine(Kernel *core, QWidget *parent) : QWidget{parent} {
    kernel = core;
    sacn = new SacnServer();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DmxEngine::sendDmx);

    QHBoxLayout *layout = new QHBoxLayout(this);

    interfaceSelectionBox = new QComboBox();
    for (QNetworkInterface interface : QNetworkInterface::allInterfaces()) {
        for (QNetworkAddressEntry address : interface.addressEntries()) {
            if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                interfaceSelectionBox->addItem(address.ip().toString());
            }
        }
    }
    layout->addWidget(interfaceSelectionBox);

    startDmxButton  = new QPushButton("Start sACN Output");
    connect(startDmxButton, &QPushButton::pressed, this, &DmxEngine::startDmx);
    layout->addWidget(startDmxButton);

    stopDmxButton  = new QPushButton("Stop sACN Output");
    stopDmxButton->setEnabled(false);
    connect(stopDmxButton, &QPushButton::pressed, this, &DmxEngine::stopDmx);
    layout->addWidget(stopDmxButton);

    for (int channel=0; channel<=512; channel++) {
        currentCueValues.append(0);
        lastCueValues.append(0);
    }
}

void DmxEngine::generateDmx() {
    if (kernel->cuelistView->currentCue == nullptr) {
        remainingFadeFrames = 0;
        totalFadeFrames = 0;
        for (int channel = 1; channel <= 512; channel++) {
            currentCueValues[channel] = 0; // reset current cue values
        }
    } else {
        if (kernel->cuelistView->currentCue != lastCue) {
            float fade = kernel->cuelistView->currentCue->fade;
            totalFadeFrames = 40 * fade + 0.5;
            remainingFadeFrames = totalFadeFrames;
            lastCue = kernel->cuelistView->currentCue;
            lastCueValues = currentCueValues;
            for (int channel = 1; channel <= 512; channel++) {
                currentCueValues[channel] = 0; // reset current cue values
            }
        }
        QMap<Fixture*, Intensity*> fixtureIntensities;
        QMap<Fixture*, Color*> fixtureColors;
        for (Group* group : kernel->groups->items) {
            if (lastCue->intensities.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureIntensities[fixture] = lastCue->intensities[group];
                }
            }
            if (lastCue->colors.contains(group)) {
                for (Fixture* fixture : group->fixtures) {
                    fixtureColors[fixture] = lastCue->colors[group];
                }
            }
        }
        for (Fixture* fixture : kernel->fixtures->items) {
            float dimmer = 0.0;
            if (fixtureIntensities.contains(fixture)) {
                dimmer = fixtureIntensities.value(fixture)->dimmer;
            }
            float red = 100.0;
            float green = 100.0;
            float blue = 100.0;
            if (fixtureColors.contains(fixture)) {
                red = fixtureColors.value(fixture)->red;
                green = fixtureColors.value(fixture)->green;
                blue = fixtureColors.value(fixture)->blue;
            }
            if (fixture->address > 0) {
                QString channels = fixture->model->channels;
                for (int channel = fixture->address; channel < (fixture->address + channels.size()); channel++) {
                    uint8_t raw = 0;
                    if (channels.at(channel - fixture->address) == QChar('D')) { // DIMMER
                        raw = (dimmer * 2.55 + 0.5);
                    } else if (channels.at(channel - fixture->address) == QChar('R')) { // RED
                        raw = (red * 2.55 + 0.5);
                        if (!channels.contains("D")) { // Create Virtual Dimmer
                            raw = (red * 2.55 * dimmer / 100 + 0.5);
                        }
                    } else if (channels.at(channel - fixture->address) == QChar('G')) { // GREEN
                        raw = (green * 2.55 + 0.5);
                        if (!channels.contains("D")) { // Create Virtual Dimmer
                            raw = (green * 2.55 * dimmer / 100 + 0.5);
                        }
                    } else if (channels.at(channel - fixture->address) == QChar('B')) { // BLUE
                        raw = (blue * 2.55 + 0.5);
                        if (!channels.contains("D")) { // Create Virtual Dimmer
                            raw = (blue * 2.55 * dimmer / 100 + 0.5);
                        }
                    } else if (channels.at(channel - fixture->address) == QChar('0')) {
                        raw = 0;
                    } else if (channels.at(channel - fixture->address) == QChar('1')) {
                        raw = 255;
                    }
                    currentCueValues[channel] = raw;
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
            sacn->setChannel(channel, lastCueValues[channel] + delta);
        }
        remainingFadeFrames--;
    } else {
        for (int channel = 1; channel <= 512; channel++) {
            sacn->setChannel(channel, currentCueValues[channel]);
        }
    }
    sacn->send();
}

void DmxEngine::startDmx() {
    interfaceSelectionBox->setEnabled(false);
    startDmxButton->setEnabled(false);
    stopDmxButton->setEnabled(true);
    sacn->connect(interfaceSelectionBox->currentText());
    timer->start(25);
}

void DmxEngine::stopDmx() {
    QMessageBox messageBox;
    messageBox.setText("Are you sure you want to stop sACN Output?");
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Cancel);
    int result  = messageBox.exec();
    if (result != QMessageBox::Ok) {
        return;
    }
    timer->stop();
    interfaceSelectionBox->setEnabled(true);
    startDmxButton->setEnabled(true);
    stopDmxButton->setEnabled(false);
    sacn->disconnect();
}
