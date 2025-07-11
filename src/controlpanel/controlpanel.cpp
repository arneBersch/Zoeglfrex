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
    resize(700, 300);

    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    auto setupColumn = [this] (int column, int dialMinValue, int dialMaxValue, bool dialWrapping, int itemKey, QString attributeId, QString labelText, QGridLayout *layout) {
        QLabel* label = new QLabel(labelText);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label, ControlPanelRows::label, column);
        QPushButton* valueButton = new QPushButton();
        valueButton->setCheckable(true);
        connect(valueButton, &QPushButton::clicked, this, [this, column, itemKey, attributeId] { setExceptions(column, itemKey, attributeId); });
        layout->addWidget(valueButton, ControlPanelRows::valueButton, column);
        valueButtons[column] = valueButton;
        QPushButton* modelValueButton = new QPushButton();
        modelValueButton->setCheckable(true);
        connect(modelValueButton, &QPushButton::clicked, this, [this, column, itemKey, attributeId] { setExceptions(column, itemKey, attributeId); });
        layout->addWidget(modelValueButton, ControlPanelRows::modelValueButton, column);
        modelValueButtons[column] = modelValueButton;
        QPushButton* fixtureValueButton = new QPushButton();
        fixtureValueButton->setCheckable(true);
        connect(fixtureValueButton, &QPushButton::clicked, this, [this, column, itemKey, attributeId] { setExceptions(column, itemKey, attributeId); });
        layout->addWidget(fixtureValueButton, ControlPanelRows::fixtureValueButton, column);
        fixtureValueButtons[column] = fixtureValueButton;
        QDial* dial = new QDial();
        dial->setRange(dialMinValue, dialMaxValue);
        dial->setWrapping(dialWrapping);
        connect(dial, &QDial::valueChanged, this, [this, column, itemKey, attributeId] { setValue(column, itemKey, attributeId); });
        layout->addWidget(dial, ControlPanelRows::dial, column);
        dials[column] = dial;
    };

    setupColumn(ControlPanelColumns::dimmer, 0, 100, false, Keys::Intensity, kernel->INTENSITYDIMMERATTRIBUTEID, "Dimmer", layout);
    setupColumn(ControlPanelColumns::hue, 0, 359, true, Keys::Color, kernel->COLORHUEATTRIBUTEID, "Hue", layout);
    setupColumn(ControlPanelColumns::saturation, 0, 100, false, Keys::Color, kernel->COLORSATURATIONATTRIBUTEID, "Saturation", layout);
    setupColumn(ControlPanelColumns::pan, 0, 359, true, Keys::Position, kernel->POSITIONPANATTRIBUTEID, "Pan", layout);
    setupColumn(ControlPanelColumns::tilt, -180, 180, false, Keys::Position, kernel->POSITIONTILTATTRIBUTEID, "Tilt", layout);
    setupColumn(ControlPanelColumns::zoom, 0, 180, false, Keys::Position, kernel->POSITIONZOOMATTRIBUTEID, "Zoom", layout);
}

