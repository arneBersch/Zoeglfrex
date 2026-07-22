/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INTENSITYDATA_H
#define INTENSITYDATA_H

#include "valuedata.h"

class IntensityData : public ValueData {
public:
    IntensityData();
    IntensityData(int fixtureKey, int intensityKey);
    void merge(IntensityData intensity);
    void fade(IntensityData lastIntensity, float fade);
    void smoothDim(float lastDimmer);
    static IntensityData highlightValue();
    float getDimmer() const;
private:
    float dimmer = 0;
};

#endif // INTENSITYDATA_H
