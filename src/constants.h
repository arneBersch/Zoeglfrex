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
const char* const id = "0";
const char* const label = "1";

const char* const modelChannels = "2";
const char* const modelPanRange = "3.1";
const char* const modelTiltRange = "3.2";
const char* const modelMinZoom = "4.1";
const char* const modelMaxZoom = "4.2";

const char* const fixtureModel = "2";
const char* const fixtureUniverse = "3.1";
const char* const fixtureAddress = "3.2";
const char* const fixtureRotation = "4.1";
const char* const fixtureInvertPan = "4.2";

const char* const groupFixtures = "2";

const char* const intensityDimmer = "2";

const char* const colorHue = "2";
const char* const colorSaturation = "3";
const char* const colorQuality = "4";

const char* const positionPan = "2";
const char* const positionTilt = "3";
const char* const positionZoom = "4";
const char* const positionFocus = "5";

const char* const effectSteps = "2";

const char* const cuelistPriority = "2";
const char* const cuelistMoveWhileDark = "3";

const char* const cueIntensities = "2";
const char* const cueColors = "3";
const char* const cuePositions = "4";
}

#endif // CONSTANTS_H
