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

    kernel->mainWindow->setupShortcuts(this);
    setWindowTitle("Zöglfrex Control Panel");
    resize(500, 300);

    layout = new QGridLayout();
    setLayout(layout);

    auto setupColumn = [this] (int column, int dialMinValue, int dialMaxValue, bool dialWrapping, int itemKey, QString attributeId, QString labelText) {
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
        connect(dial, &QDial::valueChanged, this, [this, column, itemKey, attributeId] {
            if (!reloading) {
                QMap<int, QString> attributes = {};
                attributes[Keys::Attribute] = attributeId;
                kernel->terminal->printMessages = false;
                QString value = QString::number(dials[column]->value());
                if (itemKey == Keys::Intensity) {
                    kernel->intensities->setAttribute({kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
                } else if (itemKey == Keys::Color) {
                    kernel->colors->setAttribute({kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
                } else if (itemKey == Keys::Position) {
                    kernel->positions->setAttribute({kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
                } else {
                    Q_ASSERT(false);
                }
                kernel->terminal->printMessages = true;
                kernel->cuelistView->reload();
            }
        });
        layout->addWidget(dial, ControlPanelRows::dial, column);
        dials[column] = dial;
    };

    setupColumn(ControlPanelColumns::dimmer, 0, 100, false, Keys::Intensity, kernel->intensities->DIMMERATTRIBUTEID, "Dimmer");
    setupColumn(ControlPanelColumns::hue, 0, 359, true, Keys::Color, kernel->colors->HUEATTRIBUTEID, "Hue");
    setupColumn(ControlPanelColumns::saturation, 0, 100, false, Keys::Color, kernel->colors->SATURATIONATTRIBUTEID, "Saturation");
    setupColumn(ControlPanelColumns::pan, 0, 359, true, Keys::Position, kernel->positions->PANATTRIBUTEID, "Pan");
    setupColumn(ControlPanelColumns::tilt, -180, 180, false, Keys::Position, kernel->positions->TILTATTRIBUTEID, "Tilt");
}

void ControlPanel::reload() {
    reloading = true;

    auto setColumn = [this] (int column, float value, QString unit) {
        valueLabels[column]->setText(QString::number(value) + unit);
        dials[column]->setDisabled(false);
        dials[column]->setValue(value);
    };
    auto emptyColumn = [this] (int column, QString valueMessage) {
        valueLabels[column]->setText(valueMessage);
        dials[column]->setDisabled(true);
        dials[column]->setValue(0);
    };

    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
        setColumn(ControlPanelColumns::dimmer, kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->intensities->DIMMERATTRIBUTEID), "%");
    } else {
        emptyColumn(ControlPanelColumns::dimmer, "No Intensity");
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
        setColumn(ControlPanelColumns::hue, kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->colors->HUEATTRIBUTEID), "°");
        setColumn(ControlPanelColumns::saturation, kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID), "%");
    } else {
        emptyColumn(ControlPanelColumns::hue, "No Color");
        emptyColumn(ControlPanelColumns::saturation, "No Color");
    }
    if ((kernel->cuelistView->currentCue != nullptr) && kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
        setColumn(ControlPanelColumns::pan, kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->angleAttributes.value(kernel->positions->PANATTRIBUTEID), "°");;
        setColumn(ControlPanelColumns::tilt, kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->floatAttributes.value(kernel->positions->TILTATTRIBUTEID), "°");
    } else {
        emptyColumn(ControlPanelColumns::pan, "No Position");
        emptyColumn(ControlPanelColumns::tilt, "No Position");
    }
    reloading = false;
}
