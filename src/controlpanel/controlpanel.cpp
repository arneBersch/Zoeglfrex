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
        QLabel* modelValueLabel = new QLabel();
        modelValueLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(modelValueLabel, ControlPanelRows::modelValueLabel, column);
        modelValueLabels[column] = modelValueLabel;
        QLabel* fixtureValueLabel = new QLabel();
        fixtureValueLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(fixtureValueLabel, ControlPanelRows::fixtureValueLabel, column);
        fixtureValueLabels[column] = fixtureValueLabel;
        QDial* dial = new QDial();
        dial->setRange(dialMinValue, dialMaxValue);
        dial->setWrapping(dialWrapping);
        connect(dial, &QDial::valueChanged, this, [this, column, itemKey, attributeId] {
            if (!reloading) {
                QMap<int, QString> attributes = {};
                attributes[Keys::Attribute] = attributeId;
                if (!fixtureValueLabels[column]->text().isEmpty()) {
                    attributes[Keys::Fixture] = kernel->cuelistView->currentFixture->id;
                } else if (!modelValueLabels[column]->text().isEmpty()) {
                    attributes[Keys::Model] = kernel->cuelistView->currentFixture->model->id;
                }
                kernel->terminal->printMessages = false;
                QString value = QString::number(dials[column]->value());
                Q_ASSERT(kernel->cuelistView->currentCuelist != nullptr);
                Q_ASSERT(kernel->cuelistView->currentCuelist->currentCue != nullptr);
                if (itemKey == Keys::Intensity) {
                    kernel->intensities->setAttribute({kernel->cuelistView->currentCuelist->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
                } else if (itemKey == Keys::Color) {
                    kernel->colors->setAttribute({kernel->cuelistView->currentCuelist->currentCue->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
                } else if (itemKey == Keys::Position) {
                    kernel->positions->setAttribute({kernel->cuelistView->currentCuelist->currentCue->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, {}, value);
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

    setupColumn(ControlPanelColumns::dimmer, 0, 100, false, Keys::Intensity, kernel->INTENSITYDIMMERATTRIBUTEID, "Dimmer");
    setupColumn(ControlPanelColumns::hue, 0, 359, true, Keys::Color, kernel->COLORHUEATTRIBUTEID, "Hue");
    setupColumn(ControlPanelColumns::saturation, 0, 100, false, Keys::Color, kernel->COLORSATURATIONATTRIBUTEID, "Saturation");
    setupColumn(ControlPanelColumns::pan, 0, 359, true, Keys::Position, kernel->POSITIONPANATTRIBUTEID, "Pan");
    setupColumn(ControlPanelColumns::tilt, -180, 180, false, Keys::Position, kernel->POSITIONTILTATTRIBUTEID, "Tilt");
    setupColumn(ControlPanelColumns::zoom, 0, 180, false, Keys::Position, kernel->POSITIONZOOMATTRIBUTEID, "Zoom");
}

void ControlPanel::reload() {
    reloading = true;

    auto setColumn = [this] (int column, int itemKey, QString attribute, bool angle, QString unit, QString noValueMessage) {
        Item* item = nullptr;
        if ((itemKey == Keys::Intensity) && (kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCuelist->currentCue->intensities.value(kernel->cuelistView->currentGroup);
        } else if ((itemKey == Keys::Color) && (kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCuelist->currentCue->colors.value(kernel->cuelistView->currentGroup);
        } else if ((itemKey == Keys::Position) && (kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr) && kernel->cuelistView->currentCuelist->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
            item = kernel->cuelistView->currentCuelist->currentCue->positions.value(kernel->cuelistView->currentGroup);
        }
        valueLabels[column]->setText(noValueMessage);
        modelValueLabels[column]->setText(QString());
        fixtureValueLabels[column]->setText(QString());
        dials[column]->setValue(0);
        if (item == nullptr) {
            dials[column]->setDisabled(true);
        } else {
            dials[column]->setDisabled(false);
            if (angle) {
                valueLabels[column]->setText(QString::number(item->angleAttributes.value(attribute)) + unit);
                if ((kernel->cuelistView->currentFixture != nullptr) && item->modelSpecificAngleAttributes.value(attribute).contains(kernel->cuelistView->currentFixture->model)) {
                    modelValueLabels[column]->setText("Model:\n" + QString::number(item->modelSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model)) + unit);
                }
                if (item->fixtureSpecificAngleAttributes.value(attribute).contains(kernel->cuelistView->currentFixture)) {
                    fixtureValueLabels[column]->setText("Fixture:\n" + QString::number(item->fixtureSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture)) + unit);
                }
                if (!fixtureValueLabels[column]->text().isEmpty()) {
                    dials[column]->setValue(item->fixtureSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture));
                } else if (!modelValueLabels[column]->text().isEmpty()) {
                    dials[column]->setValue(item->modelSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model));
                } else {
                    dials[column]->setValue(item->angleAttributes.value(attribute));
                }
            } else {
                valueLabels[column]->setText(QString::number(item->floatAttributes.value(attribute)) + unit);
                if ((kernel->cuelistView->currentFixture != nullptr) && item->modelSpecificFloatAttributes.value(attribute).contains(kernel->cuelistView->currentFixture->model)) {
                    modelValueLabels[column]->setText("Model:\n" + QString::number(item->modelSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model)) + unit);
                }
                if (item->fixtureSpecificFloatAttributes.value(attribute).contains(kernel->cuelistView->currentFixture)) {
                    fixtureValueLabels[column]->setText("Fixture:\n" + QString::number(item->fixtureSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture)) + unit);
                }
                if (!fixtureValueLabels[column]->text().isEmpty()) {
                    dials[column]->setValue(item->fixtureSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture));
                } else if (!modelValueLabels[column]->text().isEmpty()) {
                    dials[column]->setValue(item->modelSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model));
                } else {
                    dials[column]->setValue(item->floatAttributes.value(attribute));
                }
            }
        }
    };

    setColumn(ControlPanelColumns::dimmer, Keys::Intensity, kernel->INTENSITYDIMMERATTRIBUTEID, false, "%", "No Intensity");
    setColumn(ControlPanelColumns::hue, Keys::Color, kernel->COLORHUEATTRIBUTEID, true, "°", "No Color");
    setColumn(ControlPanelColumns::saturation, Keys::Color, kernel->COLORSATURATIONATTRIBUTEID, false, "%", "No Color");
    setColumn(ControlPanelColumns::pan, Keys::Position, kernel->POSITIONPANATTRIBUTEID, true, "°", "No Position");;
    setColumn(ControlPanelColumns::tilt, Keys::Position, kernel->POSITIONTILTATTRIBUTEID, false, "°", "No Position");
    setColumn(ControlPanelColumns::zoom, Keys::Position, kernel->POSITIONZOOMATTRIBUTEID, false, "°", "No Position");
    reloading = false;
}
