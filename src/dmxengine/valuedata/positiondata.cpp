/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "positiondata.h"

QHash<int, float> PositionData::fixturePan;
QHash<int, float> PositionData::oldFixturePan;

PositionData::PositionData() {}

PositionData::PositionData(const int fixtureKey, const int positionKey) {
    panAngle = getFixtureValue(fixtureKey, positionKey, "positions", "pan", "position_model_pan", "position_fixture_pan");
    tiltAngle = getFixtureValue(fixtureKey, positionKey, "positions", "tilt", "position_model_tilt", "position_fixture_tilt");
    zoomAngle = getFixtureValue(fixtureKey, positionKey, "positions", "zoom", "position_model_zoom", "position_fixture_zoom");
    focus = getFixtureValue(fixtureKey, positionKey, "positions", "focus", "position_model_focus", "position_fixture_focus");
}

void PositionData::fade(const PositionData lastPosition, const float fade) {
    float lastPan = lastPosition.panAngle;
    if (std::abs(panAngle - lastPan) > 180) {
        if (lastPan > panAngle) {
            panAngle += 360;
        } else {
            lastPan += 360;
        }
    }
    panAngle = fadeValue(lastPan, panAngle, fade);
    panAngle = std::fmod(panAngle, 360);

    tiltAngle = fadeValue(lastPosition.tiltAngle, tiltAngle, fade);

    zoomAngle = fadeValue(lastPosition.zoomAngle, zoomAngle, fade);

    focus = fadeValue(lastPosition.focus, focus, fade);
}

float PositionData::getPan(const int fixtureKey, const float panRange, const float rotation, const bool invertPan) const {
    float resultingAngle;
    if (invertPan) {
        resultingAngle = rotation - panAngle;
    } else {
        resultingAngle = rotation + panAngle;
    }

    while (resultingAngle >= 360) {
        resultingAngle -= 360;
    }
    while (resultingAngle < 0) {
        resultingAngle += 360;
    }

    float pan = panAngle / panRange;

    const float lastFramePan = oldFixturePan.value(fixtureKey, 0);

    for (float angle = panAngle; angle <= panRange; angle += 360) {
        const float anglePan = angle / panRange;

        if (std::abs(lastFramePan - anglePan) < std::abs(lastFramePan - pan)) {
            pan = anglePan;
        }
    }

    pan = std::min<float>(pan, 1);

    fixturePan[fixtureKey] = pan;

    return pan;
}

float PositionData::getPanAngle() const {
    return panAngle;
}

float PositionData::getTilt(const float tiltRange) const{
    float tilt = 0.5 + (tiltAngle / tiltRange);

    tilt = std::min<float>(tilt, 1);
    tilt = std::max<float>(tilt, 0);
    return tilt;
}

float PositionData::getTiltAngle() const {
    return tiltAngle;
}

float PositionData::getZoom(const float minZoom, const float maxZoom) const {
    float zoom = (zoomAngle - minZoom) / (maxZoom - minZoom);

    zoom = std::min<float>(zoom, 1);
    zoom = std::max<float>(zoom, 0);
    return zoom;
}

float PositionData::getZoomAngle() const {
    return zoomAngle;
}

float PositionData::getFocus() const {
    return focus;
}

void PositionData::nextFrame() {
    oldFixturePan = fixturePan;
    fixturePan.clear();
}
