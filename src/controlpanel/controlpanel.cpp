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
    resize(500, 300);
    QHBoxLayout* layout = new QHBoxLayout();
    setLayout(layout);

    attributePanels.append(new AttributePanel("intensities", "dimmer", "cue_group_intensities", "Dimmer", "%"));
    attributePanels.append(new AttributePanel("colors", "hue", "cue_group_colors", "Hue", "°"));
    attributePanels.append(new AttributePanel("colors", "saturation", "cue_group_colors", "Saturation", "%"));
    attributePanels.append(new AttributePanel("positions", "pan", "cue_group_positions", "Pan", "°"));
    attributePanels.append(new AttributePanel("positions", "tilt", "cue_group_positions", "Tilt", "°"));
    attributePanels.append(new AttributePanel("positions", "zoom", "cue_group_positions", "Zoom", "°"));
    attributePanels.append(new AttributePanel("positions", "focus", "cue_group_positions", "Focus", "%"));

    for (AttributePanel* panel : attributePanels) {
        layout->addWidget(panel);
        connect(panel, &AttributePanel::dbChanged, this, &ControlPanel::dbChanged);
    }
}

void ControlPanel::reload() {
    for (AttributePanel* panel : attributePanels) {
        panel->reload();
    }
}
