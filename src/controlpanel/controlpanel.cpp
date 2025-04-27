/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "controlpanel.h"
#include "kernel/kernel.h"

ControlPanel::ControlPanel(Kernel* core) {
    kernel = core;

    setWindowTitle("Zöglfrex Control Panel");
    resize(500, 300);

    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    QLabel* dimmerLabel = new QLabel("Dimmer");
    dimmerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(dimmerLabel, 0, 0);
    dimmerDial = new QDial();
    dimmerDial->setRange(0, 100);
    connect(dimmerDial, &QDial::valueChanged, this, &ControlPanel::setDimmer);
    layout->addWidget(dimmerDial, 1, 0);

    QLabel* hueLabel = new QLabel("Hue");
    hueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(hueLabel, 0, 1);
    hueDial = new QDial();
    hueDial->setRange(0, 359);
    hueDial->setWrapping(true);
    connect(hueDial, &QDial::valueChanged, this, &ControlPanel::setHue);
    layout->addWidget(hueDial, 1, 1);

    QLabel* saturationLabel = new QLabel("Saturation");
    saturationLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(saturationLabel, 0, 2);
    saturationDial = new QDial();
    saturationDial->setRange(0, 100);
    connect(saturationDial, &QDial::valueChanged, this, &ControlPanel::setSaturation);
    layout->addWidget(saturationDial, 1, 2);

    QLabel* panLabel = new QLabel("Pan");
    panLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(panLabel, 0, 3);
    panDial = new QDial();
    panDial->setRange(0, 359);
    panDial->setWrapping(true);
    connect(panDial, &QDial::valueChanged, this, &ControlPanel::setPan);
    layout->addWidget(panDial, 1, 3);

    QLabel* tiltLabel = new QLabel("Tilt");
    tiltLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(tiltLabel, 0, 4);
    tiltDial = new QDial();
    tiltDial->setRange(-180, 180);
    connect(tiltDial, &QDial::valueChanged, this, &ControlPanel::setTilt);
    layout->addWidget(tiltDial, 1, 4);
}

void ControlPanel::reload() {
    reloading = true;
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
        dimmerDial->setDisabled(false);
        dimmerDial->setValue(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID));
    } else {
        dimmerDial->setDisabled(true);
        dimmerDial->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
        hueDial->setDisabled(false);
        hueDial->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID));
        saturationDial->setDisabled(false);
        saturationDial->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID));
    } else {
        hueDial->setDisabled(true);
        hueDial->setValue(0);
        saturationDial->setDisabled(true);
        saturationDial->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
        panDial->setDisabled(false);
        panDial->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID));
        tiltDial->setDisabled(false);
        tiltDial->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID));
    } else {
        panDial->setDisabled(true);
        panDial->setValue(0);
        tiltDial->setDisabled(true);
        tiltDial->setValue(0);
    }
    reloading = false;
}

void ControlPanel::setDimmer(int dimmer) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = kernel->intensities->DIMMERATTRIBUTEID;
        kernel->terminal->printMessages = false;
        kernel->intensities->setAttribute({kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, QString::number(dimmer));
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}

void ControlPanel::setHue(int hue) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = kernel->colors->HUEATTRIBUTEID;
        kernel->terminal->printMessages = false;
        kernel->colors->setAttribute({kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, QString::number(hue));
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}

void ControlPanel::setSaturation(int saturation) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = kernel->colors->SATURATIONATTRIBUTEID;
        kernel->terminal->printMessages = false;
        kernel->colors->setAttribute({kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, QString::number(saturation));
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}

void ControlPanel::setPan(int pan) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = kernel->positions->PANATTRIBUTEID;
        kernel->terminal->printMessages = false;
        kernel->positions->setAttribute({kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, QString::number(pan));
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}

void ControlPanel::setTilt(int tilt) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = kernel->positions->TILTATTRIBUTEID;
        kernel->terminal->printMessages = false;
        kernel->positions->setAttribute({kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, QString::number(tilt));
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}
