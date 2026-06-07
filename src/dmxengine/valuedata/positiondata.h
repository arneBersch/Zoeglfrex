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
    void fade(PositionData lastPosition, float fade);
    float getPan();
    float getTilt();
    float getZoom();
    float getFocus();
private:
    float pan = 0; // 0 <= pan < 360
    float tilt = 0; // -180 <= tilt <= 180
    float zoom = 15; // 0 < zoom <= 180
    float focus = 0; // 0 <= focus <= 1
};

#endif // POSITIONDATA_H
