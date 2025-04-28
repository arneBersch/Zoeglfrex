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

    layout = new QGridLayout();
    setLayout(layout);

    setupColumn(ControlPanelColumns::dimmer, 0, 100, false, Keys::Intensity, kernel->intensities->DIMMERATTRIBUTEID, "Dimmer");
    setupColumn(ControlPanelColumns::hue, 0, 359, true, Keys::Color, kernel->colors->HUEATTRIBUTEID, "Hue");
    setupColumn(ControlPanelColumns::saturation, 0, 100, false, Keys::Color, kernel->colors->SATURATIONATTRIBUTEID, "Saturation");
    setupColumn(ControlPanelColumns::pan, 0, 359, true, Keys::Position, kernel->positions->PANATTRIBUTEID, "Pan");
    setupColumn(ControlPanelColumns::tilt, -180, 180, false, Keys::Position, kernel->positions->TILTATTRIBUTEID, "Tilt");
}

void ControlPanel::setupColumn(int column, int dialMinValue, int dialMaxValue, bool dialWrapping, int itemKey, QString attributeId, QString labelText) {
    QLabel* label = new QLabel(labelText);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, ControlPanelRows::label, column);
    QLabel* valueLabel = new QLabel();
    valueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel, ControlPanelRows::valueLabel, column);
    valueLabels[column] = valueLabel;
    QDial* dial = new QDial();
    dial->setRange(dialMinValue, dialMaxValue);
    dial->setWrapping(dialWrapping);
    connect(dial, &QDial::valueChanged, this, [this, column, itemKey, attributeId] { setValue(column, itemKey, attributeId); });
    layout->addWidget(dial, ControlPanelRows::dial, column);
    dials[column] = dial;
}

void ControlPanel::reload() {
    reloading = true;
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
        valueLabels[ControlPanelColumns::dimmer]->setText(QString::number(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID)) + "%");
        dials[ControlPanelColumns::dimmer]->setDisabled(false);
        dials[ControlPanelColumns::dimmer]->setValue(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID));
    } else {
        valueLabels[ControlPanelColumns::dimmer]->setText("No Intensity");
        dials[ControlPanelColumns::dimmer]->setDisabled(true);
        dials[ControlPanelColumns::dimmer]->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
        valueLabels[ControlPanelColumns::hue]->setText(QString::number(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID)) + "°");
        dials[ControlPanelColumns::hue]->setDisabled(false);
        dials[ControlPanelColumns::hue]->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID));
        valueLabels[ControlPanelColumns::saturation]->setText(QString::number(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID)) + "%");
        dials[ControlPanelColumns::saturation]->setDisabled(false);
        dials[ControlPanelColumns::saturation]->setValue(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID));
    } else {
        valueLabels[ControlPanelColumns::hue]->setText("No Color");
        dials[ControlPanelColumns::hue]->setDisabled(true);
        dials[ControlPanelColumns::hue]->setValue(0);
        valueLabels[ControlPanelColumns::saturation]->setText("No Color");
        dials[ControlPanelColumns::saturation]->setDisabled(true);
        dials[ControlPanelColumns::saturation]->setValue(0);
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
        valueLabels[ControlPanelColumns::pan]->setText(QString::number(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID)) + "°");
        dials[ControlPanelColumns::pan]->setDisabled(false);
        dials[ControlPanelColumns::pan]->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID));
        valueLabels[ControlPanelColumns::tilt]->setText(QString::number(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID)) + "°");
        dials[ControlPanelColumns::tilt]->setDisabled(false);
        dials[ControlPanelColumns::tilt]->setValue(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID));
    } else {
        valueLabels[ControlPanelColumns::pan]->setText("No Position");
        dials[ControlPanelColumns::pan]->setDisabled(true);
        dials[ControlPanelColumns::pan]->setValue(0);
        valueLabels[ControlPanelColumns::tilt]->setText("No Position");
        dials[ControlPanelColumns::tilt]->setDisabled(true);
        dials[ControlPanelColumns::tilt]->setValue(0);
    }
    reloading = false;
}

void ControlPanel::setValue(int column, int itemType, QString attribute) {
    if (!reloading) {
        QMap<int, QString> attributes = {};
        attributes[Keys::Attribute] = attribute;
        kernel->terminal->printMessages = false;
        QString value = QString::number(dials[column]->value());
        if (itemType == Keys::Intensity) {
            kernel->intensities->setAttribute({kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
        } else if (itemType == Keys::Color) {
            kernel->colors->setAttribute({kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
        } else if (itemType == Keys::Position) {
            kernel->positions->setAttribute({kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
        } else {
            Q_ASSERT(false);
        }
        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
}
