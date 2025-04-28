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
    layout->addWidget(dimmerLabel, ControlPanelRows::label, ControlPanelColumns::dimmer);
    dimmerValueLabel = new QLabel();
    dimmerValueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(dimmerValueLabel, ControlPanelRows::valueLabel, ControlPanelColumns::dimmer);
    dimmerDial = new QDial();
    dimmerDial->setRange(0, 100);
    connect(dimmerDial, &QDial::valueChanged, this, &ControlPanel::setDimmer);
    layout->addWidget(dimmerDial, ControlPanelRows::dial, ControlPanelColumns::dimmer);

    QLabel* hueLabel = new QLabel("Hue");
    hueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(hueLabel, ControlPanelRows::label, ControlPanelColumns::hue);
    hueValueLabel = new QLabel();
    hueValueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(hueValueLabel, ControlPanelRows::valueLabel, ControlPanelColumns::hue);
    hueDial = new QDial();
    hueDial->setRange(0, 359);
    hueDial->setWrapping(true);
    connect(hueDial, &QDial::valueChanged, this, &ControlPanel::setHue);
    layout->addWidget(hueDial, ControlPanelRows::dial, ControlPanelColumns::hue);

    QLabel* saturationLabel = new QLabel("Saturation");
    saturationLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(saturationLabel, ControlPanelRows::label, ControlPanelColumns::saturation);
    saturationValueLabel = new QLabel();
    saturationValueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(saturationValueLabel, ControlPanelRows::valueLabel, ControlPanelColumns::saturation);
    saturationDial = new QDial();
    saturationDial->setRange(0, 100);
    connect(saturationDial, &QDial::valueChanged, this, &ControlPanel::setSaturation);
    layout->addWidget(saturationDial, ControlPanelRows::dial, ControlPanelColumns::saturation);

    QLabel* panLabel = new QLabel("Pan");
    panLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(panLabel, ControlPanelRows::label, ControlPanelColumns::pan);
    panValueLabel = new QLabel();
    panValueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(panValueLabel, ControlPanelRows::valueLabel, ControlPanelColumns::pan);
    panDial = new QDial();
    panDial->setRange(0, 359);
    panDial->setWrapping(true);
    connect(panDial, &QDial::valueChanged, this, &ControlPanel::setPan);
    layout->addWidget(panDial, ControlPanelRows::dial, ControlPanelColumns::pan);

    QLabel* tiltLabel = new QLabel("Tilt");
    tiltLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(tiltLabel, ControlPanelRows::label, ControlPanelColumns::tilt);
    tiltValueLabel = new QLabel();
    tiltValueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(tiltValueLabel, ControlPanelRows::valueLabel, ControlPanelColumns::tilt);
    tiltDial = new QDial();
    tiltDial->setRange(-180, 180);
    connect(tiltDial, &QDial::valueChanged, this, &ControlPanel::setTilt);
    layout->addWidget(tiltDial, ControlPanelRows::dial, ControlPanelColumns::tilt);
}

void ControlPanel::reload() {
    reloading = true;
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
        dimmerValueLabel->setText(QString::number(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%");
        dimmerDial->setDisabled(false);
        dimmerDial->setValue(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID));
    } else {
        dimmerValueLabel->setText("No Intensity");
        dimmerDial->setDisabled(true);
        dimmerDial->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
        hueValueLabel->setText(QString::number(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID)) + "°");
        hueDial->setDisabled(false);
        hueDial->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID));
        saturationValueLabel->setText(QString::number(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID)) + "%");
        saturationDial->setDisabled(false);
        saturationDial->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID));
    } else {
        hueValueLabel->setText("No Color");
        hueDial->setDisabled(true);
        hueDial->setValue(0);
        saturationValueLabel->setText("No Color");
        saturationDial->setDisabled(true);
        saturationDial->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
        panValueLabel->setText(QString::number(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°");
        panDial->setDisabled(false);
        panDial->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID));
        tiltValueLabel->setText(QString::number(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "°");
        tiltDial->setDisabled(false);
        tiltDial->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID));
    } else {
        panValueLabel->setText("No Position");
        panDial->setDisabled(true);
        panDial->setValue(0);
        tiltValueLabel->setText("No Position");
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
