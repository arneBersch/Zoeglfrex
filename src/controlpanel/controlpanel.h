/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtWidgets>

class Kernel;

namespace ControlPanelColumns {
enum {
    dimmer,
    hue,
    saturation,
    pan,
    tilt,
    zoom,
};
}

namespace ControlPanelRows {
enum {
    label,
    valueLabel,
    modelValueLabel,
    fixtureValueLabel,
    dial,
};
}

class ControlPanel : public QWidget {
public:
    ControlPanel(Kernel* core);
    void reload();
private:
    bool reloading = false;
    QGridLayout *layout;
    QList<QLabel*> valueLabels = QList<QLabel*>(6, nullptr);
    QList<QLabel*> modelValueLabels = QList<QLabel*>(6, nullptr);
    QList<QLabel*> fixtureValueLabels = QList<QLabel*>(6, nullptr);
    QList<QDial*> dials = QList<QDial*>(6, nullptr);
    Kernel* kernel;
};

#endif // CONTROLPANEL_H
