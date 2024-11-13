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
    connect(timer, &QTimer::timeout, this, &DmxEngine::generateDmx);

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
}

void DmxEngine::generateDmx() {
    QMutexLocker locker(kernel->mutex);
    QList<uint8_t> formerChannels;
    formerChannels.append(0); // Index 1 should equal DMX Channel 1
    for (int channel=1; channel<=512; channel++) {
        formerChannels.append(sacn->getChannel(channel));
        sacn->setChannel(channel, 0); // reset DMX Data
    }
    if (kernel->cuelistView->currentCue == nullptr) {
        sacn->send();
        return;
    }
    if (kernel->cuelistView->currentCue != lastCue) {
        float fade = kernel->cuelistView->currentCue->fade;
        remainingFadeFrames = 40 * fade + 0.5;
        lastCue = kernel->cuelistView->currentCue;
    }
    for (Group* group : kernel->groups->items) {
        if (kernel->cuelistView->currentCue->intensities.contains(group)) {
            float dimmer = kernel->cuelistView->currentCue->intensities.value(group)->dimmer;
            float red = 100.0;
            float green = 100.0;
            float blue = 100.0;
            if (kernel->cuelistView->currentCue->colors.contains(group)) {
                red = kernel->cuelistView->currentCue->colors.value(group)->red;
                green = kernel->cuelistView->currentCue->colors.value(group)->green;
                blue = kernel->cuelistView->currentCue->colors.value(group)->blue;
            }
            for (Fixture *fixture : group->fixtures) {
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
                    }
                    sacn->setChannel(channel, raw);
                }
            }
        }
    }
    if (remainingFadeFrames > 0) {
        for (int channel = 1; channel <= 512; channel++) {
            float delta = ((float)sacn->getChannel(channel) - (float)formerChannels[channel]) / (float)remainingFadeFrames + 0.5;
            sacn->setChannel(channel, formerChannels[channel] + delta);
        }
        remainingFadeFrames--;
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
