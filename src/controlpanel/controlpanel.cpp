/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex Control Panel");
    QGridLayout* layout = new QGridLayout();
    setLayout(layout);

    intensityLabel = new QLabel();
    intensityLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(intensityLabel, 0, 0);
    QHBoxLayout* intensityAttributesLayout = new QHBoxLayout();
    layout->addLayout(intensityAttributesLayout, 1, 0);

    intensityDimmerPanel = new AttributePanel("intensities", "dimmer", "cue_group_intensities", "intensity_model_dimmer", "intensity_fixture_dimmer", "Dimmer", "%", 0, 100, false);
    connect(intensityDimmerPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    intensityAttributesLayout->addWidget(intensityDimmerPanel);

    colorLabel = new QLabel();
    colorLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(colorLabel, 0, 1);
    QHBoxLayout* colorAttributesLayout = new QHBoxLayout();
    layout->addLayout(colorAttributesLayout, 1, 1);

    colorHuePanel = new AttributePanel("colors", "hue", "cue_group_colors", "color_model_hue", "color_fixture_hue", "Hue", "°", 0, 360, true);
    connect(colorHuePanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    colorAttributesLayout->addWidget(colorHuePanel);

    colorSaturationPanel = new AttributePanel("colors", "saturation", "cue_group_colors", "color_model_saturation", "color_fixture_saturation", "Saturation", "%", 0, 100, false);
    connect(colorSaturationPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    colorAttributesLayout->addWidget(colorSaturationPanel);

    positionLabel = new QLabel();
    positionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(positionLabel, 0, 2);
    QHBoxLayout* positionAttributesLayout = new QHBoxLayout();
    layout->addLayout(positionAttributesLayout, 1, 2);

    positionPanPanel = new AttributePanel("positions", "pan", "cue_group_positions", "position_model_pan", "position_fixture_pan", "Pan", "°", 0, 360, true);
    connect(positionPanPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    positionAttributesLayout->addWidget(positionPanPanel);

    positionTiltPanel = new AttributePanel("positions", "tilt", "cue_group_positions", "position_model_tilt", "position_fixture_tilt", "Tilt", "°", -180, 180, false);
    connect(positionTiltPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    positionAttributesLayout->addWidget(positionTiltPanel);

    positionZoomPanel = new AttributePanel("positions", "zoom", "cue_group_positions", "position_model_zoom", "position_fixture_zoom", "Zoom", "°", 0, 180, false);
    connect(positionZoomPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    positionAttributesLayout->addWidget(positionZoomPanel);

    positionFocusPanel = new AttributePanel("positions", "focus", "cue_group_positions", "position_model_focus", "position_fixture_focus", "Focus", "%", 0, 100, false);
    connect(positionFocusPanel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    positionAttributesLayout->addWidget(positionFocusPanel);
}

void ControlPanel::reload() {
    updateItemLabel("intensities", "cue_group_intensities", "Intensity", intensityLabel);
    updateItemLabel("colors", "cue_group_colors", "Color", colorLabel);
    updateItemLabel("positions", "cue_group_positions", "Position", positionLabel);
    intensityDimmerPanel->reload();
    colorHuePanel->reload();
    colorSaturationPanel->reload();
    positionPanPanel->reload();
    positionTiltPanel->reload();
    positionZoomPanel->reload();
    positionFocusPanel->reload();
}

void ControlPanel::updateItemLabel(const QString table, const QString valueTable, const QString itemName, QLabel* label) {
    QSqlQuery query;
    if (!query.exec("SELECT CONCAT(" + table + ".id, ' ', " + table + ".label) FROM " + table + ", currentcue, currentitems, " + valueTable + " WHERE " + table + ".key = " + valueTable + ".valueitem_key AND " + valueTable + ".item_key = currentcue.key AND " + valueTable + ".foreignitem_key = currentitems.group_key")) {
        label->setText("");
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    if (query.next()) {
        label->setText(query.value(0).toString());
    } else {
        label->setText("No " + itemName);
    }
}
