/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace AttributeIds {
const int id = 0;
const int label = 1;

const int modelChannels = 2;
const int modelPanRange = 3;
const int modelTiltRange = 4;
const int modelMinZoom = 5;
const int modelMaxZoom = 6;

const int fixtureModel = 2;
const int fixtureUniverse = 3;
const int fixtureAddress = 4;
const int fixtureRotation = 5;
const int fixtureInvertPan = 6;

const int groupFixtures = 2;

const int intensityDimmer = 2;

const int colorHue = 2;
const int colorSaturation = 3;
const int colorQuality = 4;

const int positionPan = 2;
const int positionTilt = 3;
const int positionZoom = 4;

const int effectSteps = 2;

const int cuelistPriority = 2;
const int cuelistMoveWhileDark = 3;

const int cueIntensities = 2;
const int cueColors = 3;
const int cuePositions = 4;
}

#endif // CONSTANTS_H
