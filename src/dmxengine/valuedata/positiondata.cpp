/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "positiondata.h"

PositionData::PositionData() {}

PositionData::PositionData(const int fixtureKey, const int positionKey) {
    pan = getFixtureValue(fixtureKey, positionKey, "positions", "pan", "position_model_pan", "position_fixture_pan");
    tilt = getFixtureValue(fixtureKey, positionKey, "positions", "tilt", "position_model_tilt", "position_fixture_tilt");
    zoom = getFixtureValue(fixtureKey, positionKey, "positions", "zoom", "position_model_zoom", "position_fixture_zoom");
    focus = getFixtureValue(fixtureKey, positionKey, "positions", "focus", "position_model_focus", "position_fixture_focus");
}

void PositionData::fade(PositionData lastPosition, float fade) {
    if (std::abs(pan - lastPosition.pan) > 180) {
        if (lastPosition.pan > pan) {
            pan += 360;
        } else {
            lastPosition.pan += 360;
        }
    }
    pan += (lastPosition.pan - pan) * fade;
    pan = std::fmod(pan, 360);
    tilt += (lastPosition.tilt - tilt) * fade;
    zoom += (lastPosition.zoom - zoom) * fade;
    focus += (lastPosition.focus - focus) * fade;
}

float PositionData::getPan() {
    return pan;
}

float PositionData::getTilt() {
    return tilt;
}

float PositionData::getZoom() {
    return zoom;
}

float PositionData::getFocus() {
    return focus;
}