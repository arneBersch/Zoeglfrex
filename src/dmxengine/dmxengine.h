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
class Fixture;
class Group;
class Effect;
class Cue;
struct rgbColor;
struct positionAngles;

class DmxEngine : public QWidget {
    Q_OBJECT
public:
    DmxEngine(Kernel *core, QWidget *parent = nullptr);
    SacnServer *sacnServer;
    QPushButton *highlightButton;
    QPushButton *soloButton;
    QPushButton* skipFadeButton;
    const int PROCESSINGRATE = 40; // 40 FPS
private:
    void generateDmx();
    // QMap<Group*, QMap<Effect*, int>> renderCue(Cue* cue, QMap<Fixture*, float>* dimmerValues, QMap<Fixture*, rgbColor>* colorValues, QMap<Fixture*, positionAngles>* positionValues, QMap<Fixture*, QMap<int, uint8_t>>* rawValues, QMap<Fixture*, QMap<int, bool>>* rawFade);
    QTimer *timer;
    QMap<Group*, QMap<Effect*, int>> groupEffectFrames;
    QMap<Fixture*, float> fixturePan;
    QProgressBar* fadeProgress;
    Kernel *kernel;
};

#endif // DMXENGINE_H
