/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dmxengine.h"
#include "kernel/kernel.h"

DmxEngine::DmxEngine(Kernel *core, QWidget *parent) : QWidget(parent, Qt::Window) {
    kernel = core;
    sacn = new SacnServer();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DmxEngine::sendDmx);
    timer->start(25);

    setWindowTitle("DMX Output Settings");
    QVBoxLayout *layout = new QVBoxLayout(this);

    interfaceSelectionBox = new QComboBox();
    interfaceSelectionBox->addItem("None");
    for (QNetworkInterface interface : QNetworkInterface::allInterfaces()) {
        for (QNetworkAddressEntry address : interface.addressEntries()) {
            if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                interfaceSelectionBox->addItem(interface.name() + " (" + address.ip().toString() + ")");
                networkInterfaces.append(interface);
                networkAddresses.append(address);
            }
        }
    }
    connect(interfaceSelectionBox, &QComboBox::currentIndexChanged, this, &DmxEngine::setNetworkInterface);
    layout->addWidget(interfaceSelectionBox);

    for (int channel=0; channel<=512; channel++) {
        currentCueValues.append(0);
        lastCueValues.append(0);
    }
}

void DmxEngine::generateDmx() {
    QMap<Fixture*, Intensity*> fixtureIntensities;
    QMap<Fixture*, Color*> fixtureColors;
    if (kernel->cuelistView->currentCue == nullptr) {
        remainingFadeFrames = 0;
        totalFadeFrames = 0;
    } else {
        if (kernel->cuelistView->currentCue != lastCue) {
            float fade = kernel->cuelistView->currentCue->fade;
            totalFadeFrames = 40 * fade + 0.5;
            remainingFadeFrames = totalFadeFrames;
            lastCue = kernel->cuelistView->currentCue;
            lastCueValues = currentCueValues;
        }
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
    }
    for (int channel = 1; channel <= 512; channel++) {
        currentCueValues[channel] = 0; // reset current cue values
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        QString channels = fixture->model->channels;
        float dimmer = 0.0;
        float red = 0.0;
        float green = 0.0;
        float blue = 0.0;
        if (fixtureIntensities.contains(fixture)) {
            dimmer = fixtureIntensities.value(fixture)->dimmer;
        }
        if (fixtureColors.contains(fixture)) {
            const double h = (fixtureColors.value(fixture)->hue / 60.0);
            const int i = (int)h;
            const double f = h - i;
            const double p = (100.0 - fixtureColors.value(fixture)->saturation);
            const double q = (100.0 - (fixtureColors.value(fixture)->saturation * f));
            const double t = (100.0 - (fixtureColors.value(fixture)->saturation * (1.0 - f)));
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
        if (!channels.contains('D')) {
            red *= (dimmer / 100.0);
            green *= (dimmer / 100.0);
            blue *= (dimmer / 100.0);
        }
        if (fixture->address > 0) {
            for (int channel = fixture->address; channel < (fixture->address + channels.size()); channel++) {
                float value = 0.0;
                if (channels.at(channel - fixture->address) == QChar('D')) { // DIMMER
                    value = dimmer;
                } else if (channels.at(channel - fixture->address) == QChar('R')) { // RED
                    value = red;
                } else if (channels.at(channel - fixture->address) == QChar('G')) { // GREEN
                    value = green;
                } else if (channels.at(channel - fixture->address) == QChar('B')) { // BLUE
                    value = blue;
                } else if (channels.at(channel - fixture->address) == QChar('C')) { // CYAN
                    value = (100.0 - red);
                } else if (channels.at(channel - fixture->address) == QChar('M')) { // MAGENTA
                    value = (100.0 - green);
                } else if (channels.at(channel - fixture->address) == QChar('Y')) { // YELLOW
                    value = (100.0 - blue);
                } else if (channels.at(channel - fixture->address) == QChar('0')) {
                    value = 0.0;
                } else if (channels.at(channel - fixture->address) == QChar('1')) {
                    value = 100.0;
                }
                uint8_t raw = (value * 2.55 + 0.5);
                currentCueValues[channel] = raw;
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

void DmxEngine::setNetworkInterface() {
    int interfaceIndex = interfaceSelectionBox->currentIndex() - 1;
    if (interfaceIndex >= 0) {
        sacn->connect(networkInterfaces[interfaceIndex], networkAddresses[interfaceIndex]);
        kernel->terminal->success("Set sACN Output to " + networkInterfaces[interfaceIndex].name() + " (" + networkAddresses[interfaceIndex].ip().toString() + ")");
    } else {
        sacn->disconnect();
        kernel->terminal->success("Disabled sACN output.");
    }
}