void ControlPanel::reload() {
    reloading = true;

    auto setColumn = [this] (int column, int itemKey, QString attribute, bool angle, QString unit, QString noValueMessage) {
        Item* item = nullptr;
        if (kernel->cuelistView->selectedCue() != nullptr) {
            if ((itemKey == Keys::Intensity) && kernel->cuelistView->selectedCue()->intensities.contains(kernel->cuelistView->currentGroup)) {
                item = kernel->cuelistView->selectedCue()->intensities.value(kernel->cuelistView->currentGroup);
            } else if ((itemKey == Keys::Color) && kernel->cuelistView->selectedCue()->colors.contains(kernel->cuelistView->currentGroup)) {
                item = kernel->cuelistView->selectedCue()->colors.value(kernel->cuelistView->currentGroup);
            } else if ((itemKey == Keys::Position) && kernel->cuelistView->selectedCue()->positions.contains(kernel->cuelistView->currentGroup)) {
                item = kernel->cuelistView->selectedCue()->positions.value(kernel->cuelistView->currentGroup);
            }
        }
        if (item == nullptr) {
            valueButtons[column]->setText(noValueMessage);
            valueButtons[column]->setChecked(false);
            valueButtons[column]->setDisabled(true);
            modelValueButtons[column]->setText(noValueMessage);
            modelValueButtons[column]->setDisabled(true);
            fixtureValueButtons[column]->setText(noValueMessage);
            fixtureValueButtons[column]->setDisabled(true);
            dials[column]->setValue(0);
            dials[column]->setDisabled(true);
        } else {
            valueButtons[column]->setChecked(true);
            valueButtons[column]->setDisabled(true);
            if (angle) {
                valueButtons[column]->setText(QString::number(item->angleAttributes.value(attribute)) + unit);
            } else {
                valueButtons[column]->setText(QString::number(item->floatAttributes.value(attribute)) + unit);
            }

            if ((kernel->cuelistView->currentFixture != nullptr) && (kernel->cuelistView->currentFixture->model != nullptr)) {
                modelValueButtons[column]->setEnabled(true);
                float modelValue;
                if (angle) {
                    if (item->modelSpecificAngleAttributes.value(attribute).contains(kernel->cuelistView->currentFixture->model)) {
                        modelValueButtons[column]->setChecked(true);
                        modelValue = item->modelSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                    } else {
                        modelValue = item->angleAttributes.value(attribute);
                        modelValueButtons[column]->setChecked(false);
                    }
                } else {
                    if (item->modelSpecificFloatAttributes.value(attribute).contains(kernel->cuelistView->currentFixture->model)) {
                        modelValueButtons[column]->setChecked(true);
                        modelValue = item->modelSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                    } else {
                        modelValue = item->floatAttributes.value(attribute);
                        modelValueButtons[column]->setChecked(false);
                    }
                }
                modelValueButtons[column]->setText("Model:\n" + QString::number(modelValue) + unit);
            } else {
                modelValueButtons[column]->setChecked(false);
                modelValueButtons[column]->setText("No Model\nselected.");
                modelValueButtons[column]->setDisabled(true);
            }

            if (kernel->cuelistView->currentFixture != nullptr) {
                fixtureValueButtons[column]->setEnabled(true);
                float fixtureValue;
                if (angle) {
                    if (item->fixtureSpecificAngleAttributes.value(attribute).contains(kernel->cuelistView->currentFixture)) {
                        fixtureValueButtons[column]->setChecked(true);
                        fixtureValue = item->fixtureSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture);
                    } else {
                        fixtureValueButtons[column]->setChecked(false);
                        if (modelValueButtons[column]->isChecked()) {
                            fixtureValue = item->modelSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                        } else {
                            fixtureValue = item->angleAttributes.value(attribute);
                        }
                    }
                } else {
                    if (item->fixtureSpecificFloatAttributes.value(attribute).contains(kernel->cuelistView->currentFixture)) {
                        fixtureValueButtons[column]->setChecked(true);
                        fixtureValue = item->fixtureSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture);
                    } else {
                        fixtureValueButtons[column]->setChecked(false);
                        if (modelValueButtons[column]->isChecked()) {
                            fixtureValue = item->modelSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                        } else {
                            fixtureValue = item->floatAttributes.value(attribute);
                        }
                    }
                }
                fixtureValueButtons[column]->setText("Fixture:\n" + QString::number(fixtureValue) + unit);
            } else {
                fixtureValueButtons[column]->setChecked(false);
                fixtureValueButtons[column]->setText("No Fixture\nselected.");
                fixtureValueButtons[column]->setDisabled(true);
            }

            dials[column]->setEnabled(true);
            float dialValue;
            if (fixtureValueButtons[column]->isChecked()) {
                if (angle) {
                    dialValue = item->fixtureSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture);
                } else {
                    dialValue = item->fixtureSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture);
                }
            } else if (modelValueButtons[column]->isChecked()) {
                if (angle) {
                    dialValue = item->modelSpecificAngleAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                } else {
                    dialValue = item->modelSpecificFloatAttributes.value(attribute).value(kernel->cuelistView->currentFixture->model);
                }
            } else {
                if (angle) {
                    dialValue = item->angleAttributes.value(attribute);
                } else {
                    dialValue = item->floatAttributes.value(attribute);
                }
            }
            dials[column]->setValue(dialValue);
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

void ControlPanel::setAttribute(int itemKey, QMap<int, QString> attributes, QList<int> keys, QString value) {
    Q_ASSERT(kernel->cuelistView->selectedCue() != nullptr);
    Q_ASSERT(kernel->cuelistView->currentGroup != nullptr);
    if (itemKey == Keys::Intensity) {
        kernel->intensities->setAttribute({kernel->cuelistView->selectedCue()->intensities.value(kernel->cuelistView->currentGroup)->id}, attributes, keys, value);
    } else if (itemKey == Keys::Color) {
        kernel->colors->setAttribute({kernel->cuelistView->selectedCue()->colors.value(kernel->cuelistView->currentGroup)->id}, attributes, keys, value);
    } else if (itemKey == Keys::Position) {
        kernel->positions->setAttribute({kernel->cuelistView->selectedCue()->positions.value(kernel->cuelistView->currentGroup)->id}, attributes, keys, value);
    } else {
        Q_ASSERT(false);
    }
};

void ControlPanel::setExceptions(int column, int itemKey, QString attributeId) {
    if (!reloading) {
        QMutexLocker locker(kernel->mutex);
        kernel->terminal->printMessages = false;

        if (modelValueButtons[column]->isEnabled()) {
            QMap<int, QString> attributes = QMap<int, QString>();
            attributes[Keys::Attribute] = attributeId;
            attributes[Keys::Model] = kernel->cuelistView->currentFixture->model->id;
            if (modelValueButtons[column]->isChecked()) {
                setAttribute(itemKey, attributes, {Keys::Plus, Keys::Zero});
            } else {
                setAttribute(itemKey, attributes, {Keys::Minus});
            }
        }

        if (fixtureValueButtons[column]->isEnabled()) {
            QMap<int, QString> attributes = QMap<int, QString>();
            attributes[Keys::Attribute] = attributeId;
            attributes[Keys::Fixture] = kernel->cuelistView->currentFixture->id;
            if (fixtureValueButtons[column]->isChecked()) {
                setAttribute(itemKey, attributes, {Keys::Plus, Keys::Zero});
            } else {
                setAttribute(itemKey, attributes, {Keys::Minus});
            }
        }

        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
};

void ControlPanel::setValue(int column, int itemKey, QString attributeId) {
    if (!reloading) {
        QMutexLocker locker(kernel->mutex);
        kernel->terminal->printMessages = false;

        QMap<int, QString> attributes = QMap<int, QString>();
        attributes[Keys::Attribute] = attributeId;
        if (fixtureValueButtons[column]->isChecked()) {
            attributes[Keys::Fixture] = kernel->cuelistView->currentFixture->id;
        } else if (modelValueButtons[column]->isChecked()) {
            attributes[Keys::Model] = kernel->cuelistView->currentFixture->model->id;
        }
        QString value = QString::number(dials[column]->value());
        setAttribute(itemKey, attributes, {}, value);

        kernel->terminal->printMessages = true;
        kernel->cuelistView->reload();
    }
};
