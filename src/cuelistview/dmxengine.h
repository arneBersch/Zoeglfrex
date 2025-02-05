/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DMXENGINE_H
#define DMXENGINE_H

#include <QtWidgets>

#include "sacnserver.h"
class Kernel;
struct Cue;

class DmxEngine : public QWidget
{
    Q_OBJECT
public:
    explicit DmxEngine(Kernel *core, QWidget *parent = nullptr);
    void generateDmx();
private:
    void sendDmx();
    void setNetworkInterface();
    Cue* lastCue;
    QList<uint8_t> lastCueValues;
    QList<uint8_t> currentCueValues;
    int remainingFadeFrames;
    int totalFadeFrames;
    Kernel *kernel;
    SacnServer *sacn;
    QTimer *timer;
    QComboBox *interfaceSelectionBox;
    QList<QNetworkInterface> networkInterfaces = QList<QNetworkInterface>();
    QList<QNetworkAddressEntry> networkAddresses = QList<QNetworkAddressEntry>();
};

#endif // DMXENGINE_H
