/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef POSITIONDATA_H
#define POSITIONDATA_H

#include "valuedata.h"

class PositionData : public ValueData {
public:
    PositionData();
    PositionData(int fixtureKey, int positionKey);
    static void nextFrame();
    void fade(PositionData lastPosition, float fade);
    float getPan(const int fixtureKey, const float panRange, const float rotation, const bool invertPan) const;
    float getPanAngle() const;
    float getTilt(float tiltRange) const;
    float getTiltAngle() const;
    float getZoom(float minZoom, float maxZoom) const;
    float getZoomAngle() const;
    float getFocus() const;
private:
    static QHash<int, float> fixturePan;
    static QHash<int, float> oldFixturePan;
    float panAngle = 0; // 0 <= pan < 360
    float tiltAngle = 0; // -180 <= tilt <= 180
    float zoomAngle = 15; // 0 < zoom <= 180
    float focus = 0; // 0 <= focus <= 1
};

#endif // POSITIONDATA_H
